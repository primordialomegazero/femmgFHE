// ALL GATES TEST — Classical + Quantum sa FHE Core
#include "../src/fhe/golden_fibonacci_fhe_v5.h"
#include <iostream>
#include <vector>

int main() {
    std::cout << "ALL GATES COMPLETE TEST\n";
    std::cout << "=======================\n\n";
    
    NTL::ZZ Q = NTL::to_ZZ("115792089237316195423570985008687907853269984665640564039457584007913129640731");
    golden_fhe_v5::FibonacciFHEV5 fhe(Q, 42);
    
    int total = 0, passed = 0;
    
    // Classical gates
    std::cout << "CLASSICAL GATES:\n";
    auto ct0 = fhe.encrypt(false);
    auto ct1 = fhe.encrypt(true);
    
    auto nand11 = fhe.nand_gate(ct1, ct1);
    total++; passed += (fhe.decrypt(nand11) == 0);
    std::cout << "  NAND(1,1) = " << fhe.decrypt(nand11) << " ✓\n";
    
    auto xor01 = fhe.xor_gate(ct0, ct1);
    total++; passed += (fhe.decrypt(xor01) == 1);
    std::cout << "  XOR(0,1) = " << fhe.decrypt(xor01) << " ✓\n";
    
    auto and11 = fhe.and_gate(ct1, ct1);
    total++; passed += (fhe.decrypt(and11) == 1);
    std::cout << "  AND(1,1) = " << fhe.decrypt(and11) << " ✓\n";
    
    auto or00 = fhe.or_gate(ct0, ct0);
    total++; passed += (fhe.decrypt(or00) == 0);
    std::cout << "  OR(0,0) = " << fhe.decrypt(or00) << " ✓\n\n";
    
    // Quantum gates
    std::cout << "QUANTUM GATES:\n";
    auto h1 = fhe.hadamard(ct1);
    total++; passed += (fhe.decrypt(h1) == 0);
    std::cout << "  H|1⟩ = " << fhe.decrypt(h1) << " ✓\n";
    
    auto cnot10 = fhe.cnot(ct1, ct0);
    total++; passed += (fhe.decrypt(cnot10) == 1);
    std::cout << "  CNOT(1,0) = " << fhe.decrypt(cnot10) << " ✓\n";
    
    auto s1 = fhe.phase_gate(ct1);
    total++; passed += (fhe.decrypt(s1) == 1);
    std::cout << "  S|1⟩ = " << fhe.decrypt(s1) << " ✓\n";
    
    auto t1 = fhe.t_gate(ct1);
    total++; passed += (fhe.decrypt(t1) == 0);
    std::cout << "  T|1⟩ = " << fhe.decrypt(t1) << " ✓\n";
    
    auto bell = fhe.bell_state(ct1, ct0);
    total++; passed += (fhe.decrypt(bell) == 0);
    std::cout << "  Bell(1,0) = " << fhe.decrypt(bell) << " ✓\n\n";
    
    std::cout << "RESULT: " << passed << "/" << total << " PASS\n";
    return passed == total ? 0 : 1;
}
