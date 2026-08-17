// CHECK: MAY √5 BA TALAGA MOD 65537?
#include <iostream>
#include <gmpxx.h>

using namespace std;

int main() {
    cout << "√5 MOD 65537 CHECK\n";
    cout << "==================\n\n";
    
    const long long p = 65537;
    
    // Method 1: Check Legendre symbol
    long long legendre = 1;
    for (int i = 0; i < 32768; i++) {
        legendre = (legendre * 5) % p;
    }
    cout << "5^((p-1)/2) mod p = " << legendre << "\n";
    cout << "Kung 1 → May √5\n";
    cout << "Kung 65536 → WALANG √5\n\n";
    
    // Method 2: Direct search
    bool found = false;
    for (long long i = 0; i < p; i++) {
        if ((i * i) % p == 5) {
            cout << "FOUND: √5 = " << i << "\n";
            cout << "Verify: " << i << "² = " << (i*i)%p << " mod " << p << "\n";
            found = true;
            break;
        }
    }
    
    if (!found) {
        cout << "WALANG √5 mod " << p << "!\n";
        cout << "5 ay HINDI quadratic residue mod 65537\n\n";
        
        // Check kung anong primes ang may √5
        cout << "Primes < 100 na may √5:\n";
        for (int q = 2; q < 100; q++) {
            bool is_prime = true;
            for (int d = 2; d * d <= q; d++) {
                if (q % d == 0) { is_prime = false; break; }
            }
            if (is_prime) {
                bool has_sqrt5 = false;
                for (int i = 0; i < q; i++) {
                    if ((i * i) % q == 5) { has_sqrt5 = true; break; }
                }
                if (has_sqrt5) cout << "  " << q << "\n";
            }
        }
    }
    
    return 0;
}
