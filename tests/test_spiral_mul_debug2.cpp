#include <iostream>
using namespace std;

int main() {
    uint64_t q = 1618033988;
    uint64_t t = 2;
    uint64_t delta = q / t;  // ~809M
    
    cout << "q=" << q << " t=" << t << " delta=" << delta << "\n";
    cout << "delta² = " << (delta * delta) << "\n\n";
    
    // Simulate: m=2, m=3, product=6
    // Encryption adds noise ~delta/4 = 202M
    uint64_t noise = delta / 4;
    
    uint64_t m1 = 2, m2 = 3, expected = 6;
    
    // c0 for m1: m1*delta + noise ≈ 2*809M + 202M = 1.82B
    // But q = 1.618B, so this WRAPS!
    uint64_t c0 = (m1 * delta + noise) % q;
    cout << "c0 for m1=" << m1 << ": " << c0 << " (delta*m1=" << (m1*delta) << ")\n";
    cout << "  delta*m1 + noise = " << (m1 * delta + noise) << " > q=" << q << "? " << ((m1 * delta + noise) > q ? "YES - WRAPPED!" : "no") << "\n";
    
    return 0;
}
