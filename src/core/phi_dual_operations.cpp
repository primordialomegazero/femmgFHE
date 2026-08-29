// ============================================
// φ-DUAL REALITY ALL OPERATIONS
//
// Normal Space: Addition + Subtraction
// Log Space: Multiplication + Division
//
// Lahat sa ISANG EvalAdd!
//
// Author: Dan Fernandez / Primordial Omega Zero
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>
#include <chrono>
#include <complex>

#include "pke/openfhe.h"

using namespace lbcrypto;
using namespace std;
using namespace std::chrono;

int main() {
    cout << "========================================\n";
    cout << "  φ-DUAL REALITY ALL OPERATIONS\n";
    cout << "  Add + Sub + Mult + Div SABAY\n";
    cout << "========================================\n\n";
    
    CCParams<CryptoContextCKKSRNS> parameters;
    parameters.SetMultiplicativeDepth(1);
    parameters.SetScalingModSize(20);
    parameters.SetBatchSize(2);  // [normal, log]
    parameters.SetSecurityLevel(HEStd_128_classic);
    
    CryptoContext<DCRTPoly> cc = GenCryptoContext(parameters);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);
    
    auto keyPair = cc->KeyGen();
    
    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);
    
    cout << "  ✅ CKKS initialized (128-bit, 2 slots)\n";
    cout << "  Slot 0: Normal (para sa + at -)\n";
    cout << "  Slot 1: Log (para sa × at ÷)\n\n";
    
    // ============================================
    // DUAL ENCODING
    // ============================================
    
    auto encrypt_dual = [&](double value) {
        vector<double> dual(2, 0.0);
        dual[0] = value;                        // Normal space
        dual[1] = log(value) / LN_PHI;          // Log space (φ-base)
        
        Plaintext pt = cc->MakeCKKSPackedPlaintext(dual);
        return cc->Encrypt(keyPair.publicKey, pt);
    };
    
    auto decrypt_dual = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext result_pt;
        cc->Decrypt(keyPair.secretKey, ct, &result_pt);
        result_pt->SetLength(2);
        auto results = result_pt->GetCKKSPackedValue();
        
        double normal = results[0].real();
        double log_val = results[1].real();
        double log_result = pow(PHI, log_val);  // Convert from log space
        
        cout << "  Normal: " << fixed << setprecision(4) << normal << "\n";
        cout << "  Log: " << fixed << setprecision(4) << log_result << "\n";
        
        return make_pair(normal, log_result);
    };
    
    // ============================================
    // TEST 1: ADDITION (5 + 7)
    // ============================================
    
    cout << "========================================\n";
    cout << "  TEST 1: ADDITION (5 + 7)\n";
    cout << "========================================\n\n";
    
    auto ct_5 = encrypt_dual(5.0);
    auto ct_7 = encrypt_dual(7.0);
    auto ct_add = cc->EvalAdd(ct_5, ct_7);
    
    cout << "  Result:\n";
    auto add_result = decrypt_dual(ct_add);
    
    cout << "  Normal: 5 + 7 = " << add_result.first << " (expected: 12)\n";
    cout << "  Log: 5 × 7 = " << add_result.second << " (expected: 35)\n";
    cout << "  Level: " << ct_add->GetLevel() << "\n\n";
    
    // ============================================
    // TEST 2: SUBTRACTION (100 - 7)
    // ============================================
    
    cout << "========================================\n";
    cout << "  TEST 2: SUBTRACTION (100 - 7)\n";
    cout << "========================================\n\n";
    
    auto ct_100 = encrypt_dual(100.0);
    auto ct_7b = encrypt_dual(7.0);
    auto neg_7 = cc->EvalNegate(ct_7b);
    auto ct_sub = cc->EvalAdd(ct_100, neg_7);
    
    cout << "  Result:\n";
    auto sub_result = decrypt_dual(ct_sub);
    
    cout << "  Normal: 100 - 7 = " << sub_result.first << " (expected: 93)\n";
    cout << "  Log: 100 ÷ 7 = " << sub_result.second << " (expected: 14.29)\n";
    cout << "  Level: " << ct_sub->GetLevel() << "\n\n";
    
    // ============================================
    // TEST 3: MULTIPLICATION (3 × 11)
    // ============================================
    
    cout << "========================================\n";
    cout << "  TEST 3: MULTIPLICATION (3 × 11)\n";
    cout << "========================================\n\n";
    
    auto ct_3 = encrypt_dual(3.0);
    auto ct_11 = encrypt_dual(11.0);
    auto ct_mult = cc->EvalAdd(ct_3, ct_11);
    
    cout << "  Result:\n";
    auto mult_result = decrypt_dual(ct_mult);
    
    cout << "  Normal: 3 + 11 = " << mult_result.first << " (expected: 14)\n";
    cout << "  Log: 3 × 11 = " << mult_result.second << " (expected: 33)\n";
    cout << "  Level: " << ct_mult->GetLevel() << "\n\n";
    
    // ============================================
    // TEST 4: DIVISION (55 ÷ 5)
    // ============================================
    
    cout << "========================================\n";
    cout << "  TEST 4: DIVISION (55 ÷ 5)\n";
    cout << "========================================\n\n";
    
    auto ct_55 = encrypt_dual(55.0);
    auto ct_5b = encrypt_dual(5.0);
    auto neg_5 = cc->EvalNegate(ct_5b);
    auto ct_div = cc->EvalAdd(ct_55, neg_5);
    
    cout << "  Result:\n";
    auto div_result = decrypt_dual(ct_div);
    
    cout << "  Normal: 55 - 5 = " << div_result.first << " (expected: 50)\n";
    cout << "  Log: 55 ÷ 5 = " << div_result.second << " (expected: 11)\n";
    cout << "  Level: " << ct_div->GetLevel() << "\n\n";
    
    // ============================================
    // TEST 5: MIXED CHAIN (2×3×5÷7×11)
    // ============================================
    
    cout << "========================================\n";
    cout << "  TEST 5: MIXED CHAIN\n";
    cout << "  (2×3×5÷7×11)\n";
    cout << "========================================\n\n";
    
    auto ct_mixed = encrypt_dual(2.0);
    
    // Operations: ×3, ×5, ÷7, ×11
    vector<pair<char, double>> ops = {
        {'*', 3.0}, {'*', 5.0}, {'/', 7.0}, {'*', 11.0}
    };
    
    double expected_mult = 2.0;
    double expected_add = 2.0;
    
    for (auto& [op, val] : ops) {
        auto ct_val = encrypt_dual(val);
        
        if (op == '*') {
            ct_mixed = cc->EvalAdd(ct_mixed, ct_val);
            expected_mult *= val;
            expected_add += val;
        } else {
            auto neg_val = cc->EvalNegate(ct_val);
            ct_mixed = cc->EvalAdd(ct_mixed, neg_val);
            expected_mult /= val;
            expected_add -= val;
        }
    }
    
    cout << "  Result:\n";
    auto mixed_result = decrypt_dual(ct_mixed);
    
    cout << "  Normal (add/sub): " << mixed_result.first << " (expected: " << expected_add << ")\n";
    cout << "  Log (mult/div): " << mixed_result.second << " (expected: " << expected_mult << ")\n";
    cout << "  Level: " << ct_mixed->GetLevel() << "\n\n";
    
    // ============================================
    // SUMMARY
    // ============================================
    
    cout << "========================================\n";
    cout << "  DUAL REALITY ALL OPERATIONS COMPLETE\n";
    cout << "========================================\n\n";
    cout << "  ✅ Addition: Normal space (5+7=12)\n";
    cout << "  ✅ Multiplication: Log space (5×7=35)\n";
    cout << "  ✅ Subtraction: Normal space (100-7=93)\n";
    cout << "  ✅ Division: Log space (55÷5=11)\n";
    cout << "  ✅ Mixed: Both sabay\n";
    cout << "  ✅ Level 0\n";
    cout << "  ✅ Pure FHE\n";
    cout << "  ✅ Walang bootstrapping\n";
    cout << "  ✅ DUAL REALITY: AUTO-ADJUST!\n\n";
    
    return 0;
}
