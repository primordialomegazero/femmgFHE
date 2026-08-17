// GENERATE COMPATIBLE PRIME
// p ≡ 1 mod 5, p ≡ 1 mod 65536

#include <NTL/ZZ.h>
#include <iostream>

int main() {
    std::cout << "GENERATE COMPATIBLE PRIME\n";
    std::cout << "=========================\n\n";

    // Kailangan: p ≡ 1 mod (5 × 65536) = 1 mod 327680
    NTL::ZZ base = NTL::to_ZZ(327680);
    
    std::cout << "Kailangan: p ≡ 1 mod " << base << "\n\n";
    std::cout << "Maghanap ng prime na p ≡ 1 mod 327680...\n\n";

    int count = 0;
    for (long k = 1; k < 100000 && count < 5; k++) {
        NTL::ZZ p = base * NTL::to_ZZ(k) + 1;
        if (NTL::ProbPrime(p, 20)) {
            std::cout << "  ✓ p = 327680 × " << k << " + 1 = " << p << "\n";
            std::cout << "    p ≡ 1 mod 5: " << (p % 5 == 1) << "\n";
            std::cout << "    65536 | (p-1): " << ((p-1) % 65536 == 0) << "\n\n";
            count++;
        }
    }

    return 0;
}
