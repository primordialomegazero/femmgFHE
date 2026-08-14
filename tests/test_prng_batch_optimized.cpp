#include "../src/golden_prng.h"
#include <iostream>
#include <chrono>
#include <set>

int main() {
    std::cout << "PRNG BATCH OPTIMIZATION TEST\n\n";
    
    // Test 1: Batch PRNG speed
    std::cout << "1. Batch-Optimized PRNG:\n";
    GoldenAnglePRNG prng;
    
    auto start1 = std::chrono::high_resolution_clock::now();
    uint64_t checksum1 = 0;
    for (int i = 0; i < 1000000; i++) {
        checksum1 ^= prng.next();
    }
    auto end1 = std::chrono::high_resolution_clock::now();
    double t1 = std::chrono::duration<double>(end1 - start1).count();
    
    std::cout << "   1M nonces: " << t1 << " s → " << 1000000.0/t1 << " nonces/sec\n\n";
    
    // Test 2: Direct computation (comparison)
    std::cout << "2. Direct computation (walang batch):\n";
    GoldenAnglePRNG prng_direct;
    
    auto start2 = std::chrono::high_resolution_clock::now();
    uint64_t checksum2 = 0;
    for (int i = 0; i < 1000000; i++) {
        checksum2 ^= prng_direct.next_direct();
    }
    auto end2 = std::chrono::high_resolution_clock::now();
    double t2 = std::chrono::duration<double>(end2 - start2).count();
    
    std::cout << "   1M direct: " << t2 << " s → " << 1000000.0/t2 << " nonces/sec\n\n";
    
    // Test 3: Uniqueness
    std::cout << "3. Uniqueness check:\n";
    GoldenAnglePRNG prng_unique;
    std::set<uint64_t> unique;
    for (int i = 0; i < 100000; i++) {
        unique.insert(prng_unique.next());
    }
    std::cout << "   100K nonces: " << unique.size() << " unique\n";
    std::cout << "   Status: " << (unique.size() == 100000 ? "PERFECT ✅" : "COLLISIONS ❌") << "\n\n";
    
    // Test 4: Batch API
    std::cout << "4. Batch API (128 nonces at once):\n";
    GoldenAnglePRNG prng_batch;
    auto start3 = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < 1000; i++) {
        auto batch = prng_batch.next_batch(128);
    }
    auto end3 = std::chrono::high_resolution_clock::now();
    double t3 = std::chrono::duration<double>(end3 - start3).count();
    
    std::cout << "   1000 batches (128K nonces): " << t3 << " s\n";
    std::cout << "   " << 128000.0/t3 << " nonces/sec\n";
    
    return 0;
}
