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
    
    NTL::ZZ_pX s;
    NTL::SetCoeff(s, 0, PHI_MOD_Q);
    
    NTL::ZZ_pX a, e;
    for (int i = 0; i < N; i++) {
        uint64_t state = 42;
        state ^= (state << 13); state ^= (state >> 7); state ^= (state << 17);
        NTL::SetCoeff(a, i, state % Q);
        NTL::SetCoeff(e, i, (state % 10000) == 0 ? 1 : 0);
    }
    
    NTL::ZZ_pX pk0 = -(a * s + e);
    NTL::ZZ_pX pk1 = a;
    
    long golden_plain = static_cast<long>(Q / PHI);
    long threshold = static_cast<long>(Q / (2 * PHI));
    
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
    
    auto get_noise = [&](const auto& ct) {
        NTL::ZZ_pX noise = ct.first + ct.second * s;
        reduce_mod(noise);
        return NTL::conv<long>(NTL::coeff(noise, 0));
    };
    
    auto ct0 = encrypt(false, 1000);
    auto ct1 = encrypt(true, 2000);
    
    std::cout << "ct0 noise: " << get_noise(ct0) << " (expected 0)\n";
    std::cout << "ct1 noise: " << get_noise(ct1) << " (expected " << golden_plain << ")\n";
    
    // NAND(1,1)
    auto nand11 = [&]() {
        NTL::ZZ_pX t0 = ct1.first * ct1.first;
        NTL::ZZ_pX t1 = ct1.first * ct1.second + ct1.second * ct1.first;
        NTL::ZZ_pX t2 = ct1.second * ct1.second;
        reduce_mod(t0); reduce_mod(t1); reduce_mod(t2);
        
        NTL::ZZ_pX result_c0 = t0 + t2;
        NTL::ZZ_pX result_c1 = t1 + t2;
        reduce_mod(result_c0); reduce_mod(result_c1);
        
        NTL::ZZ_pX golden_poly;
        NTL::SetCoeff(golden_poly, 0, golden_plain);
        
        return std::make_pair(golden_poly - result_c0, -result_c1);
    };
    
    auto nand_result = nand11();
    std::cout << "NAND(1,1) noise: " << get_noise(nand_result) 
              << " (expected 0 kasi NAND(1,1)=0, dapat < " << threshold << ")\n";
    
    // I-check ang multiplication result
    NTL::ZZ_pX t0 = ct1.first * ct1.first;
    NTL::ZZ_pX t1 = ct1.first * ct1.second + ct1.second * ct1.first;
    NTL::ZZ_pX t2 = ct1.second * ct1.second;
    reduce_mod(t0); reduce_mod(t1); reduce_mod(t2);
    
    NTL::ZZ_pX mult_c0 = t0 + t2;
    NTL::ZZ_pX mult_c1 = t1 + t2;
    reduce_mod(mult_c0); reduce_mod(mult_c1);
    
    NTL::ZZ_pX mult_noise = mult_c0 + mult_c1 * s;
    reduce_mod(mult_noise);
    long mult_v = NTL::conv<long>(NTL::coeff(mult_noise, 0));
    std::cout << "1×1 mult noise: " << mult_v << " (expected ~" << (golden_plain * golden_plain) % Q << ")\n";
    
    return 0;
}
