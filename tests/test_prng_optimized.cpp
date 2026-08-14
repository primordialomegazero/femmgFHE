#include "../src/golden_privacy_system.h"
#include "../src/golden_logger.h"
#include <iostream>
#include <chrono>

int main() {
    std::cout << "PRNG OPTIMIZATION RESEARCH\n\n";
    
    GoldenPrivacySystem gps(42);
    
    // Test 1: Current slow path (full encryption per nonce)
    std::cout << "1. Current: encrypt_data(false, 0) - full RLWE encryption\n";
    auto start1 = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < 10000; i++) {
        gps.encrypt_data(false, 0);
    }
    auto end1 = std::chrono::high_resolution_clock::now();
    double t1 = std::chrono::duration<double>(end1 - start1).count();
    std::cout << "   10K encryptions: " << t1 << " s → " << 10000.0/t1 << " enc/sec\n\n";
    
    // Test 2: Direct PRNG call (walang encryption)
    std::cout << "2. Direct Golden Angle PRNG (walang encryption):\n";
    GoldenAnglePRNG prng;
    auto start2 = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < 1000000; i++) {
        prng.next();
    }
    auto end2 = std::chrono::high_resolution_clock::now();
    double t2 = std::chrono::duration<double>(end2 - start2).count();
    std::cout << "   1M PRNG calls: " << t2 << " s → " << 1000000.0/t2 << " nonces/sec\n\n";
    
    // Test 3: PRNG + simple bit (walang RLWE)
    std::cout << "3. PRNG + bit packing (walang RLWE):\n";
    auto start3 = std::chrono::high_resolution_clock::now();
    uint64_t result = 0;
    for (int i = 0; i < 1000000; i++) {
        result ^= prng.next();
    }
    auto end3 = std::chrono::high_resolution_clock::now();
    double t3 = std::chrono::duration<double>(end3 - start3).count();
    std::cout << "   1M XOR ops: " << t3 << " s → " << 1000000.0/t3 << " ops/sec\n\n";
    
    std::cout << "=== ANALYSIS ===\n";
    std::cout << "Bottleneck: RLWE encryption mismo (polynomial ops N=1024)\n";
    std::cout << "Solution: i-batch ang PRNG nonces sa isang ciphertext\n";
    std::cout << "O kaya: gumamit ng direct PRNG output na walang RLWE\n";
    
    return 0;
}
