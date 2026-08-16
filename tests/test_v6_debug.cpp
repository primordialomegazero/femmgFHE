// V6 Debug
#include "../src/fhe/golden_fibonacci_fhe_v6.h"
#include <iostream>

int main() {
    std::cout << "V6 DEBUG\n";
    std::cout << "========\n\n";
    
    NTL::ZZ Q = NTL::to_ZZ("115792089237316195423570985008687907853269984665640564039457584007913129640731");
    golden_fhe_v6::FibonacciFHEV6 fhe(Q, 42);
    
    auto ct0 = fhe.encrypt(false);
    auto ct1 = fhe.encrypt(true);
    
    std::cout << "ct0 decrypts to: " << fhe.decrypt(ct0) << "\n";
    std::cout << "ct1 decrypts to: " << fhe.decrypt(ct1) << "\n\n";
    
    auto nand11 = fhe.nand_gate(ct1, ct1);
    std::cout << "NAND(1,1) = " << fhe.decrypt(nand11) << " (exp 0)\n";
    
    auto xor01 = fhe.xor_gate(ct0, ct1);
    std::cout << "XOR(0,1) = " << fhe.decrypt(xor01) << " (exp 1)\n";
    
    auto cnot10 = fhe.cnot(ct1, ct0);
    std::cout << "CNOT(1,0) = " << fhe.decrypt(cnot10) << " (exp 1)\n";
    
    // Check α and β
    std::cout << "\nα = " << fhe.alpha << "\n";
    std::cout << "β = " << fhe.beta << "\n";
    std::cout << "s_val = " << fhe.s_val << "\n";
    std::cout << "golden_plain = " << fhe.golden_plain << "\n";
    
    // Verify s² = α·s + β
    NTL::ZZ s_sq = (fhe.s_val * fhe.s_val) % Q;
    NTL::ZZ asb = (fhe.alpha * fhe.s_val + fhe.beta) % Q;
    std::cout << "s² = α·s + β: " << (s_sq == asb ? "YES ✓" : "NO ✗") << "\n";
    
    return 0;
}
