#include "../src/golden_privacy_system.h"
#include <iostream>
#include <chrono>
#include <set>

int main() {
    std::cout << "PRNG FINAL OPTIMIZED TEST\n\n";
    
    // Standalone PRNG (walang RLWE)
    GoldenAnglePRNG prng;
    
    // Test 1: 100M nonces
    std::cout << "100M Golden Angle nonces (standalone):\n";
    auto start = std::chrono::high_resolution_clock::now();
    uint64_t checksum = 0;
    
    for (long long i = 0; i < 100000000; i++) {
        checksum ^= prng.next();
        if ((i + 1) % 10000000 == 0) {
            auto now = std::chrono::high_resolution_clock::now();
            double elapsed = std::chrono::duration<double>(now - start).count();
            std::cout << "  [" << (i+1)/1000000 << "M/100M] " 
                      << (long long)((i+1)/elapsed) << " nonces/sec\n";
        }
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    double t = std::chrono::duration<double>(end - start).count();
    
    std::cout << "\n  Total: 100M nonces sa " << t << " s\n";
    std::cout << "  Throughput: " << (long long)(100000000.0/t) << " nonces/sec\n";
    std::cout << "  Checksum: " << checksum << " (non-zero)\n\n";
    
    // Test 2: Uniqueness
    std::cout << "Uniqueness check (1M samples):\n";
    GoldenAnglePRNG prng_unique;
    std::set<uint64_t> unique;
    for (int i = 0; i < 1000000; i++) {
        unique.insert(prng_unique.next());
    }
    std::cout << "  " << unique.size() << "/1M unique\n";
    std::cout << "  Status: " << (unique.size() == 1000000 ? "PERFECT ✅" : "COLLISIONS ❌") << "\n\n";
    
    // Test 3: Distribution
    std::cout << "Distribution check (1M samples):\n";
    GoldenAnglePRNG prng_dist;
    int buckets[10] = {0};
    for (int i = 0; i < 1000000; i++) {
        int bucket = static_cast<int>(prng_dist.next_double() * 10);
        buckets[bucket]++;
    }
    int min_b = buckets[0], max_b = buckets[0];
    for (int i = 1; i < 10; i++) {
        min_b = std::min(min_b, buckets[i]);
        max_b = std::max(max_b, buckets[i]);
    }
    double balance = (max_b - min_b) / 100000.0;
    std::cout << "  Balance: " << balance << " (0 = perfect)\n";
    std::cout << "  Status: " << (balance < 0.1 ? "UNIFORM ✅" : "BIASED ❌") << "\n";
    
    return 0;
}
