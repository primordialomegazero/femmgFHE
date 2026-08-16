#include "../src/fhe/golden_fibonacci_fhe_v6.h"
#include <iostream>

int main() {
    NTL::ZZ Q = NTL::to_ZZ("4294967291");
    golden_fhe_v6::FibonacciFHEV6 fhe(Q, 42);
    
    auto ct0 = fhe.encrypt(false);
    auto ct1 = fhe.encrypt(true);
    
    std::cout << "NAND(0,0): " << fhe.decrypt(fhe.nand_gate(ct0, ct0)) << " exp 1\n";
    std::cout << "NAND(0,1): " << fhe.decrypt(fhe.nand_gate(ct0, ct1)) << " exp 1\n";
    std::cout << "NAND(1,0): " << fhe.decrypt(fhe.nand_gate(ct1, ct0)) << " exp 1\n";
    std::cout << "NAND(1,1): " << fhe.decrypt(fhe.nand_gate(ct1, ct1)) << " exp 0\n";
    
    return 0;
}
