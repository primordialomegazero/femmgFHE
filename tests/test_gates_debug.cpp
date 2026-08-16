#include "../src/fhe/golden_fibonacci_fhe_v6.h"
#include <iostream>

int main() {
    std::cout << "GATES DEBUG - V6\n\n";
    
    NTL::ZZ Q = NTL::to_ZZ("340282366920938463463374607431768211297"); // 128-bit
    golden_fhe_v6::FibonacciFHEV6 fhe(Q, 42);
    
    auto ct0 = fhe.encrypt(false);
    auto ct1 = fhe.encrypt(true);
    
    std::cout << "NOT(0): " << fhe.decrypt(fhe.not_gate(ct0)) << " (exp 1)\n";
    std::cout << "NOT(1): " << fhe.decrypt(fhe.not_gate(ct1)) << " (exp 0)\n";
    std::cout << "NAND(0,0): " << fhe.decrypt(fhe.nand_gate(ct0, ct0)) << " (exp 1)\n";
    std::cout << "NAND(0,1): " << fhe.decrypt(fhe.nand_gate(ct0, ct1)) << " (exp 1)\n";
    std::cout << "NAND(1,0): " << fhe.decrypt(fhe.nand_gate(ct1, ct0)) << " (exp 1)\n";
    std::cout << "NAND(1,1): " << fhe.decrypt(fhe.nand_gate(ct1, ct1)) << " (exp 0)\n";
    
    std::cout << "\n--- Derived gates ---\n";
    std::cout << "AND(0,0): " << fhe.decrypt(fhe.and_gate(ct0, ct0)) << " (exp 0)\n";
    std::cout << "AND(0,1): " << fhe.decrypt(fhe.and_gate(ct0, ct1)) << " (exp 0)\n";
    std::cout << "AND(1,0): " << fhe.decrypt(fhe.and_gate(ct1, ct0)) << " (exp 0)\n";
    std::cout << "AND(1,1): " << fhe.decrypt(fhe.and_gate(ct1, ct1)) << " (exp 1)\n";
    
    std::cout << "\nOR(0,0): " << fhe.decrypt(fhe.or_gate(ct0, ct0)) << " (exp 0)\n";
    std::cout << "OR(0,1): " << fhe.decrypt(fhe.or_gate(ct0, ct1)) << " (exp 1)\n";
    std::cout << "OR(1,0): " << fhe.decrypt(fhe.or_gate(ct1, ct0)) << " (exp 1)\n";
    std::cout << "OR(1,1): " << fhe.decrypt(fhe.or_gate(ct1, ct1)) << " (exp 1)\n";
    
    std::cout << "\nXOR(0,0): " << fhe.decrypt(fhe.xor_gate(ct0, ct0)) << " (exp 0)\n";
    std::cout << "XOR(0,1): " << fhe.decrypt(fhe.xor_gate(ct0, ct1)) << " (exp 1)\n";
    std::cout << "XOR(1,0): " << fhe.decrypt(fhe.xor_gate(ct1, ct0)) << " (exp 1)\n";
    std::cout << "XOR(1,1): " << fhe.decrypt(fhe.xor_gate(ct1, ct1)) << " (exp 0)\n";
    
    return 0;
}
