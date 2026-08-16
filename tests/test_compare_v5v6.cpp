#include "../src/fhe/golden_fibonacci_fhe_v5.h"
#include "../src/fhe/golden_fibonacci_fhe_v6.h"
#include <iostream>

int main() {
    NTL::ZZ Q = NTL::to_ZZ("340282366920938463463374607431768211297");
    
    golden_fhe_v5::FibonacciFHEV5 fhe5(Q, 42);
    golden_fhe_v6::FibonacciFHEV6 fhe6(Q, 42);
    
    std::cout << "=== V5 ===\n";
    std::cout << "golden_plain = " << fhe5.golden_plain << "\n";
    std::cout << "inv_golden = " << fhe5.inv_golden << "\n";
    std::cout << "s_val = " << fhe5.s_val << "\n";
    std::cout << "alpha = " << fhe5.alpha << "\n";
    std::cout << "beta = " << fhe5.beta << "\n";
    
    std::cout << "\n=== V6 ===\n";
    std::cout << "golden_plain = " << fhe6.golden_plain << "\n";
    std::cout << "inv_golden = " << fhe6.inv_golden << "\n";
    std::cout << "s_val = " << fhe6.s_val << "\n";
    std::cout << "alpha = " << fhe6.alpha << "\n";
    std::cout << "beta = " << fhe6.beta << "\n";
    
    // Test NAND sa V5
    auto ct1_v5 = fhe5.encrypt(true);
    auto nand_v5 = fhe5.nand_gate(ct1_v5, ct1_v5);
    std::cout << "\nV5 NAND(1,1): " << fhe5.decrypt(nand_v5) << "\n";
    
    // Test NAND sa V6
    auto ct1_v6 = fhe6.encrypt(true);
    auto nand_v6 = fhe6.nand_gate(ct1_v6, ct1_v6);
    std::cout << "V6 NAND(1,1): " << fhe6.decrypt(nand_v6) << "\n";
    
    return 0;
}
