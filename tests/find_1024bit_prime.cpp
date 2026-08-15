#include <NTL/ZZ.h>
#include <iostream>
#include <random>

int main() {
    std::cout << "FINDING 1024-BIT PRIME Q ≡ 1 mod 5\n\n";
    
    std::random_device rd;
    std::mt19937_64 gen(rd());
    
    // Start sa known 1024-bit prime (2^1024 - 2^960 - 1 + 2^512 * something)
    // Para mas madali, gamitin natin ang 2^1024 - 105 bilang starting point
    NTL::ZZ candidate = NTL::power_ZZ(2, 1024) - 105;
    
    int attempts = 0;
    while (true) {
        attempts++;
        
        // Check kung prime at ≡ 1 mod 5
        if (candidate % 5 == 1 && NTL::ProbPrime(candidate, 10)) {
            std::cout << "FOUND after " << attempts << " attempts!\n";
            std::cout << "Q = " << candidate << "\n";
            std::cout << "Q bits: " << NTL::NumBits(candidate) << "\n";
            std::cout << "Q mod 5: " << candidate % 5 << "\n";
            std::cout << "Is prime: YES\n";
            
            // Verify sqrt(5) exists
            NTL::ZZ sqrt5;
            NTL::SqrRootMod(sqrt5, NTL::to_ZZ(5), candidate);
            std::cout << "sqrt(5)² mod Q = " << (sqrt5 * sqrt5) % candidate << "\n";
            break;
        }
        
        candidate -= 10; // Para maintain mod 5 = 1 (since 105 mod 5 = 0, need -105 ≡ 0 mod 5)
        
        if (attempts > 10000) {
            std::cout << "Took too long, trying different approach...\n";
            break;
        }
    }
    
    return 0;
}
