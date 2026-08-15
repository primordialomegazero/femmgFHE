#include <NTL/ZZ_pX.h>
#include <NTL/ZZ_p.h>
#include <iostream>
#include <vector>

int main() {
    std::cout << "FIND 128-BIT Q WITH φ PROPERTY\n\n";
    
    // Kailangan: Q ≡ 1 o 4 mod 5 (quadratic reciprocity)
    // Kailangan: Q ay prime
    
    // Maghanap ng 128-bit primes na Q ≡ 1 mod 5
    // Starting point: 2^127
    NTL::ZZ Q = NTL::to_ZZ("170141183460469231731687303715884105727"); // 2^127 - 1 (Mersenne)
    
    // I-adjust para Q ≡ 1 mod 5
    NTL::ZZ remainder = NTL::to_ZZ(5);
    NTL::ZZ adjustment = NTL::to_ZZ(1);
    adjustment = (adjustment - (Q % 5) + 5) % 5;
    Q += adjustment;
    
    std::cout << "Starting search sa " << NTL::NumBits(Q) << " bits\n\n";
    
    int found = 0;
    while (found < 10) {
        if (NTL::ProbPrime(Q, 30)) {
            std::cout << "  FOUND: " << Q << "\n";
            std::cout << "    Bits: " << NTL::NumBits(Q) << "\n";
            std::cout << "    Mod 5: " << Q % 5 << "\n";
            
            // Compute φ
            NTL::ZZ_p::init(Q);
            NTL::ZZ five = NTL::to_ZZ("5");
            NTL::ZZ sqrt5;
            NTL::SqrRootMod(sqrt5, five % Q, Q);
            
            if (sqrt5 != 0) {
                NTL::ZZ inv2 = NTL::InvMod(NTL::to_ZZ("2"), Q);
                NTL::ZZ phi = ((NTL::to_ZZ("1") + sqrt5) * inv2) % Q;
                
                NTL::ZZ phi_sq = (phi * phi) % Q;
                NTL::ZZ phi_plus_1 = (phi + 1) % Q;
                
                std::cout << "    φ = " << phi << "\n";
                std::cout << "    φ² = φ+1: " << (phi_sq == phi_plus_1 ? "YES ✓" : "NO ✗") << "\n";
                
                if (phi_sq == phi_plus_1) found++;
            }
            std::cout << "\n";
        }
        Q += 5;
    }
    
    return 0;
}
