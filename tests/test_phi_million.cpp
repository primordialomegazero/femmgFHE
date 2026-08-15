#include <NTL/ZZ_pX.h>
#include <NTL/ZZ_p.h>
#include <iostream>
#include <vector>
#include <chrono>
#include <iomanip>

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
    
    std::cout << "MILLION DEPTH TEST\n";
    std::cout << "==================\n\n";
    
    long golden_plain = static_cast<long>(Q / PHI);
    long threshold = static_cast<long>(Q / (2 * PHI));
    long inv_golden = 140433618;
    
    NTL::ZZ_pX s;
    NTL::SetCoeff(s, 0, PHI_MOD_Q);
    
    // Pre-generate keys
    NTL::ZZ_pX a, e;
    uint64_t state = 42;
    for (int i = 0; i < N; i++) {
        state ^= (state << 13); state ^= (state >> 7); state ^= (state << 17);
        NTL::SetCoeff(a, i, state % Q);
        NTL::SetCoeff(e, i, (state % 10000) == 0 ? 1 : 0);
    }
    NTL::ZZ_pX pk0 = -(a * s + e);
    NTL::ZZ_pX pk1 = a;
    reduce_mod(pk0);
    reduce_mod(pk1);
    
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
    
    auto nand_gate = [&](const auto& a, const auto& b) {
        NTL::ZZ_pX t0 = a.first * b.first;
        NTL::ZZ_pX t1 = a.first * b.second + a.second * b.first;
        NTL::ZZ_pX t2 = a.second * b.second;
        reduce_mod(t0); reduce_mod(t1); reduce_mod(t2);
        
        NTL::ZZ_pX mult_c0 = t0 + t2;
        NTL::ZZ_pX mult_c1 = t1 + t2;
        reduce_mod(mult_c0); reduce_mod(mult_c1);
        
        NTL::ZZ_pX rescaled_c0 = mult_c0 * inv_golden;
        NTL::ZZ_pX rescaled_c1 = mult_c1 * inv_golden;
        reduce_mod(rescaled_c0); reduce_mod(rescaled_c1);
        
        NTL::ZZ_pX golden_poly;
        NTL::SetCoeff(golden_poly, 0, golden_plain);
        
        return std::make_pair(golden_poly - rescaled_c0, -rescaled_c1);
    };
    
    const int TARGET = 1000000;  // 1M depth
    
    auto val = encrypt(true, 10000);
    int errors = 0;
    int checkpoints[] = {1000, 10000, 100000, 500000, 1000000};
    int check_idx = 0;
    
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < TARGET; i++) {
        val = nand_gate(val, val);
        
        // Check periodically
        if (check_idx < 5 && i + 1 == checkpoints[check_idx]) {
            bool got = decrypt(val);
            bool expected = (i + 1) % 2 == 0;
            if (got != expected) errors++;
            
            auto now = std::chrono::high_resolution_clock::now();
            double elapsed = std::chrono::duration<double>(now - start).count();
            
            std::cout << "Checkpoint " << i+1 << ": errors=" << errors 
                      << " time=" << std::fixed << std::setprecision(1) 
                      << elapsed << "s" << std::endl;
            
            check_idx++;
        }
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    double total = std::chrono::duration<double>(end - start).count();
    
    std::cout << "\n=== FINAL RESULT ===\n";
    std::cout << "Total depth: " << TARGET << "\n";
    std::cout << "Total errors: " << errors << "\n";
    std::cout << "Total time: " << std::fixed << std::setprecision(1) << total << "s\n";
    std::cout << "Time per op: " << std::fixed << std::setprecision(3) 
              << (total / TARGET) * 1000 << "ms\n";
    std::cout << "Status: " << (errors == 0 ? "ALL PASS - NO DEGRADATION ✓" : 
                                          "DEGRADATION DETECTED ❌") << "\n";
    
    return 0;
}
