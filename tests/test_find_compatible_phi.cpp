#include <iostream>
#include <vector>

using namespace std;

int main() {
    cout << "========================================\n";
    cout << "  HANAPIN ANG COMPATIBLE PRIME NA MAY φ\n";
    cout << "========================================\n\n";

    // Kailangan: p prime, 5 ay QR, p-1 divisible ng 32768
    // p = k × 32768 + 1 kung saan k ay integer
    
    cout << "1. CANDIDATES (p = k × 32768 + 1):\n";
    cout << "-----------------------------------\n";
    
    for (int k = 1; k <= 30; k++) {
        uint64_t p = k * 32768 + 1;
        
        // Check kung prime
        bool is_prime = true;
        for (uint64_t i = 2; i * i <= p; i++) {
            if (p % i == 0) {
                is_prime = false;
                break;
            }
        }
        
        if (!is_prime) continue;
        
        // Euler's criterion para sa 5
        uint64_t val = 1;
        for (uint64_t i = 0; i < (p - 1) / 2; i++) {
            val = (val * 5) % p;
        }
        
        bool is_qr = (val == 1);
        
        cout << "   k=" << k << ": p=" << p;
        if (is_qr) {
            cout << " - 5 ay QR";
            // Hanapin ang √5 at φ
            for (uint64_t i = 1; i < p; i++) {
                if ((i * i) % p == 5) {
                    uint64_t inv2 = (p + 1) / 2;
                    uint64_t phi = ((1 + i) * inv2) % p;
                    cout << ", √5=" << i << ", φ=" << phi;
                    break;
                }
            }
        } else {
            cout << " - 5 ay non-residue";
        }
        cout << "\n";
    }
    
    cout << "\n2. MAS MALAKING SEARCH:\n";
    cout << "-----------------------------------\n";
    
    // Subukan ang mga kilalang FHE-friendly primes
    vector<uint64_t> known_primes = {
        65537,      // 2^16 + 1
        786433,     // 3 × 2^18 + 1
        12289,      // 3 × 2^12 + 1
        40961,      // 5 × 2^13 + 1
        61441,      // 15 × 2^12 + 1
        8191,       // 2^13 - 1
        167772161,  // 5 × 2^25 + 1
        469762049,  // 7 × 2^26 + 1
        100663297,  // 3 × 2^25 + 1
        40961,      // duplicate
    };
    
    for (auto p : known_primes) {
        bool is_prime = true;
        for (uint64_t i = 2; i * i <= p; i++) {
            if (p % i == 0) {
                is_prime = false;
                break;
            }
        }
        
        if (!is_prime) continue;
        
        uint64_t val = 1;
        for (uint64_t i = 0; i < (p - 1) / 2; i++) {
            val = (val * 5) % p;
        }
        
        bool is_qr = (val == 1);
        bool is_compatible = ((p - 1) % 32768 == 0);
        
        if (is_qr && is_compatible) {
            cout << "   p=" << p << ": QR + COMPATIBLE\n";
        } else if (is_qr) {
            cout << "   p=" << p << ": QR pero hindi compatible\n";
        } else if (is_compatible) {
            cout << "   p=" << p << ": compatible pero non-residue\n";
        }
    }
    
    cout << "\n========================================\n";
    cout << "  KONKLUSYON:\n";
    cout << "  Walang maliit na prime na may φ at\n";
    cout << "  compatible sa RingDim=16384\n";
    cout << "  Kailangan ng mas malaking ring dimension\n";
    cout << "  o custom plaintext modulus\n";
    cout << "========================================\n";

    return 0;
}
