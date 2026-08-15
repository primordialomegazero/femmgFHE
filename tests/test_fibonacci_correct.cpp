#include <NTL/ZZ_pX.h>
#include <NTL/ZZ_p.h>
#include <iostream>
#include <vector>
#include <chrono>

constexpr long Q = 536870909;
constexpr int N = 1024;
constexpr long PHI_MOD_Q = 386640388;
constexpr double PHI = 1.6180339887498948482;

void init_ring() { NTL::ZZ_p::init(NTL::ZZ(Q)); }

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

long fib_mod(long n, long mod) {
    if (n == 0) return 0;
    long a = 0, b = 1;
    for (long i = 2; i <= n; i++) {
        long c = (a + b) % mod;
        a = b;
        b = c;
    }
    return b;
}

long mod_inv(long a, long mod) {
    for (long i = 1; i < mod; i++) {
        if ((a * i) % mod == 1) return i;
    }
    return 1;
}

long phi_pow_mod(long n, long mod) {
    long result = 1;
    for (long i = 0; i < n; i++) {
        result = (result * PHI_MOD_Q) % mod;
    }
    return result;
}

int main() {
    init_ring();
    
    std::cout << "FHE WITH CORRECT FIBONACCI REDUCTION\n\n";
    
    long golden_plain = static_cast<long>(Q / PHI);
    long threshold = static_cast<long>(Q / (2 * PHI));
    long inv_golden = 140433618;
    
    // Secret n
    long secret_n = 42;
    long s_val = phi_pow_mod(secret_n, Q);
    
    // Compute α at β para sa reduction
    long F_n = fib_mod(secret_n, Q);
    long F_n_minus_1 = fib_mod(secret_n - 1, Q);
    long F_2n = fib_mod(2 * secret_n, Q);
    long F_2n_minus_1 = fib_mod(2 * secret_n - 1, Q);
    
    long F_n_inv = mod_inv(F_n, Q);
    long alpha = (F_2n * F_n_inv) % Q;
    long beta = (F_2n_minus_1 - alpha * F_n_minus_1) % Q;
    if (beta < 0) beta += Q;
    
    std::cout << "n=" << secret_n << ", α=" << alpha << ", β=" << beta << "\n\n";
    
    NTL::ZZ_pX s;
    NTL::SetCoeff(s, 0, s_val);
    
    NTL::ZZ_pX a, e;
    for (int i = 0; i < N; i++) {
        uint64_t state = 42;
        state ^= (state << 13); state ^= (state >> 7); state ^= (state << 17);
        NTL::SetCoeff(a, i, state % Q);
        NTL::SetCoeff(e, i, (state % 10000) == 0 ? 1 : 0);
    }
    NTL::ZZ_pX pk0 = -(a * s + e);
    NTL::ZZ_pX pk1 = a;
    
    auto encrypt = [&](bool bit, uint64_t nonce) {
        NTL::ZZ_pX m;
        NTL::SetCoeff(m, 0, bit ? golden_plain : 0);
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
        reduce_mod(c0); reduce_mod(c1);
        return std::make_pair(c0, c1);
    };
    
    auto decrypt = [&](const auto& ct) {
        NTL::ZZ_pX noise = ct.first + ct.second * s;
        reduce_mod(noise);
        long v = NTL::conv<long>(NTL::coeff(noise, 0));
        return v > threshold;
    };
    
    // NAND na may CORRECT reduction
    auto nand_gate = [&](const auto& a, const auto& b) {
        NTL::ZZ_pX t0 = a.first * b.first;
        NTL::ZZ_pX t1 = a.first * b.second + a.second * b.first;
        NTL::ZZ_pX t2 = a.second * b.second;
        reduce_mod(t0); reduce_mod(t1); reduce_mod(t2);
        
        // CORRECT: s² = α·s + β
        // c1·d1·s² = c1·d1·(α·s + β) = α·(c1·d1)·s + β·(c1·d1)
        NTL::ZZ_pX mult_c0 = t0 + t2 * beta;
        NTL::ZZ_pX mult_c1 = t1 + t2 * alpha;
        reduce_mod(mult_c0); reduce_mod(mult_c1);
        
        NTL::ZZ_pX rescaled_c0 = mult_c0 * inv_golden;
        NTL::ZZ_pX rescaled_c1 = mult_c1 * inv_golden;
        reduce_mod(rescaled_c0); reduce_mod(rescaled_c1);
        
        NTL::ZZ_pX golden_poly;
        NTL::SetCoeff(golden_poly, 0, golden_plain);
        
        return std::make_pair(golden_poly - rescaled_c0, -rescaled_c1);
    };
    
    // Basic test
    auto ct0 = encrypt(false, 1000);
    auto ct1 = encrypt(true, 2000);
    
    std::cout << "Basic test:\n";
    std::cout << "  Encrypt(0) → " << decrypt(ct0) << " (expected 0)\n";
    std::cout << "  Encrypt(1) → " << decrypt(ct1) << " (expected 1)\n";
    auto nand01 = nand_gate(ct0, ct1);
    std::cout << "  NAND(0,1) → " << decrypt(nand01) << " (expected 1)\n";
    auto nand11 = nand_gate(ct1, ct1);
    std::cout << "  NAND(1,1) → " << decrypt(nand11) << " (expected 0)\n\n";
    
    // Depth test
    std::cout << "Depth test:\n";
    auto val = encrypt(true, 10000);
    int errors = 0;
    
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < 5000; i++) {
        val = nand_gate(val, val);
        bool got = decrypt(val);
        bool expected = (i + 1) % 2 == 0;
        if (got != expected) errors++;
    }
    auto end = std::chrono::high_resolution_clock::now();
    double t = std::chrono::duration<double>(end - start).count();
    
    std::cout << "  Depth 5000: errors=" << errors << " time=" << t << "s"
              << (errors == 0 ? " ✓" : " ❌") << "\n";
    
    return 0;
}
