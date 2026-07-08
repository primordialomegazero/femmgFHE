#include <iostream>

using namespace std;

int main() {
    // Quick debug: what are the actual values in multiplication?
    uint64_t q = 1618033988ULL * 100;
    uint64_t t = 256;
    uint64_t delta = q / t;
    
    cout << "q = " << q << "\n";
    cout << "t = " << t << "\n";
    cout << "delta = " << delta << "\n";
    cout << "delta^2 = " << (delta * delta) << "\n\n";
    
    // For 2×3=6:
    uint64_t m = 6;
    uint64_t expected = m * delta * delta;
    cout << "Expected (m × delta²) = " << expected << "\n";
    cout << "Expected / q = " << (expected / q) << " (should be < 1)\n";
    cout << "Expected % q = " << (expected % q) << "\n\n";
    
    // Problem: delta² overflows uint64_t!
    cout << "MAX uint64_t = " << UINT64_MAX << "\n";
    cout << "delta² = " << (delta * delta) << " (OVERFLOW!)\n";
    
    return 0;
}
