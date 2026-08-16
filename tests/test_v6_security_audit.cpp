// V6 SECURITY AUDIT
#include "../src/fhe/golden_fibonacci_fhe_v6.h"
#include <iostream>

int main() {
    std::cout << "V6 SECURITY AUDIT\n";
    std::cout << "=================\n\n";
    
    NTL::ZZ Q = NTL::to_ZZ("115792089237316195423570985008687907853269984665640564039457584007913129640731");
    golden_fhe_v6::FibonacciFHEV6 fhe(Q, 42);
    
    // Test fresh encryption
    auto ct1 = fhe.encrypt(true);
    auto ct1_b = fhe.encrypt(true);
    bool fresh = (NTL::coeff(ct1.first, 0) != NTL::coeff(ct1_b.first, 0));
    std::cout << "Fresh encryption: " << (fresh ? "PASS ✓" : "FAIL ✗") << "\n";
    
    // Test gates
    auto ct0 = fhe.encrypt(false);
    bool gates = (fhe.decrypt(fhe.nand_gate(ct1, ct1)) == 0 &&
                  fhe.decrypt(fhe.xor_gate(ct0, ct1)) == 1 &&
                  fhe.decrypt(fhe.and_gate(ct1, ct1)) == 1 &&
                  fhe.decrypt(fhe.or_gate(ct0, ct0)) == 0);
    std::cout << "All gates: " << (gates ? "PASS ✓" : "FAIL ✗") << "\n";
    
    // Test quantum
    bool quantum = (fhe.decrypt(fhe.cnot(ct1, ct0)) == 1);
    std::cout << "Quantum CNOT: " << (quantum ? "PASS ✓" : "FAIL ✗") << "\n";
    
    // Test blinding
    auto blinded = fhe.blind(ct1);
    bool blind_diff = (NTL::coeff(blinded.first, 0) != NTL::coeff(ct1.first, 0));
    std::cout << "Blinding randomizes: " << (blind_diff ? "PASS ✓" : "FAIL ✗") << "\n";
    
    return (fresh && gates && quantum && blind_diff) ? 0 : 1;
}
