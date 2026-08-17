// HANAPIN ANG COMPATIBLE PLAINTEXT MODULUS
// ≡ 1 mod 5, prime, at 65536 | (p-1)

#include <NTL/ZZ.h>
#include <iostream>

int main() {
    std::cout << "FIND COMPATIBLE PLAINTEXT MODULUS\n";
    std::cout << "=================================\n\n";

    std::cout << "Kailangan:\n";
    std::cout << "  1. Prime\n";
    std::cout << "  2. p ≡ 1 mod 5 (para sa √5)\n";
    std::cout << "  3. 65536 | (p-1) (para sa ring 32768)\n\n";

    NTL::ZZ candidates[] = {
        NTL::to_ZZ("786433"),           // 3×2^18+1 — kilala sa FHE
        NTL::to_ZZ("1152921504606846977"), // 2^60 - 2^14 + 1
        NTL::to_ZZ("1099511627777"),    // 2^40 + 2^15 + 1
    };

    for (auto& p : candidates) {
        bool is_prime = NTL::ProbPrime(p, 20);
        bool mod_1_5 = (p % 5 == 1);
        bool compatible = ((p - 1) % 65536 == 0);

        std::cout << "  " << p << "\n";
        std::cout << "    Prime: " << (is_prime ? "YES" : "NO") << "\n";
        std::cout << "    ≡ 1 mod 5: " << (mod_1_5 ? "YES" : "NO") << "\n";
        std::cout << "    65536 | (p-1): " << (compatible ? "YES" : "NO") << "\n";

        if (is_prime && mod_1_5 && compatible) {
            std::cout << "    ✓ VALID!\n";
            NTL::ZZ sqrt5;
            NTL::SqrRootMod(sqrt5, NTL::to_ZZ(5), p);
            std::cout << "    √5² mod p = " << (sqrt5*sqrt5)%p << " (dapat 5)\n";
        }
        std::cout << "\n";
    }

    return 0;
}
