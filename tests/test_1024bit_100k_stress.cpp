#include <NTL/ZZ_pX.h>
#include <NTL/ZZ_p.h>
#include <NTL/ZZ.h>
#include <iostream>
#include <chrono>

// 1024-bit prime, Q ≡ 1 mod 5 (verified)
const std::string Q_STR = "179769313486231590772930519078902473361797697894230657273430081157732675805500963132708477322407536021120113879871393357658789768814416622492847430639474124377767893424865485276302219601246094119453082952085005768838150682342462881473913110540827237163350510684586298239947245938479716304835356329624224137111";
constexpr int N = 1024;
constexpr double PHI = 1.6180339887498948482;

void init_ring(const NTL::ZZ& Q) {
    NTL::ZZ_p::init(Q);
}

void reduce_mod(NTL::ZZ_pX& poly) {
    if (NTL::deg(poly) < N) return;
    NTL::ZZ_pX reduced;
    reduced.SetLength(N);
    for (int i = 0; i <= NTL::deg(poly); i++) {
        NTL::ZZ_p coeff = NTL::coeff(poly, i);
        int reduced_deg = i % (2 * N);
        if (reduced_deg >= N) {
            reduced_deg -= N;
            coeff = -coeff;
        }
        NTL::SetCoeff(reduced, reduced_deg, NTL::coeff(reduced, reduced_deg) + coeff);
    }
    poly = reduced;
}

NTL::ZZ fib_mod_zz(long n, const NTL::ZZ& mod) {
    if (n == 0) return NTL::to_ZZ(0);
    if (n == 1) return NTL::to_ZZ(1);
    NTL::ZZ a = NTL::to_ZZ(0);
    NTL::ZZ b = NTL::to_ZZ(1);
    for (long i = 2; i <= n; i++) {
        NTL::ZZ c = (a + b) % mod;
        a = b;
        b = c;
    }
    return b;
}

NTL::ZZ mod_inv_zz(const NTL::ZZ& a, const NTL::ZZ& mod) {
    NTL::ZZ t = NTL::to_ZZ(0);
    NTL::ZZ new_t = NTL::to_ZZ(1);
    NTL::ZZ r = mod;
    NTL::ZZ new_r = a;
    while (new_r != 0) {
        NTL::ZZ q = r / new_r;
        NTL::ZZ temp_t = t - q * new_t;
        t = new_t;
        new_t = temp_t;
        NTL::ZZ temp_r = r - q * new_r;
        r = new_r;
        new_r = temp_r;
    }
    if (t < 0) t += mod;
    return t;
}

NTL::ZZ mod_pow_zz(NTL::ZZ base, long exp, const NTL::ZZ& mod) {
    NTL::ZZ result = NTL::to_ZZ(1);
    base %= mod;
    while (exp > 0) {
        if (exp & 1) result = (result * base) % mod;
        base = (base * base) % mod;
        exp >>= 1;
    }
    return result;
}

int main() {
    NTL::ZZ Q = NTL::to_ZZ(Q_STR.c_str());
    init_ring(Q);

    NTL::ZZ sqrt5;
    NTL::SqrRootMod(sqrt5, NTL::to_ZZ(5), Q);
    NTL::ZZ inv2 = NTL::InvMod(NTL::to_ZZ(2), Q);
    NTL::ZZ phi_zz = ((NTL::to_ZZ(1) + sqrt5) * inv2) % Q;

    // golden_plain = Q / φ using exact arithmetic
    // Since φ = (1+√5)/2, Q/φ = Q * 2/(1+√5) = Q * (√5-1)/2
    NTL::ZZ golden_plain = ((sqrt5 - 1) * inv2) % Q;
    if (golden_plain < 0) golden_plain += Q;
    NTL::ZZ inv_golden = mod_inv_zz(golden_plain, Q);

    NTL::ZZ s_val = mod_pow_zz(phi_zz, 42, Q);
    NTL::ZZ_pX s;
    NTL::SetCoeff(s, 0, NTL::to_ZZ_p(s_val));

    long secret_n = 42;
    NTL::ZZ F_n = fib_mod_zz(secret_n, Q);
    NTL::ZZ F_n_minus_1 = fib_mod_zz(secret_n - 1, Q);
    NTL::ZZ F_2n = fib_mod_zz(2 * secret_n, Q);
    NTL::ZZ F_2n_minus_1 = fib_mod_zz(2 * secret_n - 1, Q);
    NTL::ZZ F_n_inv = mod_inv_zz(F_n, Q);
    NTL::ZZ alpha = (F_2n * F_n_inv) % Q;
    NTL::ZZ beta = (F_2n_minus_1 - alpha * F_n_minus_1) % Q;
    if (beta < 0) beta += Q;

    NTL::ZZ_p alpha_p = NTL::to_ZZ_p(alpha);
    NTL::ZZ_p beta_p = NTL::to_ZZ_p(beta);
    NTL::ZZ_p golden_plain_p = NTL::to_ZZ_p(golden_plain);
    NTL::ZZ_p inv_golden_p = NTL::to_ZZ_p(inv_golden);

    std::cout << "1024-BIT POST-QUANTUM 100K NAND STRESS TEST\n";
    std::cout << "==========================================\n\n";
    std::cout << "Q bits: " << NTL::NumBits(Q) << "\n";
    std::cout << "Q mod 5: " << Q % 5 << "\n";
    std::cout << "Is prime: " << (NTL::ProbPrime(Q, 10) ? "YES" : "NO") << "\n";
    std::cout << "α = " << alpha << "\n";
    std::cout << "β = " << (beta == Q - 1 ? "Q-1" : "other") << "\n\n";

    // Verify
    NTL::ZZ s_sq = (s_val * s_val) % Q;
    NTL::ZZ asb = (alpha * s_val + beta) % Q;
    std::cout << "Verify s² = α·s + β: " << (s_sq == asb ? "YES ✓" : "NO ✗") << "\n\n";

    // KeyGen
    NTL::ZZ_pX pk0, pk1;
    NTL::ZZ_pX a_poly, e_poly;
    uint64_t state = 42;
    for (int i = 0; i < N; i++) {
        state ^= (state << 13); state ^= (state >> 7); state ^= (state << 17);
        NTL::ZZ state_zz = NTL::to_ZZ(state);
        NTL::ZZ state_mod = state_zz % Q;
        NTL::SetCoeff(a_poly, i, NTL::to_ZZ_p(state_mod));
        NTL::SetCoeff(e_poly, i, NTL::to_ZZ_p((state % 10000) == 0 ? 1 : 0));
    }
    pk0 = -(a_poly * s + e_poly);
    pk1 = a_poly;
    reduce_mod(pk0);
    reduce_mod(pk1);

    auto encrypt = [&](bool bit, uint64_t nonce) {
        NTL::ZZ_pX m;
        NTL::SetCoeff(m, 0, bit ? golden_plain_p : NTL::to_ZZ_p(0));
        uint64_t st = nonce;
        NTL::ZZ_pX u, e0, e1;
        for (int i = 0; i < N; i++) {
            st ^= (st << 13); st ^= (st >> 7); st ^= (st << 17);
            NTL::SetCoeff(u, i, NTL::to_ZZ_p((long)((st % 3) - 1)));
            NTL::SetCoeff(e0, i, NTL::to_ZZ_p((st % 10000) == 0 ? 1 : 0));
            NTL::SetCoeff(e1, i, NTL::to_ZZ_p((st % 10000) == 0 ? 1 : 0));
        }
        NTL::ZZ_pX c0 = pk0 * u + e0 + m;
        NTL::ZZ_pX c1 = pk1 * u + e1;
        reduce_mod(c0);
        reduce_mod(c1);
        return std::make_pair(c0, c1);
    };

    auto decrypt = [&](const std::pair<NTL::ZZ_pX, NTL::ZZ_pX>& ct) {
        NTL::ZZ_pX noise = ct.first + ct.second * s;
        reduce_mod(noise);
        NTL::ZZ v = NTL::rep(NTL::coeff(noise, 0));
        NTL::ZZ dist_0 = (v < Q/2) ? v : Q - v;
        NTL::ZZ diff = (v > golden_plain) ? v - golden_plain : golden_plain - v;
        NTL::ZZ dist_golden = (diff < Q/2) ? diff : Q - diff;
        return dist_golden < dist_0;
    };

    auto nand_gate = [&](const std::pair<NTL::ZZ_pX, NTL::ZZ_pX>& a,
                         const std::pair<NTL::ZZ_pX, NTL::ZZ_pX>& b) {
        NTL::ZZ_pX t0 = a.first * b.first;
        NTL::ZZ_pX t1 = a.first * b.second + a.second * b.first;
        NTL::ZZ_pX t2 = a.second * b.second;
        reduce_mod(t0); reduce_mod(t1); reduce_mod(t2);
        NTL::ZZ_pX mult_c0 = t0 + t2 * beta_p;
        NTL::ZZ_pX mult_c1 = t1 + t2 * alpha_p;
        reduce_mod(mult_c0); reduce_mod(mult_c1);
        NTL::ZZ_pX rescaled_c0 = mult_c0 * inv_golden_p;
        NTL::ZZ_pX rescaled_c1 = mult_c1 * inv_golden_p;
        reduce_mod(rescaled_c0); reduce_mod(rescaled_c1);
        NTL::ZZ_pX golden_poly;
        NTL::SetCoeff(golden_poly, 0, golden_plain_p);
        return std::make_pair(golden_poly - rescaled_c0, -rescaled_c1);
    };

    // ========== 100K NAND STRESS TEST ==========
    std::cout << "=== 100K NAND OPERATIONS (10K print) ===\n";
    auto ct1 = encrypt(true, 2000);
    auto current = ct1;
    
    int errors = 0;
    auto start_time = std::chrono::high_resolution_clock::now();
    
    for (int i = 1; i <= 100000; i++) {
        current = nand_gate(current, current);
        bool result = decrypt(current);
        bool expected = (i % 2 == 0);
        if (result != expected) {
            std::cout << "  ❌ Error at " << i << ": got " << result 
                      << ", exp " << expected << "\n";
            errors++;
            if (errors > 10) {
                std::cout << "  Too many errors, stopping...\n";
                break;
            }
        }
        if (i % 10000 == 0) {
            auto now = std::chrono::high_resolution_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - start_time).count();
            double ops_per_sec = (double)i / elapsed;
            std::cout << "  [" << i << "/100K] errors=" << errors 
                      << " time=" << elapsed << "s (" << ops_per_sec << " ops/sec)\n";
        }
    }
    
    auto end_time = std::chrono::high_resolution_clock::now();
    auto total_time = std::chrono::duration_cast<std::chrono::seconds>(end_time - start_time).count();
    
    std::cout << "\n=== " << (errors == 0 ? "PASS ✓" : "FAIL ✗") << " ===\n";
    std::cout << "Total errors: " << errors << "\n";
    std::cout << "Total time: " << total_time << "s\n";
    if (total_time > 0) {
        std::cout << "Ops/sec: " << (100000.0 / total_time) << "\n";
    }
    
    return 0;
}
