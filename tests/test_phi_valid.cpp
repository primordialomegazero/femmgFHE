#include "../src/fhe/golden_fibonacci_fhe_v5.h"
#include <iostream>

int main() {
    NTL::ZZ Q = NTL::to_ZZ("340282366920938463463374607431768211297");
    golden_fhe_v5::FibonacciFHEV5 fhe(Q, 42);
    
    NTL::ZZ gp = fhe.golden_plain;
    NTL::ZZ lhs = (gp * gp - gp - 1) % Q;
    if (lhs < 0) lhs += Q;
    
    std::cout << "golden_plain = " << gp << "\n";
    std::cout << "gp² - gp - 1 mod Q = " << lhs << " (should be 0)\n";
    
    NTL::ZZ phi = fhe.phi_zz;
    NTL::ZZ psi = fhe.psi_zz;
    std::cout << "phi = " << phi << "\n";
    std::cout << "psi = " << psi << "\n";
    std::cout << "phi + psi mod Q = " << ((phi + psi) % Q) << " (should be 1)\n";
    std::cout << "phi * psi mod Q = " << ((phi * psi) % Q) << " (should be Q-1 = -1)\n";
    
    return 0;
}
