#include "../src/golden_privacy_system.h"
#include "../src/golden_logger.h"
#include "../src/golden_error.h"
#include <iostream>
#include <vector>
#include <chrono>
#include <iomanip>

int main() {
    GoldenLogger::set_level(LogLevel::INFO);
    GoldenLogger::enable_file_logging("golden_1m.log");
    GoldenLogger::install_signal_handlers();
    
    LOG_INFO("=== FINAL BENCHMARK: 1M FHE OPERATIONS ===");
    
    try {
        GOLDEN_CHECK_NOT_SHUTDOWN();
        GoldenPrivacySystem gps(42);
        
        auto xor_func = [](const std::vector<bool>& in) {
            return in[0] ^ in[1];
        };
        gps.obfuscate_program(xor_func, 2);
        LOG_INFO("System initialized");
        
        // ========== TEST 1: iO EVALUATION ==========
        LOG_INFO("Test 1/5: iO Evaluation (1M ops)...");
        auto start1 = std::chrono::high_resolution_clock::now();
        int correct1 = 0;
        
        for (int i = 0; i < 1000000; i++) {
            std::vector<bool> input = {(bool)((i >> 1) & 1), (bool)(i & 1)};
            bool result = gps.evaluate_io_public(input);
            bool expected = input[0] ^ input[1];
            if (result == expected) correct1++;
        }
        
        auto end1 = std::chrono::high_resolution_clock::now();
        double t1 = std::chrono::duration<double>(end1 - start1).count();
        LOG_INFO("  iO: " + std::to_string(correct1) + "/1M correct, " + 
                 std::to_string((int)(1000000.0/t1)) + " ops/sec");
        
        // ========== TEST 2: BATCH ENCRYPTION ==========
        LOG_INFO("Test 2/5: Batch Encryption (1M bits)...");
        const int BATCH_SIZE = 128;
        const int NUM_BATCHES = 7813;
        
        auto start2 = std::chrono::high_resolution_clock::now();
        int total_bits = 0;
        int correct2 = 0;
        
        for (int b = 0; b < NUM_BATCHES; b++) {
            if (GoldenLogger::is_shutting_down()) {
                LOG_WARN("Shutdown requested - stopping batch encryption");
                break;
            }
            
            std::vector<bool> bits(BATCH_SIZE);
            for (int i = 0; i < BATCH_SIZE; i++) {
                bits[i] = ((b * BATCH_SIZE + i) % 2) == 0;
            }
            
            auto ct = gps.batch_encrypt(bits);
            auto decoded = gps.batch_decrypt(ct, BATCH_SIZE);
            
            for (int i = 0; i < BATCH_SIZE; i++) {
                if (decoded[i] == bits[i]) correct2++;
                total_bits++;
            }
            
            // Progress every 50K bits (~390 batches)
            if ((b + 1) % 391 == 0) {
                auto now = std::chrono::high_resolution_clock::now();
                double elapsed = std::chrono::duration<double>(now - start2).count();
                LOG_INFO("  Progress: " + std::to_string(total_bits) + "/1,000,064 bits, " +
                         std::to_string((int)(total_bits/elapsed)) + " bits/sec");
            }
        }
        
        auto end2 = std::chrono::high_resolution_clock::now();
        double t2 = std::chrono::duration<double>(end2 - start2).count();
        LOG_INFO("  Batch: " + std::to_string(correct2) + "/" + std::to_string(total_bits) + 
                 " correct, " + std::to_string((int)(total_bits/t2)) + " bits/sec");
        
        // ========== TEST 3: QUANTUM GATES ==========
        LOG_INFO("Test 3/5: Quantum Gates (1M ops)...");
        auto start3 = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < 1000000; i++) {
            gps.apply_quantum_gate();
        }
        auto end3 = std::chrono::high_resolution_clock::now();
        double t3 = std::chrono::duration<double>(end3 - start3).count();
        LOG_INFO("  Quantum: " + std::to_string((int)(1000000.0/t3)) + " gates/sec");
        
        // ========== TEST 4: LUCAS ==========
        LOG_INFO("Test 4/5: Lucas Commitments (100K ops)...");
        auto start4 = std::chrono::high_resolution_clock::now();
        int correct4 = 0;
        for (long long n = 1; n <= 100000; n++) {
            long long commitment = gps.commit_value(n);
            if (gps.verify_commitment(n, commitment)) correct4++;
        }
        auto end4 = std::chrono::high_resolution_clock::now();
        double t4 = std::chrono::duration<double>(end4 - start4).count();
        LOG_INFO("  Lucas: " + std::to_string(correct4) + "/100K correct, " +
                 std::to_string((int)(100000.0/t4)) + " commits/sec");
        
        // ========== TEST 5: PRNG ==========
        LOG_INFO("Test 5/5: Golden Angle PRNG (1M nonces)...");
        auto start5 = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < 1000000; i++) {
            gps.encrypt_data(false, 0);
            if ((i + 1) % 50000 == 0) {
                auto now = std::chrono::high_resolution_clock::now();
                double elapsed = std::chrono::duration<double>(now - start5).count();
                LOG_INFO("  Progress: " + std::to_string(i + 1) + "/1M, " +
                         std::to_string((int)((i+1)/elapsed)) + " enc/sec");
            }
        }
        auto end5 = std::chrono::high_resolution_clock::now();
        double t5 = std::chrono::duration<double>(end5 - start5).count();
        LOG_INFO("  PRNG: " + std::to_string((int)(1000000.0/t5)) + " enc/sec");
        
        // ========== SUMMARY ==========
        LOG_INFO("=== FINAL SUMMARY ===");
        LOG_INFO("  1. iO:      " + std::to_string((int)(1000000.0/t1)) + " ops/sec");
        LOG_INFO("  2. Batch:   " + std::to_string((int)(total_bits/t2)) + " bits/sec");
        LOG_INFO("  3. Quantum: " + std::to_string((int)(1000000.0/t3)) + " gates/sec");
        LOG_INFO("  4. Lucas:   " + std::to_string((int)(100000.0/t4)) + " commits/sec");
        LOG_INFO("  5. PRNG:    " + std::to_string((int)(1000000.0/t5)) + " enc/sec");
        
        gps.print_security();
        LOG_INFO("✅ BENCHMARK COMPLETE");
        
    } catch (const GoldenException& e) {
        LOG_ERROR(std::string("Fatal: ") + e.what());
        return 1;
    }
    
    GoldenLogger::close();
    return 0;
}
