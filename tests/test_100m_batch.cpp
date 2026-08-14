#include "../src/golden_privacy_system.h"
#include "../src/golden_logger.h"
#include <iostream>
#include <vector>
#include <chrono>
#include <iomanip>

int main() {
    GoldenLogger::set_level(LogLevel::INFO);
    GoldenLogger::enable_file_logging("golden_100m.log");
    
    LOG_INFO("=== 100M BATCH ENCRYPTION TEST ===");
    
    try {
        GoldenPrivacySystem gps(42);
        
        auto xor_func = [](const std::vector<bool>& in) {
            return in[0] ^ in[1];
        };
        gps.obfuscate_program(xor_func, 2);
        LOG_INFO("System initialized");
        
        // ============ TEST 1: iO Evaluation (100M) ============
        LOG_INFO("Test 1/5: iO Evaluation (100M ops)...");
        auto start1 = std::chrono::high_resolution_clock::now();
        long long correct1 = 0;
        
        for (long long i = 0; i < 100000000; i++) {
            std::vector<bool> input = {(bool)((i >> 1) & 1), (bool)(i & 1)};
            bool result = gps.evaluate_io_public(input);
            bool expected = input[0] ^ input[1];
            if (result == expected) correct1++;
            
            if ((i + 1) % 10000000 == 0) {
                auto now = std::chrono::high_resolution_clock::now();
                double elapsed = std::chrono::duration<double>(now - start1).count();
                LOG_INFO("  iO Progress: " + std::to_string((i+1)/1000000) + "M/100M, " +
                         std::to_string((long long)((i+1)/elapsed)) + " ops/sec");
            }
        }
        
        auto end1 = std::chrono::high_resolution_clock::now();
        double t1 = std::chrono::duration<double>(end1 - start1).count();
        LOG_INFO("  iO: " + std::to_string(correct1) + "/100M correct, " +
                 std::to_string((long long)(100000000.0/t1)) + " ops/sec");
        
        // ============ TEST 2: Batch Encryption (10M bits) ============
        LOG_INFO("Test 2/5: Batch Encryption (10M bits)...");
        const int BATCH_SIZE = 128;
        const int NUM_BATCHES = 78125;  // 10M / 128
        
        auto start2 = std::chrono::high_resolution_clock::now();
        long long total_bits = 0;
        long long correct2 = 0;
        
        for (int b = 0; b < NUM_BATCHES; b++) {
            std::vector<bool> bits(BATCH_SIZE);
            for (int i = 0; i < BATCH_SIZE; i++) {
                bits[i] = ((b * BATCH_SIZE + i) % 2) == 0;
            }
            
            auto ct = gps.batch_encrypt(bits);
            total_bits += BATCH_SIZE;
            
            // Verify every 1000 batches
            if (b % 1000 == 0) {
                auto decoded = gps.batch_decrypt(ct, BATCH_SIZE);
                for (int i = 0; i < BATCH_SIZE; i++) {
                    if (decoded[i] == bits[i]) correct2++;
                }
            }
            
            if ((b + 1) % 7812 == 0) {
                auto now = std::chrono::high_resolution_clock::now();
                double elapsed = std::chrono::duration<double>(now - start2).count();
                LOG_INFO("  Batch Progress: " + std::to_string(total_bits/1000000) + "M/10M bits, " +
                         std::to_string((long long)(total_bits/elapsed)) + " bits/sec");
            }
        }
        
        auto end2 = std::chrono::high_resolution_clock::now();
        double t2 = std::chrono::duration<double>(end2 - start2).count();
        LOG_INFO("  Batch: " + std::to_string(total_bits) + " bits, " +
                 std::to_string((long long)(total_bits/t2)) + " bits/sec");
        
        // ============ TEST 3: Quantum Gates (100M) ============
        LOG_INFO("Test 3/5: Quantum Gates (100M)...");
        auto start3 = std::chrono::high_resolution_clock::now();
        for (long long i = 0; i < 100000000; i++) {
            gps.apply_quantum_gate();
            if ((i + 1) % 50000000 == 0) {
                auto now = std::chrono::high_resolution_clock::now();
                double elapsed = std::chrono::duration<double>(now - start3).count();
                LOG_INFO("  Quantum Progress: " + std::to_string((i+1)/1000000) + "M/100M");
            }
        }
        auto end3 = std::chrono::high_resolution_clock::now();
        double t3 = std::chrono::duration<double>(end3 - start3).count();
        LOG_INFO("  Quantum: " + std::to_string((long long)(100000000.0/t3)) + " gates/sec");
        
        // ============ TEST 4: Lucas (1M) ============
        LOG_INFO("Test 4/5: Lucas Commitments (1M ops)...");
        auto start4 = std::chrono::high_resolution_clock::now();
        long long correct4 = 0;
        for (long long n = 1; n <= 1000000; n++) {
            long long commitment = gps.commit_value(n);
            if (gps.verify_commitment(n, commitment)) correct4++;
            if (n % 100000 == 0) {
                auto now = std::chrono::high_resolution_clock::now();
                double elapsed = std::chrono::duration<double>(now - start4).count();
                LOG_INFO("  Lucas Progress: " + std::to_string(n/1000) + "K/1M");
            }
        }
        auto end4 = std::chrono::high_resolution_clock::now();
        double t4 = std::chrono::duration<double>(end4 - start4).count();
        LOG_INFO("  Lucas: " + std::to_string(correct4) + "/1M correct, " +
                 std::to_string((long long)(1000000.0/t4)) + " commits/sec");
        
        // ============ TEST 5: PRNG (10M) ============
        LOG_INFO("Test 5/5: Golden Angle PRNG (10M nonces)...");
        auto start5 = std::chrono::high_resolution_clock::now();
        for (long long i = 0; i < 10000000; i++) {
            gps.encrypt_data(false, 0);
            if ((i + 1) % 1000000 == 0) {
                auto now = std::chrono::high_resolution_clock::now();
                double elapsed = std::chrono::duration<double>(now - start5).count();
                LOG_INFO("  PRNG Progress: " + std::to_string((i+1)/1000000) + "M/10M, " +
                         std::to_string((long long)((i+1)/elapsed)) + " enc/sec");
            }
        }
        auto end5 = std::chrono::high_resolution_clock::now();
        double t5 = std::chrono::duration<double>(end5 - start5).count();
        LOG_INFO("  PRNG: " + std::to_string((long long)(10000000.0/t5)) + " enc/sec");
        
        // ============ SUMMARY ============
        LOG_INFO("=== FINAL SUMMARY ===");
        LOG_INFO("  1. iO:      " + std::to_string((long long)(100000000.0/t1)) + " ops/sec");
        LOG_INFO("  2. Batch:   " + std::to_string((long long)(total_bits/t2)) + " bits/sec");
        LOG_INFO("  3. Quantum: " + std::to_string((long long)(100000000.0/t3)) + " gates/sec");
        LOG_INFO("  4. Lucas:   " + std::to_string((long long)(1000000.0/t4)) + " commits/sec");
        LOG_INFO("  5. PRNG:    " + std::to_string((long long)(10000000.0/t5)) + " enc/sec");
        
        gps.print_security();
        LOG_INFO("✅ 100M BENCHMARK COMPLETE");
        
    } catch (const std::exception& e) {
        LOG_ERROR(std::string("Error: ") + e.what());
        return 1;
    }
    
    GoldenLogger::close();
    return 0;
}
