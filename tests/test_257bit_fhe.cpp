#include <NTL/ZZ_pX.h>
#include <NTL/ZZ_p.h>
#include <iostream>
#include <chrono>

// 257-bit prime na Q ≡ 1 mod 5 (may φ)
const char* Q_STR = "115792089237316195423570985008687907853269984665640564039457584007913129640731";
const char* PHI_STR = "112652859229649681368096351188711019049377490364605197292503729558236545569044";

constexpr int N = 1024;

void init_ring(const NTL::ZZ& Q) {
    NTL::ZZ_p::init(Q);
    NTL::ZZ_p alpha_p = NTL::to_ZZ_p(alpha);
    NTL::ZZ_p beta_p = NTL::to_ZZ_p(beta);
    NTL::ZZ_p golden_plain_p = NTL::to_ZZ_p(golden_plain);
    NTL::ZZ_p inv_golden_p = NTL::to_ZZ_p(inv_golden);
}

void reduce_mod(NTL::ZZ_pX& poly, const NTL::ZZ& Q) {
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

// Fibonacci mod Q (fast doubling)
NTL::ZZ fib_mod(long n, const NTL::ZZ& Q) {
    if (n == 0) return NTL::to_ZZ("0");
    NTL::ZZ a = NTL::to_ZZ("0");
    NTL::ZZ b = NTL::to_ZZ("1");
    for (long i = 2; i <= n; i++) {
        NTL::ZZ c = (a + b) % Q;
        a = b;
        b = c;
    }
    return b;
}

// Modular inverse
NTL::ZZ mod_inv(const NTL::ZZ& a, const NTL::ZZ& mod) {
    return NTL::InvMod(a, mod);
}

// Modular exponentiation
NTL::ZZ mod_pow(NTL::ZZ base, NTL::ZZ exp, const NTL::ZZ& mod) {
    NTL::ZZ result = NTL::to_ZZ("1");
    base %= mod;
    while (exp > 0) {
        if ((exp % 2) == 1) result = (result * base) % mod;
        base = (base * base) % mod;
        exp /= 2;
    }
    return result;
}

int main() {
    NTL::ZZ Q = NTL::to_ZZ(Q_STR);
    NTL::ZZ phi_q = NTL::to_ZZ(PHI_STR);
    
    init_ring(Q);
    
    std::cout << "257-BIT FHE TEST (Fibonacci Secret Key)\n";
    std::cout << "=========================================\n\n";
    
    std::cout << "Q = " << Q << "\n";
    std::cout << "Bits: " << NTL::NumBits(Q) << "\n";
    std::cout << "φ = " << phi_q << "\n\n";
    
    // Secret n
    long secret_n = 42;
    NTL::ZZ s_val = mod_pow(phi_q, NTL::to_ZZ(secret_n), Q);
    
    std::cout << "s = φ^42 = " << s_val << "\n\n";
    
    // Compute α at β
    NTL::ZZ F_n = fib_mod(secret_n, Q);
    NTL::ZZ F_n_minus_1 = fib_mod(secret_n - 1, Q);
    NTL::ZZ F_2n = fib_mod(2 * secret_n, Q);
    NTL::ZZ F_2n_minus_1 = fib_mod(2 * secret_n - 1, Q);
    NTL::ZZ F_n_inv = mod_inv(F_n, Q);
    NTL::ZZ alpha = (F_2n * F_n_inv) % Q;
    NTL::ZZ beta = (F_2n_minus_1 - alpha * F_n_minus_1) % Q;
    if (beta < 0) beta += Q;
    
    std::cout << "α = " << alpha << "\n";
    std::cout << "β = " << beta << "\n\n";
    
    // Golden plain: Q / φ (floating point approximation)
    // Para sa prototype: gamitin ang maliit na value para sa test
    NTL::ZZ golden_plain = Q / 2;  // Simplified para sa test
    NTL::ZZ inv_golden = mod_inv(golden_plain, Q);
    
    // Secret key polynomial s
    NTL::ZZ_pX s;
    NTL::SetCoeff(s, 0, NTL::to_ZZ_p(s_val));
    
    // Public key
    NTL::ZZ_pX a_poly, e_poly;
    for (int i = 0; i < N; i++) {
        uint64_t state = 42;
        state ^= (state << 13); state ^= (state >> 7); state ^= (state << 17);
        NTL::SetCoeff(a_poly, i, state % Q);
        NTL::SetCoeff(e_poly, i, (state % 10000) == 0 ? 1 : 0);
    }
    
    NTL::ZZ_pX pk0 = -(a_poly * s + e_poly);
    NTL::ZZ_pX pk1 = a_poly;
    
    // Encrypt
    auto encrypt = [&](bool bit, uint64_t nonce) {
        NTL::ZZ_pX m;
        NTL::SetCoeff(m, 0, bit ? golden_plain_p : NTL::ZZ_p::zero());
        uint64_t state = nonce;
        NTL::ZZ_pX u, e0, e1;
        for (int i = 0; i < N; i++) {
            state ^= (state << 13); state ^= (state >> 7); state ^= (state << 17);
            NTL::SetCoeff(u, i, (state % 3) - 1);
            NTL::SetCoeff(e0, i, (state % 10000) == 0 ? 1 : 0);
            NTL::SetCoeff(e1, i, (state % 10000) == 0 ? 1 : 0);
        }
        NTL::ZZ_pX c0 = pk0 * u + e0 + m;
        NTL::ZZ_pX c1 = pk1 * u + e1;
        reduce_mod(c0, Q); reduce_mod(c1, Q);
        return std::make_pair(c0, c1);
    };
    
    // Decrypt
    auto decrypt = [&](const auto& ct) {
        NTL::ZZ_pX noise = ct.first + ct.second * s;
        reduce_mod(noise, Q);
        NTL::ZZ v = NTL::rep(NTL::coeff(noise, 0));
        NTL::ZZ dist_0 = std::min(v, Q - v);
        NTL::ZZ dist_golden = std::min(
            v > golden_plain ? v - golden_plain : golden_plain - v,
            Q - (v > golden_plain ? v - golden_plain : golden_plain - v)
        );
        return dist_golden < dist_0;
    };
    
    // Test basic
    auto ct0 = encrypt(false, 1000);
    auto ct1 = encrypt(true, 2000);
    
    std::cout << "=== BASIC TEST ===\n";
    std::cout << "  Encrypt(0) → " << decrypt(ct0) << " (expected 0)\n";
    std::cout << "  Encrypt(1) → " << decrypt(ct1) << " (expected 1)\n";
    
    // Test NAND
    auto nand_gate = [&](const auto& a, const auto& b) {
        NTL::ZZ_pX t0 = a.first * b.first;
        NTL::ZZ_pX t1 = a.first * b.second + a.second * b.first;
        NTL::ZZ_pX t2 = a.second * b.second;
        reduce_mod(t0, Q); reduce_mod(t1, Q); reduce_mod(t2, Q);
        
        NTL::ZZ_pX mult_c0 = t0 + t2 * beta_p;
        NTL::ZZ_pX mult_c1 = t1 + t2 * alpha_p;
        reduce_mod(mult_c0, Q); reduce_mod(mult_c1, Q);
        
        NTL::ZZ_pX rescaled_c0 = mult_c0 * inv_golden_p;
        NTL::ZZ_pX rescaled_c1 = mult_c1 * inv_golden_p;
        reduce_mod(rescaled_c0, Q); reduce_mod(rescaled_c1, Q);
        
        NTL::ZZ_pX golden_poly;
        NTL::SetCoeff(golden_poly, 0, golden_plain_p);
        
        return std::make_pair(golden_poly - rescaled_c0, -rescaled_c1);
    };
    
    auto nand01 = nand_gate(ct0, ct1);
    auto nand11 = nand_gate(ct1, ct1);
    
    std::cout << "  NAND(0,1) → " << decrypt(nand01) << " (expected 1)\n";
    std::cout << "  NAND(1,1) → " << decrypt(nand11) << " (expected 0)\n";
    
    // Depth test
    std::cout << "\n=== DEPTH TEST (100 operations) ===\n";
    auto val = ct1;
    int errors = 0;
    
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < 100; i++) {
        val = nand_gate(val, val);
        bool got = decrypt(val);
        bool expected = (i + 1) % 2 == 0;
        if (got != expected) errors++;
    }
    auto end = std::chrono::high_resolution_clock::now();
    double t = std::chrono::duration<double>(end - start).count();
    
    std::cout << "  Depth 100: errors=" << errors << " time=" << t << "s\n";
    std::cout << "  " << (errors == 0 ? "✓ PASS" : "✗ FAIL") << "\n";
    
    return 0;
}
