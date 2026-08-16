// FHE AUDIT — Top-Down Verification
// Check kung LAHAT ng operations ay tunay na homomorphic

#include "../src/fhe/golden_fibonacci_fhe_v5.h"
#include <iostream>
#include <vector>

int main() {
    std::cout << "FHE AUDIT — TOP-DOWN VERIFICATION\n";
    std::cout << "=================================\n\n";
    
    NTL::ZZ Q = NTL::to_ZZ("115792089237316195423570985008687907853269984665640564039457584007913129640731");
    golden_fhe_v5::FibonacciFHEV5 fhe(Q, 42);
    
    int total = 0, passed = 0;
    
    // ============ 1. ENCRYPTION: Fresh nonce every time ============
    std::cout << "1. ENCRYPTION (Fresh nonce test)\n";
    std::cout << "   Testing if encrypt(1) gives DIFFERENT ciphertexts...\n";
    
    auto ct1_a = fhe.encrypt(true);
    auto ct1_b = fhe.encrypt(true);
    
    bool different = (NTL::coeff(ct1_a.first, 0) != NTL::coeff(ct1_b.first, 0) ||
                      NTL::coeff(ct1_a.second, 0) != NTL::coeff(ct1_b.second, 0));
    
    total++; passed += different;
    std::cout << "   encrypt(1) twice → " << (different ? "DIFFERENT ✓" : "SAME ✗") << "\n\n";
    
    // ============ 2. NOT GATE: Homomorphic NAND ============
    std::cout << "2. NOT GATE (Homomorphic test)\n";
    std::cout << "   NOT(a) = NAND(a,a) — proper computation\n";
    
    auto not1 = fhe.not_gate(ct1_a);
    auto not0 = fhe.not_gate(fhe.encrypt(false));
    
    bool not1_correct = (fhe.decrypt(not1) == 0);
    bool not0_correct = (fhe.decrypt(not0) == 1);
    
    total += 2; passed += not1_correct + not0_correct;
    std::cout << "   NOT(1) = " << fhe.decrypt(not1) << " (exp 0) " << (not1_correct ? "✓" : "✗") << "\n";
    std::cout << "   NOT(0) = " << fhe.decrypt(not0) << " (exp 1) " << (not0_correct ? "✓" : "✗") << "\n\n";
    
    // ============ 3. NAND GATE: Actual multiplication ============
    std::cout << "3. NAND GATE (Multiplication test)\n";
    std::cout << "   NAND(a,b) = golden_plain - (a·b)·inv_golden\n";
    
    auto nand11 = fhe.nand_gate(ct1_a, ct1_b);
    auto nand00 = fhe.nand_gate(fhe.encrypt(false), fhe.encrypt(false));
    
    bool nand11_correct = (fhe.decrypt(nand11) == 0);
    bool nand00_correct = (fhe.decrypt(nand00) == 1);
    
    total += 2; passed += nand11_correct + nand00_correct;
    std::cout << "   NAND(1,1) = " << fhe.decrypt(nand11) << " (exp 0) " << (nand11_correct ? "✓" : "✗") << "\n";
    std::cout << "   NAND(0,0) = " << fhe.decrypt(nand00) << " (exp 1) " << (nand00_correct ? "✓" : "✗") << "\n\n";
    
    // ============ 4. XOR GATE: 4 NAND gates ============
    std::cout << "4. XOR GATE (4 NAND gates)\n";
    
    auto xor01 = fhe.xor_gate(ct1_a, fhe.encrypt(false));
    auto xor00 = fhe.xor_gate(fhe.encrypt(false), fhe.encrypt(false));
    
    bool xor01_correct = (fhe.decrypt(xor01) == 1);
    bool xor00_correct = (fhe.decrypt(xor00) == 0);
    
    total += 2; passed += xor01_correct + xor00_correct;
    std::cout << "   XOR(1,0) = " << fhe.decrypt(xor01) << " (exp 1) " << (xor01_correct ? "✓" : "✗") << "\n";
    std::cout << "   XOR(0,0) = " << fhe.decrypt(xor00) << " (exp 0) " << (xor00_correct ? "✓" : "✗") << "\n\n";
    
    // ============ 5. AND GATE ============
    std::cout << "5. AND GATE\n";
    
    auto and11 = fhe.and_gate(ct1_a, ct1_b);
    auto and10 = fhe.and_gate(ct1_a, fhe.encrypt(false));
    
    bool and11_correct = (fhe.decrypt(and11) == 1);
    bool and10_correct = (fhe.decrypt(and10) == 0);
    
    total += 2; passed += and11_correct + and10_correct;
    std::cout << "   AND(1,1) = " << fhe.decrypt(and11) << " (exp 1) " << (and11_correct ? "✓" : "✗") << "\n";
    std::cout << "   AND(1,0) = " << fhe.decrypt(and10) << " (exp 0) " << (and10_correct ? "✓" : "✗") << "\n\n";
    
    // ============ 6. OR GATE ============
    std::cout << "6. OR GATE\n";
    
    auto or00 = fhe.or_gate(fhe.encrypt(false), fhe.encrypt(false));
    auto or10 = fhe.or_gate(ct1_a, fhe.encrypt(false));
    
    bool or00_correct = (fhe.decrypt(or00) == 0);
    bool or10_correct = (fhe.decrypt(or10) == 1);
    
    total += 2; passed += or00_correct + or10_correct;
    std::cout << "   OR(0,0) = " << fhe.decrypt(or00) << " (exp 0) " << (or00_correct ? "✓" : "✗") << "\n";
    std::cout << "   OR(1,0) = " << fhe.decrypt(or10) << " (exp 1) " << (or10_correct ? "✓" : "✗") << "\n\n";
    
    // ============ 7. COMPOSITION: NOT(NOT(x)) = x ============
    std::cout << "7. COMPOSITION: NOT(NOT(x)) = x\n";
    
    auto not_not_1 = fhe.not_gate(fhe.not_gate(ct1_a));
    bool nn_correct = (fhe.decrypt(not_not_1) == 1);
    
    total++; passed += nn_correct;
    std::cout << "   NOT(NOT(1)) = " << fhe.decrypt(not_not_1) << " (exp 1) " << (nn_correct ? "✓" : "✗") << "\n\n";
    
    // ============ 8. DEEP COMPOSITION (100 gates) ============
    std::cout << "8. DEEP COMPOSITION (100 NOT gates)\n";
    
    auto deep = ct1_a;
    for (int i = 0; i < 100; i++) {
        deep = fhe.not_gate(deep);
    }
    bool deep_correct = (fhe.decrypt(deep) == 1);  // 100 NOTs = identity
    
    total++; passed += deep_correct;
    std::cout << "   100 NOTs = " << fhe.decrypt(deep) << " (exp 1) " << (deep_correct ? "✓" : "✗") << "\n\n";
    
    // ============ SUMMARY ============
    std::cout << "========================================\n";
    std::cout << "FHE AUDIT SUMMARY:\n";
    std::cout << "  Fresh encryption: " << (different ? "PASS ✓" : "FAIL ✗") << "\n";
    std::cout << "  NOT gate: " << (not1_correct && not0_correct ? "PASS ✓" : "FAIL ✗") << "\n";
    std::cout << "  NAND gate: " << (nand11_correct && nand00_correct ? "PASS ✓" : "FAIL ✗") << "\n";
    std::cout << "  XOR gate: " << (xor01_correct && xor00_correct ? "PASS ✓" : "FAIL ✗") << "\n";
    std::cout << "  AND gate: " << (and11_correct && and10_correct ? "PASS ✓" : "FAIL ✗") << "\n";
    std::cout << "  OR gate: " << (or00_correct && or10_correct ? "PASS ✓" : "FAIL ✗") << "\n";
    std::cout << "  Composition: " << (nn_correct ? "PASS ✓" : "FAIL ✗") << "\n";
    std::cout << "  Deep: " << (deep_correct ? "PASS ✓" : "FAIL ✗") << "\n";
    std::cout << "----------------------------------------\n";
    std::cout << "  Total: " << passed << "/" << total << " PASS\n";
    std::cout << "========================================\n";
    
    return (passed == total) ? 0 : 1;
}
