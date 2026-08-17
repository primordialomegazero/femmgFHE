// GENERATE PRIME na q ≡ 1 mod 65536 at may √5
#include <NTL/ZZ.h>
#include <iostream>

int main() {
    std::cout << "GENERATE COMPATIBLE PRIME\n";
    std::cout << "========================\n\n";
    
    // Kailangan: q ≡ 1 mod 65536 (para sa ring 65536)
    // At: q ≡ 1 mod 5 (para may √5)
    // Kaya: q ≡ 1 mod (65536 × 5) = 1 mod 327680
    
    NTL::ZZ base = NTL::to_ZZ(327680);
    
    std::cout << "Maghanap ng prime q = 327680·k + 1...\n\n";
    
    int count = 0;
    for (long k = 1; k < 100000 && count < 10; k++) {
        NTL::ZZ q = base * NTL::to_ZZ(k) + 1;
        if (NTL::ProbPrime(q, 20)) {
            std::cout << "  q = 327680 × " << k << " + 1 = " << q << "\n";
            std::cout << "    q ≡ 1 mod 5: " << (q % 5 == 1) << "\n";
            std::cout << "    65536 | (q-1): " << ((q-1) % 65536 == 0) << "\n";
            
            // Check kung may √5
            NTL::ZZ sqrt5;
            NTL::SqrRootMod(sqrt5, NTL::to_ZZ(5), q);
            std::cout << "    √5² mod q = " << (sqrt5*sqrt5)%q << " (dapat 5)\n\n";
            count++;
        }
    }
    
    return 0;
}
