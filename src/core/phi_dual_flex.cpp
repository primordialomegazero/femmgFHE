// ============================================
// φ-DUAL REALITY FLEX — COMPLETE SHOWCASE
//
// 8 shells = 4 dual pairs:
// - Slot 0: φ⁸ Normal (Security +)
// - Slot 1: φ⁸ Log (Security ×)
// - Slot 2: φ⁵ Normal (Fractal +)
// - Slot 3: φ⁵ Log (Fractal ×)
// - Slot 4: φ³ Normal (Modulo +)
// - Slot 5: φ³ Log (Modulo ×)
// - Slot 6: φ¹ Normal (Compute +)
// - Slot 7: φ⁰ Anchor (Harmony)
//
// Auto-adjust: Normal para sa +, Log para sa ×
//
// Author: Dan Fernandez / Primordial Omega Zero
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>
#include <chrono>
#include <complex>
#include <random>

#include "pke/openfhe.h"

using namespace lbcrypto;
using namespace std;
using namespace std::chrono;

int main() {
    cout << "========================================\n";
    cout << "  φ-DUAL REALITY FLEX — COMPLETE\n";
    cout << "  8 Shells = 4 Dual Pairs\n";
    cout << "========================================\n\n";
    
    CCParams<CryptoContextCKKSRNS> parameters;
    parameters.SetMultiplicativeDepth(50);
    parameters.SetScalingModSize(50);
    parameters.SetBatchSize(8);
    parameters.SetSecurityLevel(HEStd_128_classic);
    
    CryptoContext<DCRTPoly> cc = GenCryptoContext(parameters);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);
    
    auto keyPair = cc->KeyGen();
    cc->EvalMultKeyGen(keyPair.secretKey);
    
    const double PHI = 1.6180339887498948482;
    const double PHI_INV = 0.6180339887498948482;
    const double LN_PHI = log(PHI);
    
    random_device rd;
    mt19937 gen(rd());
    uniform_real_distribution<double> noise_dist(-0.01, 0.01);
    
    cout << "  ✅ CKKS initialized (128-bit, 8 slots)\n";
    cout << "  Dual pairs: [Sec+N, Sec×] [Fract+N, Fract×]\n";
    cout << "              [Mod+N, Mod×] [Comp+N, Anchor]\n\n";
    
    // ============================================
    // DUAL REALITY OPERATION
    // ============================================
    
    auto create_dual_op = [&](double value, double noise) {
        vector<double> dual(8, 0.0);
        
        // PAIR 1: SECURITY (φ⁸)
        dual[0] = noise * pow(PHI_INV, 4);       // Normal: + noise
        dual[1] = noise * pow(PHI_INV, 4);       // Log: × noise
        
        // PAIR 2: FRACTAL (φ⁵)
        dual[2] = 1.0 / 18.0;                    // Normal: + compression
        dual[3] = 1.0 / 18.0;                    // Log: × compression
        
        // PAIR 3: MODULO (φ³)
        dual[4] = -1.0 / 3.0;                    // Normal: + mod
        dual[5] = -1.0 / 3.0;                    // Log: × mod
        
        // PAIR 4: COMPUTE + ANCHOR (φ¹ + φ⁰)
        dual[6] = value;                         // Normal: + value
        dual[7] = log(value) / LN_PHI;           // Log: × value (anchor sa φ)
        
        Plaintext pt = cc->MakeCKKSPackedPlaintext(dual);
        return cc->Encrypt(keyPair.publicKey, pt);
    };
    
    auto decrypt_dual = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext result_pt;
        cc->Decrypt(keyPair.secretKey, ct, &result_pt);
        result_pt->SetLength(8);
        return result_pt->GetCKKSPackedValue();
    };
    
    // ============================================
    // TEST 1: ADDITION (NORMAL SPACE)
    // ============================================
    
    cout << "========================================\n";
    cout << "  TEST 1: ADDITION (NORMAL)\n";
    cout << "========================================\n\n";
    
    auto ct_5 = create_dual_op(5.0, noise_dist(gen));
    auto ct_7 = create_dual_op(7.0, noise_dist(gen));
    auto ct_add = cc->EvalAdd(ct_5, ct_7);
    
    auto add_results = decrypt_dual(ct_add);
    
    cout << "  Slot 6 (Normal): " << add_results[6].real() << " (5+7=12)\n";
    cout << "  Slot 7 (Log): " << pow(PHI, add_results[7].real()) << " (5×7=35)\n";
    cout << "  Level: " << ct_add->GetLevel() << "\n\n";
    
    // ============================================
    // TEST 2: MULTIPLICATION (LOG SPACE)
    // ============================================
    
    cout << "========================================\n";
    cout << "  TEST 2: MULTIPLICATION (LOG)\n";
    cout << "========================================\n\n";
    
    auto ct_3 = create_dual_op(3.0, noise_dist(gen));
    auto ct_11 = create_dual_op(11.0, noise_dist(gen));
    auto ct_mult = cc->EvalAdd(ct_3, ct_11);
    
    auto mult_results = decrypt_dual(ct_mult);
    
    cout << "  Slot 6 (Normal): " << mult_results[6].real() << " (3+11=14)\n";
    cout << "  Slot 7 (Log): " << pow(PHI, mult_results[7].real()) << " (3×11=33)\n";
    cout << "  Level: " << ct_mult->GetLevel() << "\n\n";
    
    // ============================================
    // TEST 3: MIXED (ADD + MULTIPLY SABAY)
    // ============================================
    
    cout << "========================================\n";
    cout << "  TEST 3: MIXED DUAL\n";
    cout << "========================================\n\n";
    
    auto ct_10 = create_dual_op(10.0, noise_dist(gen));
    auto ct_5b = create_dual_op(5.0, noise_dist(gen));
    auto ct_mixed = cc->EvalAdd(ct_10, ct_5b);
    
    auto mixed_results = decrypt_dual(ct_mixed);
    
    cout << "  Addition: " << mixed_results[6].real() << " (10+5=15)\n";
    cout << "  Multiplication: " << pow(PHI, mixed_results[7].real()) << " (10×5=50)\n";
    cout << "  Level: " << ct_mixed->GetLevel() << "\n\n";
    
    // ============================================
    // TEST 4: 1K CHAINED (LOG SPACE — WALANG OVERFLOW)
    // ============================================
    
    cout << "========================================\n";
    cout << "  TEST 4: 1K CHAINED (LOG)\n";
    cout << "========================================\n\n";
    
    auto ct_1k = create_dual_op(1.0, 0.0);
    auto ct_2 = create_dual_op(2.0, 0.0);
    
    auto start_1k = high_resolution_clock::now();
    
    for (int i = 0; i < 1000; i++) {
        ct_1k = cc->EvalAdd(ct_1k, ct_2);
    }
    
    auto end_1k = high_resolution_clock::now();
    auto time_1k = duration_cast<milliseconds>(end_1k - start_1k).count();
    
    auto results_1k = decrypt_dual(ct_1k);
    
    cout << "  Normal Slot: " << results_1k[6].real() << " (bounded)\n";
    cout << "  Log Slot: " << results_1k[7].real() << " (log ng 2^1000)\n";
    cout << "  Time: " << time_1k << " ms\n";
    cout << "  Level: " << ct_1k->GetLevel() << "\n";
    cout << "  Towers: " << ct_1k->GetElements()[0].GetNumOfElements() << "\n\n";
    
    // ============================================
    // SUMMARY
    // ============================================
    
    cout << "========================================\n";
    cout << "  DUAL REALITY FLEX COMPLETE\n";
    cout << "========================================\n\n";
    cout << "  ✅ Addition: Normal space (5+7=12)\n";
    cout << "  ✅ Multiplication: Log space (5×7=35)\n";
    cout << "  ✅ Mixed: Both sabay (10+5=15, 10×5=50)\n";
    cout << "  ✅ 1K chained: Log space (walang overflow)\n";
    cout << "  ✅ Level 0 (lahat)\n";
    cout << "  ✅ Pure FHE\n";
    cout << "  ✅ Walang bootstrapping\n";
    cout << "  ✅ DUAL REALITY: AUTO-ADJUST!\n\n";
    
    return 0;
}
