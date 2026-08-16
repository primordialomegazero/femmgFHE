#include "../src/fhe/golden_fibonacci_fhe_v6.h"
#include <iostream>

int main() {
    std::cout << "TEST WITH LARGER Q\n\n";
    
    // Gumamit ng 64-bit prime ≡ 1 mod 5
    NTL::ZZ Q = NTL::to_ZZ("18446744073709551611"); // 2^64 - 5
    golden_fhe_v6::FibonacciFHEV6 fhe(Q, 42);
    
    std::cout << "Q mod 5 = " << (Q % 5) << " (should be 1)\n";
    std::cout << "φ - Q/2 = " << (fhe.golden_plain - Q/2) << "\n\n";
    
    auto ct0 = fhe.encrypt(false);
    auto ct1 = fhe.encrypt(true);
    
    std::cout << "NAND(0,0): " << fhe.decrypt(fhe.nand_gate(ct0, ct0)) << " exp 1\n";
    std::cout << "NAND(0,1): " << fhe.decrypt(fhe.nand_gate(ct0, ct1)) << " exp 1\n";
    std::cout << "NAND(1,0): " << fhe.decrypt(fhe.nand_gate(ct1, ct0)) << " exp 1\n";
    std::cout << "NAND(1,1): " << fhe.decrypt(fhe.nand_gate(ct1, ct1)) << " exp 0\n";
    std::cout << "NOT(0): " << fhe.decrypt(fhe.not_gate(ct0)) << " exp 1\n";
    std::cout << "NOT(1): " << fhe.decrypt(fhe.not_gate(ct1)) << " exp 0\n";
    std::cout << "AND(1,1): " << fhe.decrypt(fhe.and_gate(ct1, ct1)) << " exp 1\n";
    std::cout << "OR(0,0): " << fhe.decrypt(fhe.or_gate(ct0, ct0)) << " exp 0\n";
    std::cout << "XOR(0,1): " << fhe.decrypt(fhe.xor_gate(ct0, ct1)) << " exp 1\n";
    std::cout << "CNOT(1,0): " << fhe.decrypt(fhe.cnot(ct1, ct0)) << " exp 1\n";
    
    return 0;
}
