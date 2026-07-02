#include "../src/core/lyapunov_fhe.h"
#include <iostream>
#include <set>
using namespace std;

int main() {
    LyapunovFHE fhe;
    int pass = 0, total = 3;
    
    // Test 1: seed+i pattern
    {
        set<uint64_t> s;
        for (int i = 0; i < 10000; i++) {
            s.insert(fhe.encrypt(0.0, 0x5ECDA1A5E + i).nonce);
        }
        bool ok = (s.size() == 10000);
        cout << "1. seed+i pattern: " << s.size() << "/10000 " << (ok ? "✅" : "❌") << endl;
        if (ok) pass++;
    }
    
    // Test 2: seed^hash pattern
    {
        set<uint64_t> s;
        for (int i = 0; i < 10000; i++) {
            s.insert(fhe.encrypt(0.0, 0x5ECDA1A5E ^ (i * 0x9E3779B97F4A7C15ULL)).nonce);
        }
        bool ok = (s.size() == 10000);
        cout << "2. seed^hash pattern: " << s.size() << "/10000 " << (ok ? "✅" : "❌") << endl;
        if (ok) pass++;
    }
    
    // Test 3: seed=0 (random)
    {
        set<uint64_t> s;
        for (int i = 0; i < 10000; i++) {
            s.insert(fhe.encrypt(0.0, 0).nonce);
        }
        bool ok = (s.size() == 10000);
        cout << "3. seed=0 (random): " << s.size() << "/10000 " << (ok ? "✅" : "❌") << endl;
        if (ok) pass++;
    }
    
    cout << "\nNONCE UNIQUENESS: " << pass << "/" << total << " PASSED" << endl;
    return (pass == total) ? 0 : 1;
}
