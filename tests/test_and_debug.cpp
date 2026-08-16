#include "../src/fhe/golden_fibonacci_fhe_v6.h"
#include <iostream>

int main() {
    std::cout << "AND DEBUG\n\n";
    
    NTL::ZZ Q = NTL::to_ZZ("340282366920938463463374607431768211297");
    golden_fhe_v6::FibonacciFHEV6 fhe(Q, 42);
    
    auto ct1 = fhe.encrypt(true);
    
    // NAND(1,1) = 0
    auto nand11 = fhe.nand_gate(ct1, ct1);
    std::cout << "NAND(1,1): " << fhe.decrypt(nand11) << " (exp 0)\n";
    
    // NOT(NAND(1,1)) = NOT(0) = 1
    auto not_nand11 = fhe.nand_gate(nand11, nand11);
    std::cout << "NOT(NAND(1,1)): " << fhe.decrypt(not_nand11) << " (exp 1)\n";
    
    // AND(1,1) = NOT(NAND(1,1))
    auto and11 = fhe.and_gate(ct1, ct1);
    std::cout << "AND(1,1): " << fhe.decrypt(and11) << " (exp 1)\n";
    
    return 0;
}
