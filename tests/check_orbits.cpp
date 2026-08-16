#include "../src/fhe/golden_fibonacci_fhe_v6.h"
#include <iostream>

int main() {
    NTL::ZZ Q = NTL::to_ZZ("4294967291");
    golden_fhe_v6::FibonacciFHEV6 fhe(Q, 42);
    
    std::cout << "Q = " << Q << "\n";
    std::cout << "Q/2 = " << Q/2 << "\n";
    std::cout << "golden_plain(φ) = " << fhe.golden_plain << "\n";
    std::cout << "psi = " << fhe.psi_zz << "\n\n";
    
    std::cout << "φ - Q/2 = " << (fhe.golden_plain - Q/2) << "\n";
    std::cout << "ψ - Q/2 = " << (fhe.psi_zz - Q/2) << "\n\n";
    
    std::cout << "dist between φ and ψ = " << (fhe.golden_plain - fhe.psi_zz) << "\n";
    std::cout << "dist between φ and Q-φ = " << (Q - 2*fhe.golden_plain) << "\n";
    
    return 0;
}
