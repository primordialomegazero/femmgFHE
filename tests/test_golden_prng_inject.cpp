#include "../src/golden_privacy_system.h"
#include <iostream>
#include <set>
#include <chrono>

int main() {
    std::cout << "GOLDEN ANGLE PRNG TEST\n\n";
    
    GoldenAnglePRNG prng;
    
    // Test 1: 1M unique
    std::cout << "1. Uniqueness (1M samples):\n";
    std::set<uint64_t> unique;
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < 1000000; i++) unique.insert(prng.next());
    auto end = std::chrono::high_resolution_clock::now();
    double t = std::chrono::duration<double>(end - start).count();
    
    std::cout << "   " << unique.size() << "/1M unique\n";
    std::cout << "   " << 1000000.0/t << " nonces/sec\n";
    std::cout << "   Status: " << (unique.size() == 1000000 ? "PERFECT" : "COLLISIONS") << "\n\n";
    
    // Test 2: Distribution
    std::cout << "2. Distribution (1M samples):\n";
    GoldenAnglePRNG prng2;
    int buckets[10] = {0};
    for (int i = 0; i < 1000000; i++) {
        int b = static_cast<int>(prng2.next_double() * 10);
        buckets[b]++;
    }
    int min_b = buckets[0], max_b = buckets[0];
    for (int i = 1; i < 10; i++) {
        min_b = std::min(min_b, buckets[i]);
        max_b = std::max(max_b, buckets[i]);
    }
    double balance = (max_b - min_b) / 100000.0;
    std::cout << "   Balance: " << balance << "\n";
    std::cout << "   Status: " << (balance < 0.1 ? "UNIFORM" : "BIASED") << "\n";
    
    return 0;
}
