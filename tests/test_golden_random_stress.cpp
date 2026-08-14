#include <iostream>
#include <cmath>
#include <set>
#include <random>
#include <chrono>

constexpr double PHI = 1.6180339887498948482;
constexpr double PI = 3.14159265358979323846;

int main() {
    std::cout << "STRESS TEST: Golden Randomness (3 Candidates)\n\n";
    
    // ========== CANDIDATE 1: Golden Angle ==========
    std::cout << "=== CANDIDATE 1: Golden Angle ===\n";
    double golden_angle = 2.0 * PI / PHI;
    
    // Test 1M iterations
    {
        std::set<long> unique;
        auto start = std::chrono::high_resolution_clock::now();
        
        for (int i = 0; i < 1000000; i++) {
            double val = std::fmod(i * golden_angle, 2.0 * PI);
            long scaled = static_cast<long>(val * 1000000);
            unique.insert(scaled);
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        double t = std::chrono::duration<double>(end - start).count();
        
        std::cout << "  1M iterations: " << unique.size() << " unique\n";
        std::cout << "  Time: " << t << " s\n";
        std::cout << "  " << unique.size() / t << " unique/sec\n";
        std::cout << "  Status: " << (unique.size() == 1000000 ? "PERFECT ✅" : "COLLISIONS ❌") << "\n\n";
    }
    
    // ========== CANDIDATE 2: PHI-base Expansion ==========
    std::cout << "=== CANDIDATE 2: PHI-base Expansion ===\n";
    
    {
        std::set<long> unique;
        auto start = std::chrono::high_resolution_clock::now();
        
        double x = 0.123456789;
        for (int i = 0; i < 1000000; i++) {
            x = std::fmod(x * PHI, 1.0);
            long scaled = static_cast<long>(x * 1000000);
            unique.insert(scaled);
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        double t = std::chrono::duration<double>(end - start).count();
        
        std::cout << "  1M iterations: " << unique.size() << " unique\n";
        std::cout << "  Time: " << t << " s\n";
        std::cout << "  " << unique.size() / t << " unique/sec\n";
        std::cout << "  Status: " << (unique.size() == 1000000 ? "PERFECT ✅" : "COLLISIONS ❌") << "\n\n";
    }
    
    // ========== CANDIDATE 3: Golden-xorshift Hybrid ==========
    std::cout << "=== CANDIDATE 3: Golden-xorshift Hybrid ===\n";
    
    {
        std::set<uint64_t> unique;
        auto start = std::chrono::high_resolution_clock::now();
        
        uint64_t state = 42;
        uint64_t golden_mask = static_cast<uint64_t>(PHI * 1e18);
        
        for (int i = 0; i < 1000000; i++) {
            state ^= (state << 13);
            state ^= (state >> 7);
            state ^= (state << 17);
            state = state ^ golden_mask;
            unique.insert(state);
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        double t = std::chrono::duration<double>(end - start).count();
        
        std::cout << "  1M iterations: " << unique.size() << " unique\n";
        std::cout << "  Time: " << t << " s\n";
        std::cout << "  " << unique.size() / t << " unique/sec\n";
        std::cout << "  Status: " << (unique.size() == 1000000 ? "PERFECT ✅" : "COLLISIONS ❌") << "\n\n";
    }
    
    // ========== STATISTICAL ANALYSIS ==========
    std::cout << "=== STATISTICAL ANALYSIS ===\n\n";
    
    // Test 1: Golden Angle distribution
    {
        int buckets[10] = {0};
        double golden_angle = 2.0 * PI / PHI;
        
        for (int i = 0; i < 100000; i++) {
            double val = std::fmod(i * golden_angle, 2.0 * PI);
            int bucket = static_cast<int>(val / (2.0 * PI / 10));
            buckets[bucket]++;
        }
        
        std::cout << "Golden Angle bucket distribution (100K samples):\n";
        for (int i = 0; i < 10; i++) {
            std::cout << "  [" << i * 10 << "%-" << (i+1) * 10 << "%]: " << buckets[i] << "\n";
        }
        std::cout << "\n";
    }
    
    // Test 2: PHI-base distribution
    {
        int buckets[10] = {0};
        double x = 0.987654321;
        
        for (int i = 0; i < 100000; i++) {
            x = std::fmod(x * PHI, 1.0);
            int bucket = static_cast<int>(x * 10);
            buckets[bucket]++;
        }
        
        std::cout << "PHI-base bucket distribution (100K samples):\n";
        for (int i = 0; i < 10; i++) {
            std::cout << "  [" << i * 10 << "%-" << (i+1) * 10 << "%]: " << buckets[i] << "\n";
        }
        std::cout << "\n";
    }
    
    // Test 3: Golden-xorshift distribution
    {
        int buckets[10] = {0};
        uint64_t state = 42;
        uint64_t golden_mask = static_cast<uint64_t>(PHI * 1e18);
        
        for (int i = 0; i < 100000; i++) {
            state ^= (state << 13);
            state ^= (state >> 7);
            state ^= (state << 17);
            state = state ^ golden_mask;
            int bucket = (state >> 61) % 10;
            buckets[bucket]++;
        }
        
        std::cout << "Golden-xorshift bucket distribution (100K samples):\n";
        for (int i = 0; i < 10; i++) {
            std::cout << "  [" << i * 10 << "%-" << (i+1) * 10 << "%]: " << buckets[i] << "\n";
        }
        std::cout << "\n";
    }
    
    return 0;
}
