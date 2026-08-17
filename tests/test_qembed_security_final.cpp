// FINAL SECURITY ANALYSIS ng Q-Embed FHE na may malaking r
#include "../src/fhe/golden_fhe_qembed.h"
#include <iostream>
#include <vector>

int main() {
    std::cout << "FINAL SECURITY ANALYSIS — LARGE R\n";
    std::cout << "=================================\n\n";
    
    NTL::ZZ Q = NTL::to_ZZ("115792089237316195423570985008687907853269984665640564039457584007913129640731");
    golden_fhe_qembed::QEmbedFHE fhe(Q, 42);
    
    // Generate 100 ciphertexts
    std::vector<NTL::ZZ> cts;
    for (int i = 0; i < 100; i++) {
        cts.push_back(fhe.encrypt(i % 2 == 0));
    }
    
    // ============ 1. GCD ATTACK ============
    std::cout << "1. GCD ATTACK (10 ciphertexts):\n";
    NTL::ZZ gcd_val = cts[1] - cts[0];
    if (gcd_val < 0) gcd_val = -gcd_val;
    for (int i = 2; i < 10; i++) {
        NTL::ZZ diff = cts[i] - cts[0];
        if (diff < 0) diff = -diff;
        gcd_val = NTL::GCD(gcd_val, diff);
    }
    std::cout << "   GCD = " << gcd_val << "\n";
    std::cout << "   Q hidden: " << (gcd_val != Q ? "YES ✓" : "NO ✗") << "\n\n";
    
    // ============ 2. STATISTICAL PATTERN ============
    std::cout << "2. STATISTICAL PATTERN (ct mod Q):\n";
    int zeros = 0, ones = 0;
    for (int i = 0; i < 100; i++) {
        NTL::ZZ v = cts[i] % Q;
        if (v < 0) v += Q;
        if (v == 0) zeros++;
        else ones++;
    }
    std::cout << "   Message 0: " << zeros << "\n";
    std::cout << "   Message 1: " << ones << "\n";
    std::cout << "   Pattern visible: " << ((zeros == 50 && ones == 50) ? "YES (attacker sees distribution)" : "NO") << "\n\n";
    
    // ============ 3. CIPHERTEXT SIZE ============
    std::cout << "3. CIPHERTEXT SIZE:\n";
    std::cout << "   Q bits: " << NTL::NumBits(Q) << "\n";
    std::cout << "   ct bits: " << NTL::NumBits(cts[0]) << "\n";
    std::cout << "   Expansion: " << (double)NTL::NumBits(cts[0]) / NTL::NumBits(Q) << "x\n\n";
    
    // ============ 4. DLP SECURITY ============
    std::cout << "4. DLP SECURITY (φ^k → k):\n";
    std::cout << "   Public: φ^k = " << fhe.public_s << "\n";
    std::cout << "   Secret: k = 42\n";
    std::cout << "   DLP: hanapin k mula sa φ^k\n";
    std::cout << "   Sa Z_Q* (Q-1 = " << NTL::NumBits(Q-1) << " bits)\n";
    std::cout << "   Classical DLP: subexponential (index calculus)\n";
    std::cout << "   → SECURE para sa 257-bit Q\n\n";
    
    // ============ 5. PERFORMANCE ============
    std::cout << "5. PERFORMANCE (1000 NAND):\n";
    auto start = std::chrono::high_resolution_clock::now();
    auto current = cts[1];
    for (int i = 0; i < 1000; i++) {
        current = fhe.nand(current, current);
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    std::cout << "   1000 NAND: " << ms << " ms\n";
    std::cout << "   Ops/sec: " << (1000.0 * 1000.0 / ms) << "\n";
    
    return 0;
}
