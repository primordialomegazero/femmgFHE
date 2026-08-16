#include "../src/fhe/golden_fibonacci_fhe_v6.h"
#include <iostream>

int main() {
    std::cout << "VALUES CHECK\n\n";
    
    NTL::ZZ Q = NTL::to_ZZ("340282366920938463463374607431768211297");
    golden_fhe_v6::FibonacciFHEV6 fhe(Q, 42);
    
    std::cout << "Q = " << Q << "\n";
    std::cout << "golden_plain = " << fhe.golden_plain << "\n";
    std::cout << "inv_golden = " << fhe.inv_golden << "\n";
    std::cout << "golden_plain * inv_golden mod Q = " 
              << (fhe.golden_plain * fhe.inv_golden) % Q << " (should be 1)\n";
    std::cout << "s_val = " << fhe.s_val << "\n";
    std::cout << "alpha = " << fhe.alpha << "\n";
    std::cout << "beta = " << fhe.beta << "\n";
    
    return 0;
}
