#include <iostream>
#include <vector>
#include <cmath>

using namespace std;

int main() {
    cout << "========================================\n";
    cout << "  HANAPIN ANG PLAINTEXT MODULUS\n";
    cout << "  kung saan ang 5 ay quadratic residue\n";
    cout << "========================================\n\n";

    // Kailangan natin ng prime p kung saan:
    // 1. May √5 (5 ay quadratic residue)
    // 2. Compatible sa OpenFHE (p - 1 divisible by 2N)
    // 3. Malaki enough para sa computation

    cout << "1. QUADRATIC RESIDUE TEST:\n";
    cout << "   5 ay QR mod p kung 5^((p-1)/2) ≡ 1 mod p\n";
    cout << "   (Euler's criterion)\n";
    cout << "-----------------------------------\n";

    vector<int64_t> candidates = {
        65537,  // Fermat prime (2^16+1) - 5 ay non-residue
        786433, // 3 * 2^18 + 1
        12289,  // 3 * 2^12 + 1
        40961,  // 5 * 2^13 + 1
        61441,  // 15 * 2^12 + 1
        8191,   // Mersenne prime (2^13 - 1)
        257,    // Fermat prime (2^8+1)
        65537,  // duplicate lang
    };

    for (auto p : candidates) {
        bool is_prime = true;
        for (int64_t i = 2; i * i <= p; i++) {
            if (p % i == 0) {
                is_prime = false;
                break;
            }
        }
        
        if (!is_prime) {
            cout << "   p=" << p << ": Hindi prime\n";
            continue;
        }
        
        // Euler's criterion
        int64_t val = 1;
        for (int64_t i = 0; i < (p - 1) / 2; i++) {
            val = (val * 5) % p;
        }
        
        bool is_qr = (val == 1);
        cout << "   p=" << p << ": 5 ay " 
             << (is_qr ? "QUADRATIC RESIDUE" : "non-residue") << "\n";
        
        if (is_qr) {
            // Hanapin ang √5
            for (int64_t i = 1; i < p; i++) {
                if ((i * i) % p == 5) {
                    cout << "     √5 = " << i << "\n";
                    // φ = (1 + √5) / 2
                    int64_t inv2 = (p + 1) / 2;
                    int64_t phi = ((1 + i) * inv2) % p;
                    cout << "     φ = " << phi << "\n";
                    cout << "     φ² mod p = " << (phi * phi) % p << "\n";
                    cout << "     φ + 1 mod p = " << (phi + 1) % p << "\n";
                    cout << "     Match: " << ((phi * phi) % p == (phi + 1) % p ? "YES" : "NO") << "\n";
                    break;
                }
            }
        }
    }

    cout << "\n2. OPENFHE COMPATIBILITY:\n";
    cout << "   Kailangan: p - 1 ay divisible ng 2N\n";
    cout << "   kung saan N ay ring dimension / 2\n";
    cout << "   Para sa RingDim=32768, N=16384\n";
    cout << "   Kailangan: p - 1 divisible ng 32768\n";
    cout << "-----------------------------------\n";

    for (auto p : candidates) {
        if ((p - 1) % 32768 == 0) {
            cout << "   p=" << p << ": COMPATIBLE (p-1 = " 
                 << (p - 1) / 32768 << " × 32768)\n";
        } else {
            cout << "   p=" << p << ": hindi compatible\n";
        }
    }

    cout << "\n========================================\n";
    cout << "  KONKLUSYON:\n";
    cout << "  Kailangan ng p na may:\n";
    cout << "  - 5 ay quadratic residue\n";
    cout << "  - p - 1 divisible ng 32768\n";
    cout << "  - Malaki enough para sa security\n";
    cout << "========================================\n";

    return 0;
}
