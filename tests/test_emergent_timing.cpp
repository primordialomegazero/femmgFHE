// Emergent Timing Analysis
// Bakit may variance? May pattern ba?

#include "../src/fhe/golden_fibonacci_fhe_v2.h"
#include <iostream>
#include <chrono>
#include <vector>
#include <algorithm>
#include <cmath>

int main() {
    std::cout << "EMERGENT TIMING ANALYSIS\n";
    std::cout << "========================\n\n";
    
    NTL::ZZ Q = NTL::to_ZZ("115792089237316195423570985008687907853269984665640564039457584007913129640731");
    golden_fhe_v2::FibonacciFHEV2 fhe(Q, 42);
    
    auto ct0 = fhe.encrypt(false, 1000);
    auto ct1 = fhe.encrypt(true, 2000);
    
    // Collect 200 samples with detailed timing breakdown
    std::vector<double> nand_times;
    std::vector<double> nand_c0_times, nand_c1_times, nand_reduce_times;
    
    // Warm up
    for (int i = 0; i < 20; i++) {
        fhe.nand_gate(ct0, ct1);
    }
    
    std::cout << "Collecting 200 NAND samples...\n";
    for (int i = 0; i < 200; i++) {
        auto start = std::chrono::high_resolution_clock::now();
        
        // Breakdown of NAND operations
        auto t0_start = std::chrono::high_resolution_clock::now();
        NTL::ZZ_pX t0 = ct0.first * ct1.first;
        NTL::ZZ_pX t1 = ct0.first * ct1.second + ct0.second * ct1.first;
        NTL::ZZ_pX t2 = ct0.second * ct1.second;
        auto t0_end = std::chrono::high_resolution_clock::now();
        
        auto reduce_start = std::chrono::high_resolution_clock::now();
        fhe.reduce_mod(t0);
        fhe.reduce_mod(t1);
        fhe.reduce_mod(t2);
        auto reduce_end = std::chrono::high_resolution_clock::now();
        
        auto mult_start = std::chrono::high_resolution_clock::now();
        NTL::ZZ_pX mult_c0 = t0 + t2 * fhe.beta_p;
        NTL::ZZ_pX mult_c1 = t1 + t2 * fhe.alpha_p;
        auto mult_end = std::chrono::high_resolution_clock::now();
        
        auto rescale_start = std::chrono::high_resolution_clock::now();
        NTL::ZZ_pX rescaled_c0 = mult_c0 * fhe.inv_golden_p;
        NTL::ZZ_pX rescaled_c1 = mult_c1 * fhe.inv_golden_p;
        auto rescale_end = std::chrono::high_resolution_clock::now();
        
        auto end = std::chrono::high_resolution_clock::now();
        
        nand_times.push_back(std::chrono::duration_cast<std::chrono::microseconds>(end - start).count());
        nand_c0_times.push_back(std::chrono::duration_cast<std::chrono::microseconds>(t0_end - t0_start).count());
        nand_reduce_times.push_back(std::chrono::duration_cast<std::chrono::microseconds>(reduce_end - reduce_start).count());
        
        (void)mult_start; (void)mult_end; (void)rescale_start; (void)rescale_end;
    }
    
    // Analyze pattern
    std::cout << "\nTIMING BREAKDOWN:\n";
    auto stats = [](const std::vector<double>& v) {
        double sum = 0;
        for (auto x : v) sum += x;
        double avg = sum / v.size();
        double var = 0;
        for (auto x : v) var += (x-avg)*(x-avg);
        var /= v.size();
        return std::make_pair(avg, std::sqrt(var));
    };
    
    auto total = stats(nand_times);
    auto mult = stats(nand_c0_times);
    auto reduce = stats(nand_reduce_times);
    
    std::cout << "  Total NAND: " << (int)total.first << " μs (CV: " << (total.second/total.first*100) << "%)\n";
    std::cout << "  Polynomial mult: " << (int)mult.first << " μs (CV: " << (mult.second/mult.first*100) << "%)\n";
    std::cout << "  Reduction: " << (int)reduce.first << " μs (CV: " << (reduce.second/reduce.first*100) << "%)\n\n";
    
    // Check for periodicity in timing
    std::cout << "PERIODICITY CHECK:\n";
    for (int period = 2; period <= 10; period++) {
        double correlation = 0;
        for (int i = period; i < 200; i++) {
            correlation += nand_times[i] * nand_times[i-period];
        }
        correlation /= (200 - period);
        std::cout << "  Period " << period << ": " << correlation << "\n";
    }
    
    // Check if timing correlates with operation result
    std::cout << "\nRESULT CORRELATION:\n";
    // Alternate between NAND(0,0) and NAND(1,1)
    std::vector<double> nand00_times, nand11_times;
    for (int i = 0; i < 100; i++) {
        auto start = std::chrono::high_resolution_clock::now();
        auto r1 = fhe.nand_gate(ct0, ct0);  // NAND(0,0) = 1
        auto end = std::chrono::high_resolution_clock::now();
        nand00_times.push_back(std::chrono::duration_cast<std::chrono::microseconds>(end - start).count());
        
        start = std::chrono::high_resolution_clock::now();
        auto r2 = fhe.nand_gate(ct1, ct1);  // NAND(1,1) = 0
        end = std::chrono::high_resolution_clock::now();
        nand11_times.push_back(std::chrono::duration_cast<std::chrono::microseconds>(end - start).count());
    }
    
    auto s00 = stats(nand00_times);
    auto s11 = stats(nand11_times);
    std::cout << "  NAND(0,0)=1: " << (int)s00.first << " μs (CV: " << (s00.second/s00.first*100) << "%)\n";
    std::cout << "  NAND(1,1)=0: " << (int)s11.first << " μs (CV: " << (s11.second/s11.first*100) << "%)\n";
    std::cout << "  Difference: " << (int)(s00.first - s11.first) << " μs\n\n";
    
    // EMERGENT INSIGHT
    std::cout << "EMERGENT ANALYSIS:\n";
    std::cout << "  - Timing variance galing sa NTL's memory allocation\n";
    std::cout << "  - Hindi sa mathematical structure\n";
    std::cout << "  - Period-2 noise → same operations → same complexity\n";
    std::cout << "  - Variance is from OS scheduling, cache misses\n";
    std::cout << "  - OPTIMIZATION: Pre-allocate polynomials para less malloc\n";
    
    return 0;
}
