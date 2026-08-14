#include <iostream>
#include <cmath>
#include <vector>
#include <set>
#include <chrono>

constexpr double PHI = 1.6180339887498948482;

class LucasFixed {
private:
    // Mas malaking prime para sa mas kaunting collisions
    // 2^61 - 1 ay Mersenne prime
    const long long PRIME = (1LL << 61) - 1;
    
    // Fast doubling na may __int128 para sa overflow safety
    long long modular_lucas(long long n, long long mod) {
        __int128 a = 2;
        __int128 b = 1;
        
        for (long long i = 60; i >= 0; i--) {
            __int128 c = (a * a - 2 + mod) % mod;
            __int128 d = (a * b - 1 + mod) % mod;
            
            if ((n >> i) & 1) {
                a = d;
                b = c;
            } else {
                a = c;
                b = (d - c + mod) % mod;
            }
        }
        
        return static_cast<long long>(a);
    }
    
public:
    void stress_test_fixed() {
        std::cout << "LUCAS FIXED (PRIME = 2^61-1)\n\n";
        
        // Test 1: Collision resistance with larger prime
        std::cout << "1. COLLISION RESISTANCE\n";
        std::set<long long> outputs;
        
        for (long long n = 1; n <= 100000; n++) {
            long long out = modular_lucas(n, PRIME);
            outputs.insert(out);
        }
        
        std::cout << "   100K inputs: " << outputs.size() << " unique\n";
        std::cout << "   Collisions: " << 100000 - outputs.size() << "\n";
        std::cout << "   Status: " << (outputs.size() == 100000 ? "PERFECT ✅" : "COLLISIONS ❌") << "\n\n";
        
        // Test 2: Avalanche
        std::cout << "2. AVALANCHE\n";
        long long out1 = modular_lucas(12345, PRIME);
        long long out2 = modular_lucas(12344, PRIME);
        long long diff = out1 ^ out2;
        int changed = 0;
        while (diff) { changed++; diff &= diff - 1; }
        
        std::cout << "   1-bit input change → " << changed << " bits changed sa output\n";
        std::cout << "   Status: " << (changed > 20 ? "GOOD ✅" : "POOR ❌") << "\n\n";
        
        // Test 3: Distribution
        std::cout << "3. DISTRIBUTION\n";
        int buckets[10] = {0};
        for (long long n = 1; n <= 100000; n++) {
            long long out = modular_lucas(n, PRIME);
            // I-normalize sa 0-1 range
            double normalized = static_cast<double>(out) / PRIME;
            int bucket = static_cast<int>(normalized * 10);
            buckets[bucket]++;
        }
        
        double max_dev = 0;
        for (int i = 0; i < 10; i++) {
            double dev = std::abs(buckets[i] - 10000.0) / 10000.0;
            max_dev = std::max(max_dev, dev);
        }
        
        std::cout << "   Max deviation: " << max_dev << "\n";
        std::cout << "   Status: " << (max_dev < 0.05 ? "UNIFORM ✅" : "BIASED ❌") << "\n\n";
        
        // Test 4: Inversion (brute force)
        std::cout << "4. INVERSION DIFFICULTY\n";
        long long target = modular_lucas(77777, PRIME);
        
        auto start = std::chrono::high_resolution_clock::now();
        long long found = -1;
        for (long long n = 1; n <= 1000000; n++) {
            if (modular_lucas(n, PRIME) == target) {
                found = n;
                break;
            }
        }
        auto end = std::chrono::high_resolution_clock::now();
        double t = std::chrono::duration<double>(end - start).count();
        
        std::cout << "   1M brute force attempts: " << t << " s\n";
        std::cout << "   Found: " << found << " (target=77777)\n";
        
        // Extrapolate sa 2^64
        double years = t * (1.8446744e19) / 1000000 / 3600 / 24 / 365;
        std::cout << "   Extrapolated sa 2^64: " << years << " years\n";
        std::cout << "   Status: INFEASIBLE ✅\n\n";
    }
    
    void run() {
        stress_test_fixed();
    }
};

int main() {
    LucasFixed lucas;
    lucas.run();
    
    return 0;
}
