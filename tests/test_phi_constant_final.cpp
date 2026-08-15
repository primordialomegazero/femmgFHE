#include <NTL/ZZ_pX.h>
#include <NTL/ZZ_p.h>
#include <iostream>

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

int main() {
    init_ring();
    
    std::cout << "FHE WITH s=φ AND RESCALING\n\n";
    
    long golden_plain = static_cast<long>(Q / PHI);
    long threshold = static_cast<long>(Q / (2 * PHI));
    
    // Modular inverse ng golden_plain
    long inv_golden = 140433618;
    
    // Secret key: s = φ (constant)
    NTL::ZZ_pX s;
    NTL::SetCoeff(s, 0, PHI_MOD_Q);
    
    // Public key
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
    
    // NAND na may rescaling
    auto nand_gate = [&](const auto& a, const auto& b) {
        NTL::ZZ_pX t0 = a.first * b.first;
        NTL::ZZ_pX t1 = a.first * b.second + a.second * b.first;
        NTL::ZZ_pX t2 = a.second * b.second;
        
        reduce_mod(t0); reduce_mod(t1); reduce_mod(t2);
        
        // s² = s + 1
        NTL::ZZ_pX mult_c0 = t0 + t2;
        NTL::ZZ_pX mult_c1 = t1 + t2;
        reduce_mod(mult_c0); reduce_mod(mult_c1);
        
        // RESCALING: divide sa golden_plain
        NTL::ZZ_pX rescaled_c0 = mult_c0 * inv_golden;
        NTL::ZZ_pX rescaled_c1 = mult_c1 * inv_golden;
        reduce_mod(rescaled_c0); reduce_mod(rescaled_c1);
        
        // NAND: golden_plain - rescaled
        NTL::ZZ_pX golden_poly;
        NTL::SetCoeff(golden_poly, 0, golden_plain);
        
        return std::make_pair(golden_poly - rescaled_c0, -rescaled_c1);
    };
    
    // TEST
    auto ct0 = encrypt(false, 1000);
    auto ct1 = encrypt(true, 2000);
    
    std::cout << "Encrypt(0) → " << decrypt(ct0) << " (expected 0)\n";
    std::cout << "Encrypt(1) → " << decrypt(ct1) << " (expected 1)\n";
    
    auto nand01 = nand_gate(ct0, ct1);
    std::cout << "NAND(0,1) → " << decrypt(nand01) << " (expected 1)\n";
    
    auto nand11 = nand_gate(ct1, ct1);
    std::cout << "NAND(1,1) → " << decrypt(nand11) << " (expected 0)\n";
    
    // Full 4-case test
    std::cout << "\n=== ALL 4 NAND CASES ===\n";
    bool all_pass = true;
    for (int i = 0; i < 4; i++) {
        bool a_bit = (i >> 1) & 1;
        bool b_bit = i & 1;
        auto ca = encrypt(a_bit, 100 + i * 10);
        auto cb = encrypt(b_bit, 200 + i * 10);
        auto result = nand_gate(ca, cb);
        bool got = decrypt(result);
        bool expected = !(a_bit && b_bit);
        
        std::cout << "  NAND(" << a_bit << "," << b_bit << ") = " << got 
                  << " (expected " << expected << ")";
        if (got != expected) { std::cout << " ❌"; all_pass = false; }
        else { std::cout << " ✓"; }
        std::cout << "\n";
    }
    
    std::cout << "\nStatus: " << (all_pass ? "ALL PASS ✓" : "FAIL ❌") << "\n";
    
    return 0;
}
