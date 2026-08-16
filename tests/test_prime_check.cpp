#include <NTL/ZZ.h>
#include <iostream>

int main() {
    NTL::ZZ Q = NTL::to_ZZ("340282366920938463463374607431768211297");
    
    std::cout << "Q = " << Q << "\n";
    std::cout << "Q mod 5 = " << (Q % 5) << "\n";
    std::cout << "Is prime: " << (NTL::ProbPrime(Q, 50) ? "YES" : "NO") << "\n";
    
    // Try isang known na valid prime
    NTL::ZZ Q_valid = NTL::to_ZZ("4294967291"); // 32-bit prime ≡ 1 mod 5
    std::cout << "\nQ_valid = " << Q_valid << "\n";
    std::cout << "Q_valid mod 5 = " << (Q_valid % 5) << "\n";
    std::cout << "Is prime: " << (NTL::ProbPrime(Q_valid, 50) ? "YES" : "NO") << "\n";
    
    // Check sqrt5 sa Q_valid
    NTL::ZZ sqrt5;
    NTL::SqrRootMod(sqrt5, NTL::to_ZZ(5), Q_valid);
    std::cout << "sqrt5² mod Q_valid = " << (sqrt5*sqrt5) % Q_valid << " (should be 5)\n";
    
    return 0;
}
