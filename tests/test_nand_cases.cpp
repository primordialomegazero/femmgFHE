#include "../src/fhe/golden_fibonacci_fhe_v6.h"
#include <iostream>

int main() {
    std::cout << "NAND CASES TEST\n\n";
    
    NTL::ZZ Q = NTL::to_ZZ("340282366920938463463374607431768211297");
    golden_fhe_v6::FibonacciFHEV6 fhe(Q, 42);
    
    auto ct0 = fhe.encrypt(false);
    auto ct1 = fhe.encrypt(true);
    
    // Basic NAND truth table
    std::cout << "NAND(0,0): " << fhe.decrypt(fhe.nand_gate(ct0, ct0)) << " (exp 1)\n";
    std::cout << "NAND(0,1): " << fhe.decrypt(fhe.nand_gate(ct0, ct1)) << " (exp 1)\n";
    std::cout << "NAND(1,0): " << fhe.decrypt(fhe.nand_gate(ct1, ct0)) << " (exp 1)\n";
    std::cout << "NAND(1,1): " << fhe.decrypt(fhe.nand_gate(ct1, ct1)) << " (exp 0)\n";
    
    // NOT = NAND(a,a)
    std::cout << "\nNOT(0): " << fhe.decrypt(fhe.not_gate(ct0)) << " (exp 1)\n";
    std::cout << "NOT(1): " << fhe.decrypt(fhe.not_gate(ct1)) << " (exp 0)\n";
    
    // AND = NOT(NAND(a,b))
    auto nand00 = fhe.nand_gate(ct0, ct0);
    auto not_nand00 = fhe.not_gate(nand00);
    std::cout << "\nAND(0,0) = NOT(NAND(0,0)): " << fhe.decrypt(not_nand00) << " (exp 0)\n";
    
    auto nand11 = fhe.nand_gate(ct1, ct1);
    auto not_nand11 = fhe.not_gate(nand11);
    std::cout << "AND(1,1) = NOT(NAND(1,1)): " << fhe.decrypt(not_nand11) << " (exp 1)\n";
    
    return 0;
}
