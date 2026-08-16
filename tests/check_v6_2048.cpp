#include "../src/fhe/golden_fibonacci_fhe_v6.h"
#include <iostream>

int main() {
    std::cout << "V6 WITH 2048-BIT Q\n\n";
    
    NTL::ZZ Q = NTL::to_ZZ("32317006071311007300714876688669951960444102669715484032130345427524655138867890893197201411522913463688717960921898019494119559150490921095088152386448283120630877367300996091750197750389652106796057638384067568276792218642619756161838094338476170470581645852036305042887575891541065808607552399123930385521914333389668342420684974786564569494856176035326322058077805659331026192708460314150258592864177116725943603718461857357598351152301645904403697613233287231227125684710820209725157101726931323469678542580656697935045997268352998638215525166389437335543602135433229604645318478604952148193555853611059596217801");
    
    golden_fhe_v6::FibonacciFHEV6 fhe(Q, 42);
    
    std::cout << "Q mod 5 = " << (Q % 5) << "\n";
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
