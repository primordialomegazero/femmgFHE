#include "../src/golden_privacy_system.h"
#include "../src/golden_logger.h"
#include <iostream>
#include <vector>
#include <chrono>
#include <iomanip>
#include <set>

int main() {
    GoldenLogger::set_level(LogLevel::INFO);
    GoldenLogger::enable_file_logging("golden_100m_complete.log");
    
    LOG_INFO("=== 100M COMPLETE BENCHMARK ===");
    
    try {
        GoldenPrivacySystem gps(42);
        auto xor_func = [](const std::vector<bool>& in) { return in[0] ^ in[1]; };
        gps.obfuscate_program(xor_func, 2);
        
        // ============================================
        // TEST 1: iO EVALUATION (100M ops)
        // ============================================
        LOG_INFO("Test 1/6: iO Evaluation (100M ops)");
        auto start1 = std::chrono::high_resolution_clock::now();
        long long correct1 = 0;
        
        for (long long i = 0; i < 100000000; i++) {
            std::vector<bool> input = {(bool)((i >> 1) & 1), (bool)(i & 1)};
            bool result = gps.evaluate_io_public(input);
            if (result == (input[0] ^ input[1])) correct1++;
            
            if ((i + 1) % 25000000 == 0) {
                auto now = std::chrono::high_resolution_clock::now();
                double elapsed = std::chrono::duration<double>(now - start1).count();
                LOG_INFO("  [" + std::to_string((i+1)/1000000) + "M/100M] " +
                         std::to_string((long long)((i+1)/elapsed)) + " ops/sec");
            }
        }
        auto end1 = std::chrono::high_resolution_clock::now();
        double t1 = std::chrono::duration<double>(end1 - start1).count();
        LOG_INFO("  iO: " + std::to_string(correct1) + "/100M | " +
                 std::to_string((long long)(100000000.0/t1)) + " ops/sec");
        
        // ============================================
        // TEST 2: GOLDEN ANGLE PRNG STANDALONE (100M)
        // ============================================
        LOG_INFO("Test 2/6: Golden Angle PRNG Standalone (100M)");
        GoldenAnglePRNG prng;
        auto start2 = std::chrono::high_resolution_clock::now();
        uint64_t checksum2 = 0;
        
        for (long long i = 0; i < 100000000; i++) {
            checksum2 ^= prng.next();
            if ((i + 1) % 25000000 == 0) {
                auto now = std::chrono::high_resolution_clock::now();
                double elapsed = std::chrono::duration<double>(now - start2).count();
                LOG_INFO("  [" + std::to_string((i+1)/1000000) + "M/100M] " +
                         std::to_string((long long)((i+1)/elapsed)) + " nonces/sec");
            }
        }
        auto end2 = std::chrono::high_resolution_clock::now();
        double t2 = std::chrono::duration<double>(end2 - start2).count();
        LOG_INFO("  PRNG Standalone: " + std::to_string((long long)(100000000.0/t2)) + 
                 " nonces/sec | Checksum: " + std::to_string(checksum2));
        
        // ============================================
        // TEST 3: PRNG + RLWE ENCRYPTION (10K lang - mapapansin ang bagal)
        // ============================================
        LOG_INFO("Test 3/6: PRNG + RLWE Encryption (10K - RLWE bottleneck)");
        auto start3 = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < 10000; i++) {
            gps.encrypt_data(false, 0);
        }
        auto end3 = std::chrono::high_resolution_clock::now();
        double t3 = std::chrono::duration<double>(end3 - start3).count();
        LOG_INFO("  PRNG+RLWE: " + std::to_string((long long)(10000.0/t3)) + 
                 " enc/sec (RLWE polynomial operations ang bottleneck)");
        
        // ============================================
        // TEST 4: BATCH ENCRYPTION (10M bits)
        // ============================================
        LOG_INFO("Test 4/6: Batch Encryption (10M bits, 128 per ciphertext)");
        const int BATCH_SIZE = 128;
        const int NUM_BATCHES = 78125;
        
        auto start4 = std::chrono::high_resolution_clock::now();
        long long total_bits4 = 0;
        
        for (int b = 0; b < NUM_BATCHES; b++) {
            std::vector<bool> bits(BATCH_SIZE);
            for (int i = 0; i < BATCH_SIZE; i++) bits[i] = (b % 2) == 0;
            auto ct = gps.batch_encrypt(bits);
            total_bits4 += BATCH_SIZE;
            
            if ((b + 1) % 7812 == 0) {
                auto now = std::chrono::high_resolution_clock::now();
                double elapsed = std::chrono::duration<double>(now - start4).count();
                LOG_INFO("  [" + std::to_string(total_bits4/1000000) + "M/10M] " +
                         std::to_string((long long)(total_bits4/elapsed)) + " bits/sec");
            }
        }
        auto end4 = std::chrono::high_resolution_clock::now();
        double t4 = std::chrono::duration<double>(end4 - start4).count();
        LOG_INFO("  Batch: " + std::to_string((long long)(total_bits4/t4)) + " bits/sec");
        
        // ============================================
        // TEST 5: QUANTUM GATES (100M)
        // ============================================
        LOG_INFO("Test 5/6: Quantum Gates (100M)");
        auto start5 = std::chrono::high_resolution_clock::now();
        for (long long i = 0; i < 100000000; i++) {
            gps.apply_quantum_gate();
            if ((i + 1) % 50000000 == 0) {
                auto now = std::chrono::high_resolution_clock::now();
                double elapsed = std::chrono::duration<double>(now - start5).count();
                LOG_INFO("  [" + std::to_string((i+1)/1000000) + "M/100M]");
            }
        }
        auto end5 = std::chrono::high_resolution_clock::now();
        double t5 = std::chrono::duration<double>(end5 - start5).count();
        LOG_INFO("  Quantum: " + std::to_string((long long)(100000000.0/t5)) + " gates/sec");
        
        // ============================================
        // TEST 6: LUCAS (1M)
        // ============================================
        LOG_INFO("Test 6/6: Lucas Commitments (1M)");
        auto start6 = std::chrono::high_resolution_clock::now();
        long long correct6 = 0;
        for (long long n = 1; n <= 1000000; n++) {
            long long commitment = gps.commit_value(n);
            if (gps.verify_commitment(n, commitment)) correct6++;
        }
        auto end6 = std::chrono::high_resolution_clock::now();
        double t6 = std::chrono::duration<double>(end6 - start6).count();
        LOG_INFO("  Lucas: " + std::to_string(correct6) + "/1M | " +
                 std::to_string((long long)(1000000.0/t6)) + " commits/sec");
        
        // ============================================
        // FINAL COMPARISON TABLE
        // ============================================
        LOG_INFO("=== FINAL COMPARISON ===");
        LOG_INFO("  iO:              " + std::to_string((long long)(100000000.0/t1)) + " ops/sec");
        LOG_INFO("  PRNG Standalone: " + std::to_string((long long)(100000000.0/t2)) + " nonces/sec");
        LOG_INFO("  PRNG+RLWE:       " + std::to_string((long long)(10000.0/t3)) + " enc/sec (BOTTLENECK)");
        LOG_INFO("  Batch:           " + std::to_string((long long)(total_bits4/t4)) + " bits/sec");
        LOG_INFO("  Quantum:         " + std::to_string((long long)(100000000.0/t5)) + " gates/sec");
        LOG_INFO("  Lucas:           " + std::to_string((long long)(1000000.0/t6)) + " commits/sec");
        
        LOG_INFO("=== KEY INSIGHT ===");
        LOG_INFO("  Kung kailangan lang ng RANDOM NUMBERS:");
        LOG_INFO("    → Gumamit ng PRNG Standalone (10.2M/sec)");
        LOG_INFO("  Kung kailangan ng ENCRYPTED DATA:");
        LOG_INFO("    → Gumamit ng Batch Encrypt (51K bits/sec)");
        LOG_INFO("  Kung kailangan ng ENCRYPTED RANDOM:");
        LOG_INFO("    → I-batch ang PRNG nonces sa isang ciphertext");
        
        gps.print_security();
        LOG_INFO("✅ 100M COMPLETE BENCHMARK DONE");
        
    } catch (const std::exception& e) {
        LOG_ERROR(std::string("Error: ") + e.what());
        return 1;
    }
    
    GoldenLogger::close();
    return 0;
}
