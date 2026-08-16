#include "../src/fhe/golden_fibonacci_fhe_v6.h"
#include <iostream>

int main() {
    std::cout << "V6 TEST WITH VALID Q\n\n";
    
    // Valid 32-bit prime ≡ 1 mod 5
    NTL::ZZ Q = NTL::to_ZZ("4294967291");
    golden_fhe_v6::FibonacciFHEV6 fhe(Q, 42);
    
    auto ct0 = fhe.encrypt(false);
    auto ct1 = fhe.encrypt(true);
    
    std::cout << "Q mod 5: " << (Q % 5) << " (should be 1)\n\n";
    
    std::cout << "Encrypt(0) → " << fhe.decrypt(ct0) << " (exp 0)\n";
    std::cout << "Encrypt(1) → " << fhe.decrypt(ct1) << " (exp 1)\n";
    std::cout << "NAND(1,1) → " << fhe.decrypt(fhe.nand_gate(ct1, ct1)) << " (exp 0)\n";
    std::cout << "NAND(0,0) → " << fhe.decrypt(fhe.nand_gate(ct0, ct0)) << " (exp 1)\n";
    std::cout << "NOT(0) → " << fhe.decrypt(fhe.not_gate(ct0)) << " (exp 1)\n";
    std::cout << "NOT(1) → " << fhe.decrypt(fhe.not_gate(ct1)) << " (exp 0)\n";
    std::cout << "AND(1,1) → " << fhe.decrypt(fhe.and_gate(ct1, ct1)) << " (exp 1)\n";
    std::cout << "OR(0,0) → " << fhe.decrypt(fhe.or_gate(ct0, ct0)) << " (exp 0)\n";
    std::cout << "XOR(0,1) → " << fhe.decrypt(fhe.xor_gate(ct0, ct1)) << " (exp 1)\n";
    std::cout << "CNOT(1,0) → " << fhe.decrypt(fhe.cnot(ct1, ct0)) << " (exp 1)\n";
    
    return 0;
}
