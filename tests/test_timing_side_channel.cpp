// Basic side-channel analysis: timing consistency
#include "../src/fhe/golden_fibonacci_fhe.h"
#include <iostream>
#include <chrono>
#include <vector>

int main() {
    std::cout << "TIMING CONSISTENCY TEST\n";
    std::cout << "=======================\n\n";
    
    NTL::ZZ Q = NTL::to_ZZ("115792089237316195423570985008687907853269984665640564039457584007913129640731");
    golden_fhe::FibonacciFHE fhe(Q, 42);
    
    auto ct0 = fhe.encrypt(false, 1000);
    auto ct1 = fhe.encrypt(true, 2000);
    
    // Measure timing for different operations
    std::vector<double> encrypt_times, decrypt_times, nand_times;
    
    for (int i = 0; i < 100; i++) {
        // Encrypt timing
        auto start = std::chrono::high_resolution_clock::now();
        auto ct = fhe.encrypt(i % 2, 3000 + i);
        auto end = std::chrono::high_resolution_clock::now();
        encrypt_times.push_back(std::chrono::duration_cast<std::chrono::microseconds>(end - start).count());
        
        // Decrypt timing
        start = std::chrono::high_resolution_clock::now();
        bool bit = fhe.decrypt(ct);
        end = std::chrono::high_resolution_clock::now();
        decrypt_times.push_back(std::chrono::duration_cast<std::chrono::microseconds>(end - start).count());
        
        // NAND timing
        start = std::chrono::high_resolution_clock::now();
        auto result = fhe.nand_gate(ct0, ct1);
        end = std::chrono::high_resolution_clock::now();
        nand_times.push_back(std::chrono::duration_cast<std::chrono::microseconds>(end - start).count());
    }
    
    // Compute statistics
    auto compute_stats = [](const std::vector<double>& times) {
        double sum = 0;
        for (auto t : times) sum += t;
        double avg = sum / times.size();
        double variance = 0;
        for (auto t : times) variance += (t - avg) * (t - avg);
        variance /= times.size();
        return std::make_pair(avg, std::sqrt(variance));
    };
    
    auto enc_stats = compute_stats(encrypt_times);
    auto dec_stats = compute_stats(decrypt_times);
    auto nand_stats = compute_stats(nand_times);
    
    std::cout << "Operation | Avg (μs) | Std Dev (μs) | Variance\n";
    std::cout << "----------|----------|--------------|---------\n";
    std::cout << "Encrypt   | " << enc_stats.first << " | " << enc_stats.second << " | " 
              << (enc_stats.second / enc_stats.first * 100) << "%\n";
    std::cout << "Decrypt   | " << dec_stats.first << " | " << dec_stats.second << " | " 
              << (dec_stats.second / dec_stats.first * 100) << "%\n";
    std::cout << "NAND      | " << nand_stats.first << " | " << nand_stats.second << " | " 
              << (nand_stats.second / nand_stats.first * 100) << "%\n";
    
    std::cout << "\nTiming consistency: " << (nand_stats.second / nand_stats.first < 0.1 ? "GOOD ✓" : "VARIABLE ⚠") << "\n";
    
    return 0;
}
