#include "../src/core/lyapunov_fhe.h"
#include <iostream>
#include <iomanip>
#include <set>
using namespace std;
#include <map>

int main() {
    LyapunovFHE fhe;
    uint64_t base_seed = 0x5ECDA1A5E;
    
    cout << "=== NONCE COLLISION ANALYSIS ===" << endl;
    cout << "Formula: nonce = counter ^ (seed & 0xFFFFFFFF)" << endl;
    cout << "Counter increments per encryption (global atomic)" << endl;
    cout << "Seed = base_seed + i (loop index)" << endl;
    cout << endl;
    
    // Show first 20 nonces with their components
    cout << "First 20 encryptions:" << endl;
    cout << "  i    | seed & 0xFF.. | counter (est) | nonce      |" << endl;
    cout << "  -----|---------------|---------------|------------|" << endl;
    
    set<uint64_t> nonces;
    uint64_t prev_nonce = 0;
    
    for (int i = 0; i < 20; i++) {
        auto ct = fhe.encrypt(0.0, base_seed + i);
        uint64_t nonce = ct.nonce;
        uint64_t seed_low = (base_seed + i) & 0xFFFFFFFFULL;
        
        // Counter starts at 0 and increments globally
        // For fresh LyapunovFHE, first encryption uses counter=0
        // nonce = counter ^ seed_low
        // So counter = nonce ^ seed_low
        uint64_t counter_est = nonce ^ seed_low;
        
        cout << "  " << setw(4) << i << " | "
             << "0x" << hex << setw(8) << setfill('0') << seed_low << dec << " | "
             << "0x" << hex << setw(10) << setfill('0') << counter_est << dec << " | "
             << "0x" << hex << setw(10) << setfill('0') << nonce << dec << " | ";
        
        if (nonces.count(nonce)) cout << "💥 COLLISION!";
        else if (i > 0 && nonce == prev_nonce + 1) cout << "(sequential OK)";
        cout << endl;
        
        nonces.insert(nonce);
        prev_nonce = nonce;
    }
    
    // Now test: how many unique nonces for 10000 encryptions with different seed patterns?
    cout << "\n\n=== UNIQUENESS TESTS ===" << endl;
    
    // Pattern 1: base_seed + i (current behavior)
    {
        set<uint64_t> s;
        for (int i = 0; i < 10000; i++) {
            auto ct = fhe.encrypt(0.0, base_seed + i);
            s.insert(ct.nonce);
        }
        cout << "Pattern 1 (seed+i):    " << s.size() << "/10000 unique" << endl;
    }
    
    // Pattern 2: base_seed ^ (i * large_prime)
    {
        set<uint64_t> s;
        for (int i = 0; i < 10000; i++) {
            auto ct = fhe.encrypt(0.0, base_seed ^ (i * 0x9E3779B97F4A7C15ULL));
            s.insert(ct.nonce);
        }
        cout << "Pattern 2 (seed^hash): " << s.size() << "/10000 unique" << endl;
    }
    
    // Pattern 3: seed=0 (auto-random from chrono)
    {
        set<uint64_t> s;
        for (int i = 0; i < 10000; i++) {
            auto ct = fhe.encrypt(0.0, 0);  // seed=0 → uses chrono
            s.insert(ct.nonce);
        }
        cout << "Pattern 3 (seed=0):    " << s.size() << "/10000 unique" << endl;
    }
    
    // Show the collision pattern visually
    cout << "\n=== COLLISION PATTERN (seed+i for i=0..255) ===" << endl;
    {
        map<uint64_t, int> freq;
        for (int i = 0; i < 256; i++) {
            auto ct = fhe.encrypt(0.0, base_seed + i);
            freq[ct.nonce]++;
        }
        cout << "Unique nonces: " << freq.size() << "/256" << endl;
        cout << "Collision clusters: ";
        int clusters = 0;
        for (auto& [k, v] : freq) {
            if (v > 1) {
                clusters++;
                if (clusters <= 5) cout << "nonce=0x" << hex << k << dec << " ×" << v << "  ";
            }
        }
        if (clusters > 5) cout << "... (" << clusters << " total clusters)";
        cout << endl;
    }
    
    cout << "\n====================================================" << endl;
    cout << "  NONCE UNIQUENESS: " << unique_count << "/3 PATTERNS PASSED" << endl;
    cout << "====================================================" << endl;
    return (unique_count == 3) ? 0 : 1;
}
