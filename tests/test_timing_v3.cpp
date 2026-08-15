#include "../src/fhe/golden_fibonacci_fhe_v3.h"
#include <iostream>
#include <chrono>
#include <vector>
#include <algorithm>

int main() {
    std::cout << "TIMING CONSISTENCY TEST — V3 (Pre-allocated)\n";
    std::cout << "=============================================\n\n";
    
    NTL::ZZ Q = NTL::to_ZZ("115792089237316195423570985008687907853269984665640564039457584007913129640731");
    golden_fhe_v3::FibonacciFHEV3 fhe(Q, 42);
    
    auto ct0 = fhe.encrypt(false, 1000);
    auto ct1 = fhe.encrypt(true, 2000);
    
    std::vector<double> encrypt_times, decrypt_times, nand_times;
    
    // Warm up (more iterations para sa cache)
    for (int i = 0; i < 50; i++) {
        auto ct = fhe.encrypt(i % 2, 3000 + i);
        fhe.decrypt(ct);
        fhe.nand_gate(ct0, ct1);
    }
    
    // Measure
    for (int i = 0; i < 200; i++) {
        auto start = std::chrono::high_resolution_clock::now();
        auto ct = fhe.encrypt(i % 2, 3000 + i);
        auto end = std::chrono::high_resolution_clock::now();
        encrypt_times.push_back(std::chrono::duration_cast<std::chrono::microseconds>(end - start).count());
        
        start = std::chrono::high_resolution_clock::now();
        fhe.decrypt(ct);
        end = std::chrono::high_resolution_clock::now();
        decrypt_times.push_back(std::chrono::duration_cast<std::chrono::microseconds>(end - start).count());
        
        start = std::chrono::high_resolution_clock::now();
        fhe.nand_gate(ct0, ct1);
        end = std::chrono::high_resolution_clock::now();
        nand_times.push_back(std::chrono::duration_cast<std::chrono::microseconds>(end - start).count());
    }
    
    auto stats = [](const std::vector<double>& times) {
        double sum = 0;
        for (auto t : times) sum += t;
        double avg = sum / times.size();
        double var = 0;
        for (auto t : times) var += (t-avg)*(t-avg);
        var /= times.size();
        return std::make_pair(avg, std::sqrt(var));
    };
    
    auto enc = stats(encrypt_times);
    auto dec = stats(decrypt_times);
    auto nand = stats(nand_times);
    
    std::cout << "Operation | Avg (μs) | Std Dev (μs) | CV%\n";
    std::cout << "----------|----------|--------------|-----\n";
    std::cout << "Encrypt   | " << (int)enc.first << " | " << (int)enc.second << " | " 
              << (enc.second/enc.first*100) << "%\n";
    std::cout << "Decrypt   | " << (int)dec.first << " | " << (int)dec.second << " | " 
              << (dec.second/dec.first*100) << "%\n";
    std::cout << "NAND      | " << (int)nand.first << " | " << (int)nand.second << " | " 
              << (nand.second/nand.first*100) << "%\n";
    
    double max_cv = std::max({enc.second/enc.first, dec.second/dec.first, nand.second/nand.first});
    std::cout << "\nMax CV: " << (max_cv*100) << "%\n";
    std::cout << "Status: " << (max_cv < 0.10 ? "GOOD ✓" : "VARIABLE ⚠") << "\n";
    
    return 0;
}
