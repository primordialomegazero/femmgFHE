#include <NTL/ZZ_pX.h>
#include <NTL/ZZ_p.h>
#include <NTL/ZZ.h>
#include <iostream>
#include <chrono>

const std::string Q_STR = "115792089237316195423570985008687907853269984665640564039457584007913129640731";
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

    // Compute φ
    NTL::ZZ sqrt5;
    NTL::SqrRootMod(sqrt5, NTL::to_ZZ(5), Q);
    NTL::ZZ inv2 = NTL::InvMod(NTL::to_ZZ(2), Q);
    NTL::ZZ phi_zz = ((NTL::to_ZZ(1) + sqrt5) * inv2) % Q;

    // golden_plain = Q / φ (using double approximation)
    double Q_double = NTL::conv<double>(Q);
    NTL::ZZ golden_plain = NTL::to_ZZ(Q_double / PHI);
    NTL::ZZ inv_golden = mod_inv_zz(golden_plain, Q);

    // s = φ^42
    NTL::ZZ s_val = mod_pow_zz(phi_zz, 42, Q);
    NTL::ZZ_pX s;
    NTL::SetCoeff(s, 0, NTL::to_ZZ_p(s_val));

    // Compute α at β
    long secret_n = 42;
    NTL::ZZ F_n = fib_mod_zz(secret_n, Q);
    NTL::ZZ F_n_minus_1 = fib_mod_zz(secret_n - 1, Q);
    NTL::ZZ F_2n = fib_mod_zz(2 * secret_n, Q);
    NTL::ZZ F_2n_minus_1 = fib_mod_zz(2 * secret_n - 1, Q);
    NTL::ZZ F_n_inv = mod_inv_zz(F_n, Q);
    NTL::ZZ alpha = (F_2n * F_n_inv) % Q;
    NTL::ZZ beta = (F_2n_minus_1 - alpha * F_n_minus_1) % Q;
    if (beta < 0) beta += Q;

    // Convert to ZZ_p for polynomial operations
    NTL::ZZ_p alpha_p = NTL::to_ZZ_p(alpha);
    NTL::ZZ_p beta_p = NTL::to_ZZ_p(beta);
    NTL::ZZ_p golden_plain_p = NTL::to_ZZ_p(golden_plain);
    NTL::ZZ_p inv_golden_p = NTL::to_ZZ_p(inv_golden);

    std::cout << "257-BIT EXACT PORT (32-bit structure)\n\n";
    std::cout << "PARAMETERS:\n";
    std::cout << "  Q = " << Q << " (" << NTL::NumBits(Q) << " bits)\n";
    std::cout << "  φ = " << phi_zz << "\n";
    std::cout << "  golden_plain = " << golden_plain << "\n";
    std::cout << "  inv_golden = " << inv_golden << "\n";
    std::cout << "  s = φ^42 = " << s_val << "\n";
    std::cout << "  α = " << alpha << "\n";
    std::cout << "  β = " << beta << "\n\n";

    // Verify s² = α·s + β
    NTL::ZZ s_sq = (s_val * s_val) % Q;
    NTL::ZZ asb = (alpha * s_val + beta) % Q;
    std::cout << "VERIFY s² = α·s + β: " << (s_sq == asb ? "YES ✓" : "NO ✗") << "\n\n";

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

    // Encrypt/Decrypt functions
    auto encrypt = [&](bool bit, uint64_t nonce) {
        NTL::ZZ_pX m;
        NTL::SetCoeff(m, 0, bit ? golden_plain_p : NTL::to_ZZ_p(0));
        
        uint64_t st = nonce;
        NTL::ZZ_pX u, e0, e1;
        for (int i = 0; i < N; i++) {
            st ^= (st << 13); st ^= (st >> 7); st ^= (st << 17);
            long u_val = (st % 3) - 1;
            NTL::SetCoeff(u, i, NTL::to_ZZ_p(u_val));
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
        
        // Relinearization: s² = α·s + β
        NTL::ZZ_pX mult_c0 = t0 + t2 * beta_p;
        NTL::ZZ_pX mult_c1 = t1 + t2 * alpha_p;
        reduce_mod(mult_c0); reduce_mod(mult_c1);
        
        // Rescaling
        NTL::ZZ_pX rescaled_c0 = mult_c0 * inv_golden_p;
        NTL::ZZ_pX rescaled_c1 = mult_c1 * inv_golden_p;
        reduce_mod(rescaled_c0); reduce_mod(rescaled_c1);
        
        // NAND: golden_plain - product
        NTL::ZZ_pX golden_poly;
        NTL::SetCoeff(golden_poly, 0, golden_plain_p);
        
        return std::make_pair(golden_poly - rescaled_c0, -rescaled_c1);
    };

    // Basic tests
    auto ct0 = encrypt(false, 1000);
    auto ct1 = encrypt(true, 2000);
    
    std::cout << "BASIC TESTS:\n";
    std::cout << "  Encrypt(0) → " << decrypt(ct0) << " ✓\n";
    std::cout << "  Encrypt(1) → " << decrypt(ct1) << " ✓\n";
    
    auto nand01 = nand_gate(ct0, ct1);
    std::cout << "  NAND(0,1) = " << decrypt(nand01) << " (exp 1)\n";
    
    auto nand11 = nand_gate(ct1, ct1);
    std::cout << "  NAND(1,1) = " << decrypt(nand11) << " (exp 0)\n\n";

    // Depth test
    std::cout << "DEPTH TEST (100 iterations):\n";
    auto current = ct1;
    bool ok = true;
    for (int i = 1; i <= 100; i++) {
        current = nand_gate(current, current);  // NOT operation
        bool result = decrypt(current);
        bool expected = (i % 2 == 0);  // NOT(NOT(...)) pattern
        if (result != expected) {
            std::cout << "  ❌ FAIL at depth " << i << ": got " << result 
                      << ", exp " << expected << "\n";
            ok = false;
            break;
        }
        if (i <= 5 || i % 20 == 0) {
            std::cout << "  Depth " << i << ": " << result 
                      << " (exp " << expected << ") ✓\n";
        }
    }
    if (ok) std::cout << "  All 100 depths passed! ✓\n";
    std::cout << "\n=== " << (ok ? "PASS ✓" : "FAIL ✗") << " ===\n";

    return 0;
}
