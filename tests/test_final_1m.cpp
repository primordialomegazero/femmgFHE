#include "../src/golden_privacy_system.h"
#include <iostream>
#include <vector>
#include <chrono>
#include <iomanip>

int main() {
    std::cout << "========================================\n";
    std::cout << "FINAL BENCHMARK: 1M FHE OPERATIONS\n";
    std::cout << "========================================\n\n";
    
    GoldenPrivacySystem gps(42);
    
    auto xor_func = [](const std::vector<bool>& in) {
        return in[0] ^ in[1];
    };
    gps.obfuscate_program(xor_func, 2);
    
    // ========== TEST 1: iO EVALUATION (1M) ==========
    std::cout << "1. iO EVALUATION (1M ops)\n";
    std::cout << "   Obfuscated: XOR function\n";
    
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
    
    std::cout << "   Correct: " << correct1 << "/1,000,000\n";
    std::cout << "   Time: " << t1 << " s\n";
    std::cout << "   Throughput: " << std::fixed << std::setprecision(0) 
              << 1000000.0 / t1 << " ops/sec\n\n";
    
    // ========== TEST 2: BATCH ENCRYPTION (1M bits) ==========
    std::cout << "2. BATCH ENCRYPTION (1M bits)\n";
    
    const int BATCH_SIZE = 128;
    const int NUM_BATCHES = 7813;  // 128 * 7813 ≈ 1M
    
    auto start2 = std::chrono::high_resolution_clock::now();
    int total_bits = 0;
    int correct2 = 0;
    
    for (int b = 0; b < NUM_BATCHES; b++) {
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
    }
    
    auto end2 = std::chrono::high_resolution_clock::now();
    double t2 = std::chrono::duration<double>(end2 - start2).count();
    
    std::cout << "   Correct: " << correct2 << "/" << total_bits << "\n";
    std::cout << "   Time: " << t2 << " s\n";
    std::cout << "   Throughput: " << std::fixed << std::setprecision(0) 
              << total_bits / t2 << " bits/sec\n\n";
    
    // ========== TEST 3: QUANTUM GATES (1M) ==========
    std::cout << "3. QUANTUM GATES (1M ops)\n";
    
    auto start3 = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < 1000000; i++) {
        gps.apply_quantum_gate();
    }
    auto end3 = std::chrono::high_resolution_clock::now();
    double t3 = std::chrono::duration<double>(end3 - start3).count();
    
    std::cout << "   Time: " << t3 << " s\n";
    std::cout << "   Throughput: " << std::fixed << std::setprecision(0) 
              << 1000000.0 / t3 << " gates/sec\n\n";
    
    // ========== TEST 4: LUCAS COMMITMENTS (100K) ==========
    std::cout << "4. LUCAS COMMITMENTS (100K ops)\n";
    
    auto start4 = std::chrono::high_resolution_clock::now();
    int correct4 = 0;
    
    for (long long n = 1; n <= 100000; n++) {
        long long commitment = gps.commit_value(n);
        if (gps.verify_commitment(n, commitment)) correct4++;
    }
    
    auto end4 = std::chrono::high_resolution_clock::now();
    double t4 = std::chrono::duration<double>(end4 - start4).count();
    
    std::cout << "   Correct: " << correct4 << "/100,000\n";
    std::cout << "   Time: " << t4 << " s\n";
    std::cout << "   Throughput: " << std::fixed << std::setprecision(0) 
              << 100000.0 / t4 << " commits/sec\n\n";
    
    // ========== TEST 5: GOLDEN ANGLE PRNG (1M) ==========
    std::cout << "5. GOLDEN ANGLE PRNG (1M nonces)\n";
    
    auto start5 = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < 1000000; i++) {
        gps.encrypt_data(false, 0);  // Golden Angle PRNG nonce
    }
    auto end5 = std::chrono::high_resolution_clock::now();
    double t5 = std::chrono::duration<double>(end5 - start5).count();
    
    std::cout << "   Time: " << t5 << " s\n";
    std::cout << "   Throughput: " << std::fixed << std::setprecision(0) 
              << 1000000.0 / t5 << " encryptions/sec\n\n";
    
    // ========== SUMMARY ==========
    std::cout << "========================================\n";
    std::cout << "FINAL BENCHMARK SUMMARY\n";
    std::cout << "========================================\n";
    std::cout << "1. iO Evaluation:     " << std::setw(10) << std::fixed << std::setprecision(0) 
              << 1000000.0/t1 << " ops/sec  (" << correct1 << "/1M correct)\n";
    std::cout << "2. Batch Encryption:  " << std::setw(10) << total_bits/t2 
              << " bits/sec (" << correct2 << "/" << total_bits << " correct)\n";
    std::cout << "3. Quantum Gates:     " << std::setw(10) << 1000000.0/t3 
              << " gates/sec\n";
    std::cout << "4. Lucas Commit:      " << std::setw(10) << 100000.0/t4 
              << " commits/sec (" << correct4 << "/100K correct)\n";
    std::cout << "5. PRNG Encryption:   " << std::setw(10) << 1000000.0/t5 
              << " enc/sec\n";
    std::cout << "========================================\n";
    
    gps.print_security();
    
    std::cout << "\n✅ FINAL BENCHMARK COMPLETE!\n";
    
    return 0;
}
