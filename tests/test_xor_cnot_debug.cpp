// XOR + CNOT Debug
#include "../src/fhe/golden_fibonacci_fhe_v6.h"
#include <iostream>

int main() {
    std::cout << "XOR + CNOT DEBUG\n";
    std::cout << "================\n\n";
    
    NTL::ZZ Q = NTL::to_ZZ("32317006071311007300714876688669951960444102669715484032130345427524655138867890893197201411522913463688717960921898019494119559150490921095088152386448283120630877367300996091750197750389652106796057638384067568276792218642619756161838094338476170470581645852036305042887575891541065808607552399123930385521914333389668342420684974786564569494856176035326322058077805659331026192708460314150258592864177116725943603718461857357598351152301645904403697613233287231227125684710820209725157101726931323469678542580656697935045997268352998638215525166389437335543602135433229604645318478604952148193555853611059596217801");
    
    golden_fhe_v6::FibonacciFHEV6 fhe(Q, 42);
    
    auto ct0 = fhe.encrypt(false);
    auto ct1 = fhe.encrypt(true);
    
    // XOR step by step
    std::cout << "XOR(0,1) decomposition:\n";
    auto n1 = fhe.raw_nand(ct0, ct1);       // NAND(0,1) = 1
    std::cout << "  Step 1: NAND(0,1) = " << fhe.decrypt(n1) << " (exp 1)\n";
    
    auto n2 = fhe.raw_nand(ct0, n1);       // NAND(0, NAND(0,1)) = NAND(0,1) = 1
    std::cout << "  Step 2: NAND(0,n1) = " << fhe.decrypt(n2) << " (exp 1)\n";
    
    auto n3 = fhe.raw_nand(ct1, n1);       // NAND(1, NAND(0,1)) = NAND(1,1) = 0
    std::cout << "  Step 3: NAND(1,n1) = " << fhe.decrypt(n3) << " (exp 0)\n";
    
    auto xor_result = fhe.nand_gate(n2, n3);  // NAND(1, 0) = 1
    std::cout << "  Step 4: NAND(n2,n3) = " << fhe.decrypt(xor_result) << " (exp 1)\n\n";
    
    // Direct XOR
    auto xor_direct = fhe.xor_gate(ct0, ct1);
    std::cout << "  Direct XOR(0,1) = " << fhe.decrypt(xor_direct) << " (exp 1)\n";
    
    // CNOT
    auto cnot_result = fhe.cnot(ct1, ct0);
    std::cout << "  CNOT(1,0) = " << fhe.decrypt(cnot_result) << " (exp 1)\n";
    
    return 0;
}
