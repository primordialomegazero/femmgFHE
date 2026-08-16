#include "../src/fhe/golden_fibonacci_fhe_v6.h"
#include <iostream>

int main() {
    NTL::ZZ Q = NTL::to_ZZ("4294967291");
    golden_fhe_v6::FibonacciFHEV6 fhe(Q, 42);
    
    std::cout << "s_val = " << fhe.s_val << "\n";
    std::cout << "alpha = " << fhe.alpha << "\n";
    std::cout << "beta = " << fhe.beta << "\n";
    
    // I-verify: s² = α·s + β
    NTL::ZZ s_sq = (fhe.s_val * fhe.s_val) % Q;
    NTL::ZZ asb = (fhe.alpha * fhe.s_val + fhe.beta) % Q;
    
    std::cout << "s² mod Q = " << s_sq << "\n";
    std::cout << "α·s+β mod Q = " << asb << "\n";
    std::cout << "Match: " << (s_sq == asb ? "YES" : "NO") << "\n";
    
    return 0;
}
