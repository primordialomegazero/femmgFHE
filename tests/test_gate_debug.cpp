// GATE DEBUG — Saan exactly nagfa-fail ang composed gates?
#include "../src/fhe/golden_io_fhe.h"
#include <iostream>

int main() {
    std::cout << "GATE DEBUG\n";
    std::cout << "==========\n\n";
    
    golden_io_fhe::GoldenIOFHE fhe(42);
    
    // I-test ang basic gates muna
    std::cout << "1. BASIC GATES:\n";
    auto ct0 = fhe.encrypt(0);
    auto ct1 = fhe.encrypt(1);
    
    std::cout << "   NOT(0) = " << fhe.decrypt(fhe.not_gate(ct0)) << " (exp 1)\n";
    std::cout << "   NOT(1) = " << fhe.decrypt(fhe.not_gate(ct1)) << " (exp 0)\n";
    std::cout << "   NAND(0,0) = " << fhe.decrypt(fhe.not_gate(ct0)) << " (exp 1)\n";
    std::cout << "   NAND(1,1) = " << fhe.decrypt(fhe.not_gate(ct1)) << " (exp 0)\n\n";
    
    // XOR decomposition
    std::cout << "2. XOR DECOMPOSITION (4 NAND gates):\n";
    auto n1 = fhe.nand(ct0, ct1);       // NAND(0,1) = 1
    std::cout << "   Step 1: NAND(0,1) = " << fhe.decrypt(n1) << " (exp 1)\n";
    
    auto n2 = fhe.nand(ct0, n1);        // NAND(0,1) = 1
    std::cout << "   Step 2: NAND(0,1) = " << fhe.decrypt(n2) << " (exp 1)\n";
    
    auto n3 = fhe.nand(ct1, n1);        // NAND(1,1) = 0
    std::cout << "   Step 3: NAND(1,1) = " << fhe.decrypt(n3) << " (exp 0)\n";
    
    auto xor_result = fhe.nand(n2, n3); // NAND(1,0) = 1
    std::cout << "   Step 4: NAND(1,0) = " << fhe.decrypt(xor_result) << " (exp 1)\n\n";
    
    // AND decomposition
    std::cout << "3. AND DECOMPOSITION (2 NAND gates):\n";
    auto a1 = fhe.nand(ct0, ct1);       // NAND(0,1) = 1
    std::cout << "   Step 1: NAND(0,1) = " << fhe.decrypt(a1) << " (exp 1)\n";
    
    auto a2 = fhe.nand(a1, a1);         // NOT(NAND(0,1)) = 0
    std::cout << "   Step 2: NOT(1) = " << fhe.decrypt(a2) << " (exp 0)\n\n";
    
    // FULL ADDER decomposition — unang part lang
    std::cout << "4. FULL ADDER (first half):\n";
    auto xor_ab = fhe.xor_gate(ct0, ct1);
    std::cout << "   XOR(0,1) = " << fhe.decrypt(xor_ab) << " (exp 1)\n";
    
    auto sum = fhe.xor_gate(xor_ab, ct1);
    std::cout << "   XOR(XOR(0,1), 1) = " << fhe.decrypt(sum) << " (exp 0)\n";
    
    return 0;
}
