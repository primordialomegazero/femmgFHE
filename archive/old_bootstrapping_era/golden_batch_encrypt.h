#pragma once
#include "golden_privacy_system.h"
#include <vector>
#include <thread>
#include <chrono>
#include <iomanip>

// ============================================
// BATCH ENCRYPTION - 128 nonces per ciphertext
// ============================================

class GoldenBatchEncryptor {
private:
    GoldenPrivacySystem& gps;
    static const int BATCH_SIZE = 128;  // Fibonacci optimal
    
public:
    GoldenBatchEncryptor(GoldenPrivacySystem& system) : gps(system) {}
    
    // BATCH: Encrypt multiple bits in one ciphertext
    GoldenFHE::Cipher batch_encrypt(const std::vector<bool>& bits) {
        return gps.batch_encrypt(bits);
    }
    
    // BATCH: Encrypt N bits using batching
    std::vector<GoldenFHE::Cipher> encrypt_many(const std::vector<bool>& bits) {
        std::vector<GoldenFHE::Cipher> results;
        
        for (size_t i = 0; i < bits.size(); i += BATCH_SIZE) {
            std::vector<bool> batch;
            for (size_t j = i; j < std::min(i + BATCH_SIZE, bits.size()); j++) {
                batch.push_back(bits[j]);
            }
            results.push_back(gps.batch_encrypt(batch));
        }
        
        return results;
    }
    
    // ULTRA FAST: Batch encrypt for 100M bits
    void benchmark_100m() {
        std::cout << "100M BATCH ENCRYPTION BENCHMARK\n";
        std::cout << "================================\n\n";
        
        const long long TOTAL = 100000000;  // 100M bits
        const int BATCH = 128;
        const long long NUM_BATCHES = TOTAL / BATCH;
        
        auto start = std::chrono::high_resolution_clock::now();
        
        long long total_processed = 0;
        long long correct = 0;
        
        for (long long b = 0; b < NUM_BATCHES; b++) {
            std::vector<bool> bits(BATCH);
            for (int i = 0; i < BATCH; i++) {
                bits[i] = ((b * BATCH + i) % 2) == 0;
            }
            
            auto ct = gps.batch_encrypt(bits);
            auto decoded = gps.batch_decrypt(ct, BATCH);
            
            for (int i = 0; i < BATCH; i++) {
                if (decoded[i] == bits[i]) correct++;
                total_processed++;
            }
            
            // Progress every 10M bits
            if ((b + 1) % 78125 == 0) {
                auto now = std::chrono::high_resolution_clock::now();
                double elapsed = std::chrono::duration<double>(now - start).count();
                std::cout << "  [" << total_processed / 1000000 << "M/100M] "
                          << std::fixed << std::setprecision(0)
                          << total_processed / elapsed << " bits/sec"
                          << "  (" << correct << " correct)\n";
            }
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        double t = std::chrono::duration<double>(end - start).count();
        
        std::cout << "\n=== 100M RESULTS ===\n";
        std::cout << "Total: " << total_processed << " bits\n";
        std::cout << "Correct: " << correct << " / " << total_processed << "\n";
        std::cout << "Time: " << t << " s\n";
        std::cout << "Throughput: " << std::fixed << std::setprecision(0) 
                  << total_processed / t << " bits/sec\n";
        std::cout << "Status: " << (correct == total_processed ? "PERFECT ✅" : "ERRORS ❌") << "\n";
    }
};
