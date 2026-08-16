#include "../src/fhe/golden_fibonacci_fhe_v6.h"
#include <iostream>

int main() {
    std::cout << "ENC/DEC TEST - V6\n\n";
    
    NTL::ZZ Q = NTL::to_ZZ("340282366920938463463374607431768211297");
    golden_fhe_v6::FibonacciFHEV6 fhe(Q, 42);
    
    auto ct0 = fhe.encrypt(false);
    auto ct1 = fhe.encrypt(true);
    
    std::cout << "Decrypt(Encrypt(0)): " << fhe.decrypt(ct0) << " (exp 0)\n";
    std::cout << "Decrypt(Encrypt(1)): " << fhe.decrypt(ct1) << " (exp 1)\n";
    
    // I-check kung magkaiba ang ct0 at ct1
    std::cout << "\nct0.first != ct1.first: " << (ct0.first != ct1.first) << "\n";
    std::cout << "ct0.second != ct1.second: " << (ct0.second != ct1.second) << "\n";
    
    // NAND(1,1)
    auto nand11 = fhe.nand_gate(ct1, ct1);
    std::cout << "\nNAND(1,1): " << fhe.decrypt(nand11) << " (exp 0)\n";
    
    // NOT(0)
    auto not0 = fhe.not_gate(ct0);
    std::cout << "NOT(0): " << fhe.decrypt(not0) << " (exp 1)\n";
    
    return 0;
}
