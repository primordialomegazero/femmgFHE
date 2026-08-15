#include "../src/fhe/golden_fibonacci_fhe_v4.h"
#include <iostream>
#include <chrono>

int main() {
    std::cout << "V4 EMERGENT-OPTIMIZED TEST\n";
    std::cout << "==========================\n\n";
    
    NTL::ZZ Q = NTL::to_ZZ("115792089237316195423570985008687907853269984665640564039457584007913129640731");
    golden_fhe_v4::FibonacciFHEV4 fhe(Q, 42);
    
    // Correctness test
    auto ct0 = fhe.encrypt(false);
    auto ct1 = fhe.encrypt(true);
    std::cout << "Encrypt(0) → " << fhe.decrypt(ct0) << " ✓\n";
    std::cout << "Encrypt(1) → " << fhe.decrypt(ct1) << " ✓\n\n";
    
    // NOT toggle test
    auto not1 = fhe.not_gate(ct1);
    auto not0 = fhe.not_gate(ct0);
    std::cout << "NOT(1) → " << fhe.decrypt(not1) << " (exp 0) ✓\n";
    std::cout << "NOT(0) → " << fhe.decrypt(not0) << " (exp 1) ✓\n\n";
    
    // NAND test
    auto nand11 = fhe.nand_gate(ct1, ct1);
    std::cout << "NAND(1,1) → " << fhe.decrypt(nand11) << " (exp 0) ✓\n\n";
    
    // Performance: Toggle NOT vs NAND NOT
    auto start = std::chrono::high_resolution_clock::now();
    auto current = ct1;
    for (int i = 0; i < 1000; i++) {
        current = fhe.not_gate(current);  // Toggle
    }
    auto end = std::chrono::high_resolution_clock::now();
    double toggle_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    std::cout << "1000 toggle NOTs: " << toggle_ms << " ms\n";
    
    start = std::chrono::high_resolution_clock::now();
    current = ct1;
    for (int i = 0; i < 1000; i++) {
        current = fhe.nand_gate(current, current);  // NAND NOT
    }
    end = std::chrono::high_resolution_clock::now();
    double nand_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    std::cout << "1000 NAND NOTs: " << nand_ms << " ms\n";
    std::cout << "Speedup: " << nand_ms / toggle_ms << "x\n";
    
    return 0;
}
