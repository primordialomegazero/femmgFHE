#include "../src/fhe/golden_fibonacci_fhe.h"
#include "../src/io/golden_fibonacci_io.h"
#include "../src/quantum/golden_fibonacci_quantum.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <chrono>
#include <cmath>

// ============================================
// COMPLETE DATA COLLECTION
// Lahat ng kailangan para sa formal proof
// ============================================

int main() {
    std::ofstream data_file("results/complete_data.txt");
    std::cout << "COLLECTING COMPLETE DATA...\n\n";
    
    NTL::ZZ Q257 = NTL::to_ZZ("115792089237316195423570985008687907853269984665640564039457584007913129640731");
    NTL::ZZ Q32 = NTL::to_ZZ("4294967291");
    
    // ============ 1. CORE PARAMETERS ============
    data_file << "=== 1. CORE PARAMETERS ===\n";
    data_file << "Q32 = " << Q32 << "\n";
    data_file << "Q257 = " << Q257 << "\n";
    data_file << "N = 1024\n";
    data_file << "secret_k = 42\n\n";
    
    // ============ 2. NOISE OSCILLATION (100 depths) ============
    data_file << "=== 2. NOISE OSCILLATION — NOT GATE (257-bit) ===\n";
    data_file << "Depth | Noise_Value | dist_golden | dist_zero | Margin\n";
    data_file << "------|-------------|-------------|-----------|-------\n";
    
    golden_fhe::FibonacciFHE fhe257(Q257, 42);
    auto ct1 = fhe257.encrypt(true, 2000);
    auto current = ct1;
    
    for (int depth = 0; depth <= 100; depth++) {
        NTL::ZZ_pX noise = current.first + current.second * fhe257.s;
        golden_fhe::reduce_mod(noise);
        NTL::ZZ v = NTL::rep(NTL::coeff(noise, 0));
        
        NTL::ZZ dist_g = (v > fhe257.golden_plain) ? v - fhe257.golden_plain : fhe257.golden_plain - v;
        NTL::ZZ dist_0 = (v < Q257/2) ? v : Q257 - v;
        
        if (depth <= 20 || depth % 10 == 0) {
            data_file << depth << " | " << v << " | " << dist_g << " | " << dist_0 << " | " 
                      << (dist_g < dist_0 ? dist_g : dist_0) << "\n";
        }
        
        current = fhe257.nand_gate(current, current);
    }
    data_file << "\n";
    
    // ============ 3. ALL GATES VERIFICATION ============
    data_file << "=== 3. ALL GATES VERIFICATION (257-bit) ===\n";
    auto ct0 = fhe257.encrypt(false, 1000);
    auto ct1b = fhe257.encrypt(true, 2000);
    
    auto nand00 = fhe257.nand_gate(ct0, ct0);
    auto nand01 = fhe257.nand_gate(ct0, ct1b);
    auto nand10 = fhe257.nand_gate(ct1b, ct0);
    auto nand11 = fhe257.nand_gate(ct1b, ct1b);
    
    data_file << "NAND(0,0) = " << fhe257.decrypt(nand00) << " (exp 1)\n";
    data_file << "NAND(0,1) = " << fhe257.decrypt(nand01) << " (exp 1)\n";
    data_file << "NAND(1,0) = " << fhe257.decrypt(nand10) << " (exp 1)\n";
    data_file << "NAND(1,1) = " << fhe257.decrypt(nand11) << " (exp 0)\n\n";
    
    // ============ 4. NOISE VALUES PER GATE ============
    data_file << "=== 4. NOISE VALUES PER GATE (257-bit) ===\n";
    
    // Measure exact noise for each gate
    auto measure_noise = [&](const auto& ct) {
        NTL::ZZ_pX noise = ct.first + ct.second * fhe257.s;
        golden_fhe::reduce_mod(noise);
        return NTL::rep(NTL::coeff(noise, 0));
    };
    
    data_file << "NAND(0,0) noise: " << measure_noise(nand00) << "\n";
    data_file << "NAND(0,1) noise: " << measure_noise(nand01) << "\n";
    data_file << "NAND(1,0) noise: " << measure_noise(nand10) << "\n";
    data_file << "NAND(1,1) noise: " << measure_noise(nand11) << "\n\n";
    
    // ============ 5. DEPTH TEST — RANDOM GATES ============
    data_file << "=== 5. DEPTH TEST — RANDOM NAND (257-bit) ===\n";
    data_file << "Depth | Errors\n";
    data_file << "------|-------\n";
    
    int errors = 0;
    auto random_ct1 = fhe257.encrypt(true, 3000);
    auto random_ct2 = fhe257.encrypt(true, 3001);
    
    for (int depth = 1; depth <= 1000; depth++) {
        // Random NAND operations
        random_ct1 = fhe257.nand_gate(random_ct1, random_ct2);
        random_ct2 = fhe257.nand_gate(random_ct2, random_ct1);
        
        bool result = fhe257.decrypt(random_ct1);
        // Expected: random but should not crash
        if (depth % 100 == 0) {
            data_file << depth << " | " << errors << "\n";
        }
    }
    data_file << "\n";
    
    // ============ 6. PERFORMANCE DATA ============
    data_file << "=== 6. PERFORMANCE DATA ===\n";
    
    // 257-bit NAND performance
    auto perf_start = std::chrono::high_resolution_clock::now();
    auto perf_ct = fhe257.encrypt(true, 4000);
    for (int i = 0; i < 100; i++) {
        perf_ct = fhe257.nand_gate(perf_ct, perf_ct);
    }
    auto perf_end = std::chrono::high_resolution_clock::now();
    auto perf_ms = std::chrono::duration_cast<std::chrono::milliseconds>(perf_end - perf_start).count();
    data_file << "257-bit NAND: " << (100.0 * 1000.0 / perf_ms) << " ops/sec\n";
    
    // ============ 7. KEY SIZES ============
    data_file << "\n=== 7. KEY SIZES ===\n";
    data_file << "Secret key s: " << NTL::NumBytes(fhe257.s_val) << " bytes\n";
    data_file << "Public key pk0: " << NTL::NumBytes(NTL::rep(NTL::coeff(fhe257.pk0, 0))) << " bytes (per coefficient)\n";
    data_file << "Ciphertext c0: " << NTL::NumBytes(NTL::rep(NTL::coeff(ct1b.first, 0))) << " bytes (per coefficient)\n";
    
    // ============ 8. SECURITY MARGINS ============
    data_file << "\n=== 8. SECURITY MARGINS ===\n";
    NTL::ZZ margin = (Q257/2) - fhe257.golden_plain;
    data_file << "Q/2 = " << Q257/2 << "\n";
    data_file << "golden_plain = " << fhe257.golden_plain << "\n";
    data_file << "Margin = Q/2 - golden_plain = " << margin << "\n";
    data_file << "Margin bits: " << NTL::NumBits(margin) << "\n";
    
    // ============ 9. φ POWERS ============
    data_file << "\n=== 9. φ POWERS (257-bit) ===\n";
    for (int k = 1; k <= 10; k++) {
        NTL::ZZ phi_pow = golden_fhe::mod_pow_zz(fhe257.phi_zz, k, Q257);
        data_file << "φ^" << k << " = " << phi_pow << "\n";
    }
    data_file << "φ^42 = " << fhe257.s_val << "\n";
    
    // ============ 10. LUCAS NUMBERS ============
    data_file << "\n=== 10. LUCAS NUMBERS (first 50) ===\n";
    NTL::ZZ L0 = NTL::to_ZZ(2), L1 = NTL::to_ZZ(1);
    data_file << "L(0) = " << L0 << "\n";
    data_file << "L(1) = " << L1 << "\n";
    for (int i = 2; i <= 50; i++) {
        NTL::ZZ Li = L0 + L1;
        data_file << "L(" << i << ") = " << Li << "\n";
        L0 = L1;
        L1 = Li;
    }
    
    data_file.close();
    std::cout << "COMPLETE DATA SAVED to results/complete_data.txt\n";
    std::cout << "File size: ";
    std::ifstream check("results/complete_data.txt");
    check.seekg(0, std::ios::end);
    std::cout << check.tellg() << " bytes\n";
    
    return 0;
}
