// EVEN NAND FRACTAL — Lahat ng operations ay PAIRED!
#include "openfhe.h"
#include <iostream>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  EVEN NAND FRACTAL — FIXED!\n";
    std::cout << "  (Lahat ng NANDs ay PAIRED)\n";
    std::cout << "========================================\n\n";

    CCParams<CryptoContextBFVRNS> parameters;
    parameters.SetPlaintextModulus(65537);
    parameters.SetMultiplicativeDepth(20);
    parameters.SetRingDim(32768);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(parameters);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);

    auto keys = cc->KeyGen();
    cc->EvalMultKeyGen(keys.secretKey);

    auto ct0 = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext({0}));
    auto ct1 = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext({1}));

    auto nand = [&](Ciphertext<DCRTPoly> a, Ciphertext<DCRTPoly> b) {
        auto ab = cc->EvalMult(a, b);
        return cc->EvalSub(ct1, ab);
    };

    auto decrypt = [&](Ciphertext<DCRTPoly> ct) -> int64_t {
        Plaintext pt;
        cc->Decrypt(keys.secretKey, ct, &pt);
        return pt->GetPackedValue()[0];
    };

    // ============================================
    // EVEN NAND: Laging PAIRED para sa period-2
    // ============================================
    
    // Instead of: NAND(x,x) → flips (1 NAND)
    // Use: NAND(NAND(x,x), NAND(x,x)) → resets (2 NANDs)
    
    auto even_not = [&](Ciphertext<DCRTPoly> x) {
        // 2 NANDs = period-2 reset = identity
        auto temp = nand(x, x);
        return nand(temp, temp);
    };
    
    auto odd_not = [&](Ciphertext<DCRTPoly> x) {
        // 1 NAND = flip
        return nand(x, x);
    };

    // ============================================
    // TEST 1: EVEN vs ODD NOT
    // ============================================
    std::cout << "1. EVEN vs ODD NOT:\n";
    std::cout << "--------------------\n";
    
    std::cout << "  Odd NOT(0) = " << decrypt(odd_not(ct0)) << " (expected 1)\n";
    std::cout << "  Odd NOT(1) = " << decrypt(odd_not(ct1)) << " (expected 0)\n";
    std::cout << "  Even NOT(0) = " << decrypt(even_not(ct0)) << " (expected 0 — identity!)\n";
    std::cout << "  Even NOT(1) = " << decrypt(even_not(ct1)) << " (expected 1 — identity!)\n\n";

    // ============================================
    // TEST 2: EVEN NAND FRACTAL LEVEL 1-3
    // ============================================
    std::cout << "2. EVEN NAND FRACTAL (Levels 1-3):\n";
    std::cout << "-----------------------------------\n";
    
    auto level1_even = [&](Ciphertext<DCRTPoly> q1, Ciphertext<DCRTPoly> q2) {
        // 2 NANDs (even) para sa period-2
        auto temp = nand(q1, q2);
        return nand(temp, temp);
    };
    
    auto level2_even = [&](Ciphertext<DCRTPoly> q1, Ciphertext<DCRTPoly> q2) {
        // 4 NANDs (even) — Level 1 composed with itself
        auto first = level1_even(q1, q2);
        return level1_even(first, q2);
    };
    
    auto level3_even = [&](Ciphertext<DCRTPoly> q1, Ciphertext<DCRTPoly> q2) {
        // 6 NANDs (even) — Level 2 + Level 1
        auto first = level2_even(q1, q2);
        return level1_even(first, q2);
    };
    
    std::cout << "  Level 1 (2 NANDs): ";
    int l1_ok = 0;
    for (int a = 0; a <= 1; a++) {
        for (int b = 0; b <= 1; b++) {
            auto q1 = a ? ct1 : ct0;
            auto q2 = b ? ct1 : ct0;
            int val = decrypt(level1_even(q1, q2));
            int expected = a & b;  // AND (kasi NAND(NAND) = AND)
            if (val == expected) l1_ok++;
        }
    }
    std::cout << l1_ok << "/4 correct\n";
    
    std::cout << "  Level 2 (4 NANDs): ";
    int l2_ok = 0;
    for (int a = 0; a <= 1; a++) {
        for (int b = 0; b <= 1; b++) {
            auto q1 = a ? ct1 : ct0;
            auto q2 = b ? ct1 : ct0;
            int val = decrypt(level2_even(q1, q2));
            int expected = a & b;
            if (val == expected) l2_ok++;
        }
    }
    std::cout << l2_ok << "/4 correct\n";
    
    std::cout << "  Level 3 (6 NANDs): ";
    int l3_ok = 0;
    for (int a = 0; a <= 1; a++) {
        for (int b = 0; b <= 1; b++) {
            auto q1 = a ? ct1 : ct0;
            auto q2 = b ? ct1 : ct0;
            int val = decrypt(level3_even(q1, q2));
            int expected = a & b;
            if (val == expected) l3_ok++;
        }
    }
    std::cout << l3_ok << "/4 correct\n\n";

    // ============================================
    // TEST 3: EVEN NAND CHAIN (20 levels)
    // ============================================
    std::cout << "3. EVEN NAND CHAIN (20 levels):\n";
    std::cout << "--------------------------------\n";
    
    auto current = ct1;
    int errors = 0;
    for (int i = 0; i < 20; i++) {
        // Every iteration: 2 NANDs (even) = period-2 reset
        auto temp = nand(current, current);
        current = nand(temp, temp);
        
        int val = decrypt(current);
        if (val != 1) errors++;
    }
    std::cout << "  20 even-NAND iterations: " 
              << (errors == 0 ? "✅ ALL CORRECT!" : "❌ " + std::to_string(errors) + " errors") << "\n";

    // ============================================
    // CONCLUSION
    // ============================================
    std::cout << "\n========================================\n";
    if (l1_ok == 4 && l2_ok == 4 && l3_ok == 4 && errors == 0) {
        std::cout << "  ✅ EVEN NAND STRATEGY WORKS!\n";
        std::cout << "  ✅ Levels 1-3 ALL CORRECT!\n";
        std::cout << "  ✅ 20 iterations NO ERRORS!\n";
        std::cout << "  ✅ PERIOD-2 RESET CONFIRMED!\n";
        std::cout << "  ✅ DEEP CIRCUITS POSSIBLE!\n";
    } else {
        std::cout << "  ❌ L1=" << l1_ok << "/4, L2=" << l2_ok 
                  << "/4, L3=" << l3_ok << "/4, Chain errors=" << errors << "\n";
    }
    std::cout << "========================================\n";

    return 0;
}
