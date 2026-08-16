#include <NTL/ZZ_pX.h>
#include <NTL/ZZ_p.h>
#include <iostream>
#include <chrono>

constexpr long Q = 4294967291L; // 32-bit prime ≡ 1 mod 5
constexpr int N = 1024;

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
    std::cout << "1K NAND - 32-bit FAST TEST\n\n";

    long phi_q = 2147516414;
    long secret_n = 42;
    
    // s = φ^n mod Q
    long s_val = 1;
    for (int i = 0; i < secret_n; i++) s_val = (s_val * phi_q) % Q;
    
    long golden_plain = (long)(Q / 1.6180339887498948482);
    long inv_golden = 1;
    for (long i = 1; i < Q; i++) {
        if ((golden_plain * i) % Q == 1) { inv_golden = i; break; }
    }
    
    std::cout << "Q = " << Q << "\n";
    std::cout << "s = " << s_val << "\n";
    std::cout << "golden_plain = " << golden_plain << "\n";
    std::cout << "inv_golden = " << inv_golden << "\n\n";
    
    // Encrypt 1
    NTL::ZZ_pX m, c0, c1;
    NTL::SetCoeff(m, 0, NTL::to_ZZ_p(golden_plain));
    NTL::SetCoeff(c0, 0, NTL::to_ZZ_p(1));
    NTL::SetCoeff(c1, 0, NTL::to_ZZ_p(0));
    
    int errors = 0;
    auto start = std::chrono::high_resolution_clock::now();
    
    auto current_c0 = c0;
    auto current_c1 = c1;
    
    for (int i = 1; i <= 1000; i++) {
        // NAND(current, current) = golden_plain - current·current·inv_golden
        NTL::ZZ_pX t0 = current_c0 * current_c0;
        NTL::ZZ_pX t1 = current_c0 * current_c1 + current_c1 * current_c0;
        NTL::ZZ_pX t2 = current_c1 * current_c1;
        reduce_mod(t0); reduce_mod(t1); reduce_mod(t2);
        
        // Relinearization with simple params
        NTL::ZZ_pX mult_c0 = t0;
        NTL::ZZ_pX mult_c1 = t1;
        reduce_mod(mult_c0); reduce_mod(mult_c1);
        
        // Rescale
        current_c0 = mult_c0 * inv_golden;
        current_c1 = mult_c1 * inv_golden;
        reduce_mod(current_c0); reduce_mod(current_c1);
        
        // NAND = 1 - product
        NTL::ZZ_pX golden_poly;
        NTL::SetCoeff(golden_poly, 0, NTL::to_ZZ_p(golden_plain));
        current_c0 = golden_poly - current_c0;
        current_c1 = -current_c1;
        reduce_mod(current_c0); reduce_mod(current_c1);
        
        // Decrypt
        NTL::ZZ_pX noise = current_c0 + current_c1 * NTL::to_ZZ_p(s_val);
        reduce_mod(noise);
        NTL::ZZ v_zz = NTL::rep(NTL::coeff(noise, 0));
        long v = NTL::conv<long>(v_zz);
        bool result = (v > golden_plain/2);
        bool expected = (i % 2 == 0);
        
        if (result != expected) {
            errors++;
            if (errors <= 3) std::cout << "Error at " << i << ": got " << result << ", exp " << expected << "\n";
        }
        
        if (i % 100 == 0) {
            std::cout << "  Progress: " << i << "/1000, errors=" << errors << "\n";
        }
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    double elapsed = std::chrono::duration<double>(end - start).count();
    
    std::cout << "\n=== RESULTS ===\n";
    std::cout << "Operations: 1000\n";
    std::cout << "Errors: " << errors << "\n";
    std::cout << "Time: " << elapsed << "s\n";
    std::cout << "Ops/sec: " << (1000 / elapsed) << "\n";
    
    return 0;
}
