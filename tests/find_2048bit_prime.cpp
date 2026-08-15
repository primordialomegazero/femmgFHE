#include <NTL/ZZ.h>
#include <iostream>

int main() {
    std::cout << "FINDING 2048-BIT PRIME Q ≡ 1 mod 5\n\n";
    
    // Start from 2^2048
    NTL::ZZ candidate = NTL::power_ZZ(2, 2048) - 105;
    
    int attempts = 0;
    while (true) {
        attempts++;
        
        if (candidate % 5 == 1 && NTL::ProbPrime(candidate, 10)) {
            std::cout << "FOUND after " << attempts << " attempts!\n";
            std::cout << "Q = " << candidate << "\n";
            std::cout << "Q bits: " << NTL::NumBits(candidate) << "\n";
            std::cout << "Q mod 5: " << candidate % 5 << "\n";
            std::cout << "Is prime: YES\n";
            
            NTL::ZZ sqrt5;
            NTL::SqrRootMod(sqrt5, NTL::to_ZZ(5), candidate);
            std::cout << "sqrt(5)² mod Q = " << (sqrt5 * sqrt5) % candidate << "\n";
            break;
        }
        
        candidate -= 10;
        
        if (attempts > 10000) {
            std::cout << "Took too long...\n";
            break;
        }
    }
    
    return 0;
}
