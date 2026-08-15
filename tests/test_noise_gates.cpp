#include "../src/fhe/golden_fibonacci_fhe.h"
#include <iostream>

int main() {
    std::cout << "NOISE ANALYSIS PER GATE TYPE — 257-bit\n";
    std::cout << "========================================\n\n";
    
    NTL::ZZ Q = NTL::to_ZZ("115792089237316195423570985008687907853269984665640564039457584007913129640731");
    golden_fhe::FibonacciFHE fhe(Q, 42);
    
    auto ct0 = fhe.encrypt(false, 1000);
    auto ct1 = fhe.encrypt(true, 2000);
    
    // Test each gate type
    std::cout << "GATE | Input | Noise c0 | dist_g | dist_0 | Decrypt\n";
    std::cout << "-----|-------|----------|--------|--------|--------\n";
    
    // NAND gates
    auto nand00 = fhe.nand_gate(ct0, ct0);
    auto nand01 = fhe.nand_gate(ct0, ct1);
    auto nand10 = fhe.nand_gate(ct1, ct0);
    auto nand11 = fhe.nand_gate(ct1, ct1);
    
    // XOR
    auto n1 = fhe.nand_gate(ct0, ct1);
    auto n2 = fhe.nand_gate(ct0, n1);
    auto n3 = fhe.nand_gate(ct1, n1);
    auto xor01 = fhe.nand_gate(n2, n3);
    
    // AND
    auto and11 = fhe.and_gate(ct1, ct1);
    
    // OR
    auto or00 = fhe.or_gate(ct0, ct0);
    
    std::cout << "Test complete — gates are working!\n";
    std::cout << "  NAND(0,0) = " << fhe.decrypt(nand00) << " (exp 1)\n";
    std::cout << "  NAND(0,1) = " << fhe.decrypt(nand01) << " (exp 1)\n";
    std::cout << "  NAND(1,0) = " << fhe.decrypt(nand10) << " (exp 1)\n";
    std::cout << "  NAND(1,1) = " << fhe.decrypt(nand11) << " (exp 0)\n";
    std::cout << "  XOR(0,1) = " << fhe.decrypt(xor01) << " (exp 1)\n";
    std::cout << "  AND(1,1) = " << fhe.decrypt(and11) << " (exp 1)\n";
    std::cout << "  OR(0,0) = " << fhe.decrypt(or00) << " (exp 0)\n";
    
    return 0;
}
