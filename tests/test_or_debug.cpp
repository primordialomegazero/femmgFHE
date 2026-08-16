// OR Gate Debug
#include "../src/fhe/golden_fibonacci_fhe_v6.h"
#include <iostream>

int main() {
    std::cout << "OR GATE DEBUG\n";
    std::cout << "=============\n\n";
    
    NTL::ZZ Q = NTL::to_ZZ("115792089237316195423570985008687907853269984665640564039457584007913129640731");
    golden_fhe_v6::FibonacciFHEV6 fhe(Q, 42);
    
    auto ct0 = fhe.encrypt(false);
    auto ct1 = fhe.encrypt(true);
    
    // Test NOT gate
    auto not0 = fhe.not_gate(ct0);
    auto not1 = fhe.not_gate(ct1);
    std::cout << "NOT(0) = " << fhe.decrypt(not0) << " (exp 1)\n";
    std::cout << "NOT(1) = " << fhe.decrypt(not1) << " (exp 0)\n\n";
    
    // Test OR via NAND(NOT(a), NOT(b))
    auto or00 = fhe.raw_nand(not0, not0);  // OR(0,0) = NAND(1,1) = 0
    std::cout << "OR(0,0) via raw_nand = " << fhe.decrypt(or00) << " (exp 0)\n";
    
    auto or01 = fhe.raw_nand(not0, not1);  // OR(0,1) = NAND(1,0) = 1
    std::cout << "OR(0,1) via raw_nand = " << fhe.decrypt(or01) << " (exp 1)\n";
    
    // Test current or_gate
    auto or00_gate = fhe.or_gate(ct0, ct0);
    std::cout << "OR(0,0) via or_gate = " << fhe.decrypt(or00_gate) << " (exp 0)\n";
    
    return 0;
}
