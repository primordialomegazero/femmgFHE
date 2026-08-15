#include <NTL/ZZ.h>
#include <iostream>

int main() {
    // Try NIST P-256 prime scaled: 2^1024 - 2^960 + 2^896 - 1 is not ≡ 1 mod 5
    // Instead, use: 2^1024 - 109 (this is ≡ 1 mod 5 if 2^1024 ≡ 4 mod 5)
    // Check: 2^1024 mod 5 = 4 (since 2^4 = 16 ≡ 1 mod 5, 1024/4 = 256 remainder 0)
    // So 2^1024 ≡ 1 mod 5, need subtract something ≡ 0 mod 5
    
    NTL::ZZ Q = NTL::power_ZZ(2, 1024) - 109;
    
    std::cout << "Candidate Q = 2^1024 - 109\n";
    std::cout << "Q bits: " << NTL::NumBits(Q) << "\n";
    std::cout << "Q mod 5: " << Q % 5 << "\n";
    std::cout << "Is prime: " << (NTL::ProbPrime(Q, 20) ? "YES" : "NO") << "\n";
    
    if (Q % 5 == 1 && NTL::ProbPrime(Q, 20)) {
        NTL::ZZ sqrt5;
        NTL::SqrRootMod(sqrt5, NTL::to_ZZ(5), Q);
        std::cout << "sqrt5² mod Q = " << (sqrt5 * sqrt5) % Q << " (should be 5)\n";
        std::cout << "\nQ = " << Q << "\n";
    }
    
    return 0;
}
