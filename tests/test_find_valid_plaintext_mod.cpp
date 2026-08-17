// HANAPIN ANG TAMANG PLAINTEXT MODULUS
// Prime, ≡ 1 mod 5, > 599074578

#include <NTL/ZZ.h>
#include <iostream>

int main() {
    std::cout << "FIND VALID PLAINTEXT MODULUS\n";
    std::cout << "============================\n\n";
    
    // Kailangan: > 599074578, prime, ≡ 1 mod 5
    NTL::ZZ candidates[] = {
        NTL::to_ZZ("1000000007"),  // ~10^9, prime?
        NTL::to_ZZ("1000000009"),
        NTL::to_ZZ("1073741827"),  // 2^30 + 3
        NTL::to_ZZ("2147483647"),  // 2^31 - 1
        NTL::to_ZZ("4294967291"),  // 2^32 - 5
    };
    
    for (auto& p : candidates) {
        bool is_prime = NTL::ProbPrime(p, 20);
        bool mod_1_5 = (p % 5 == 1);
        
        std::cout << "  " << p << "\n";
        std::cout << "    Prime: " << (is_prime ? "YES" : "NO") << "\n";
        std::cout << "    ≡ 1 mod 5: " << (mod_1_5 ? "YES" : "NO") << "\n";
        
        if (is_prime && mod_1_5) {
            std::cout << "    ✓ VALID!\n";
            
            // I-verify na may √5
            NTL::ZZ sqrt5;
            NTL::SqrRootMod(sqrt5, NTL::to_ZZ(5), p);
            std::cout << "    √5 mod p = " << sqrt5 << "\n";
            std::cout << "    √5² mod p = " << (sqrt5*sqrt5)%p << " (dapat 5)\n";
        }
        std::cout << "\n";
    }
    
    return 0;
}
