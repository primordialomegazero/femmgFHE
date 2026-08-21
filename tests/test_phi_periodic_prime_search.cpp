// φ-PERIODIC PRIME SEARCH
// Hanapin ang prime p kung saan ang φ ay may maliit na period
// Ito ang magbibigay ng natural na modulus recycling
//
// ANG KEY:
// Kung φ^n ≡ φ (mod p) para sa maliit na n,
// ang modulus ay natural na nagre-recycle every n operations

#include <NTL/ZZ.h>
#include <iostream>
#include <vector>

int main() {
    std::cout << "========================================\n";
    std::cout << "  φ-PERIODIC PRIME SEARCH\n";
    std::cout << "  Natural Modulus Recycling\n";
    std::cout << "========================================\n\n";

    // Test sa maliit na primes muna
    std::vector<long long> primes = {
        5, 11, 19, 29, 31, 41, 59, 61, 71, 79, 89, 101
    };

    std::cout << "Prime | φ mod p | Period | φ^period mod p\n";
    std::cout << "------|----------|--------|----------------\n";

    for (long long p : primes) {
        // Compute φ mod p
        long long sqrt5 = -1;
        for (long long x = 0; x < p; x++) {
            if ((x * x) % p == 5 % p) {
                sqrt5 = x;
                break;
            }
        }

        if (sqrt5 == -1) {
            std::cout << "  " << p << "    | NO √5 | - | -\n";
            continue;
        }

        long long inv2 = (p + 1) / 2;  // inverse ng 2 mod p
        long long phi_mod = ((1 + sqrt5) * inv2) % p;

        // Hanapin ang period: φ^n ≡ 1 (mod p)
        long long period = 1;
        long long current = phi_mod;
        while (current != 1 && period < 100) {
            current = (current * phi_mod) % p;
            period++;
        }

        std::cout << "  " << p << "    | " << phi_mod 
                  << " | " << period << " | " << current << "\n";
    }

    std::cout << "\n========================================\n";
    std::cout << "  KEY: Ang primes na may MALIIT na period\n";
    std::cout << "  ay nagbibigay ng natural na level recycling!\n";
    std::cout << "========================================\n";

    return 0;
}
