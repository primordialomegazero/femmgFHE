#include "../src/fhe/golden_fibonacci_fhe_v6.h"
#include <iostream>

int main() {
    NTL::ZZ Q = NTL::to_ZZ("115792089237316195423570985008687907853269984665640564039457584007913129640731");
    golden_fhe_v6::FibonacciFHEV6 fhe(Q, 42);
    
    std::cout << "φ (sqrt-based) = " << fhe.phi_zz << "\n";
    std::cout << "ψ = " << fhe.psi_zz << "\n";
    std::cout << "golden_plain = " << fhe.golden_plain << "\n";
    std::cout << "golden_plain == φ: " << (fhe.golden_plain == fhe.phi_zz ? "YES" : "NO") << "\n";
    std::cout << "golden_plain == ψ: " << (fhe.golden_plain == fhe.psi_zz ? "YES" : "NO") << "\n";
    
    return 0;
}
