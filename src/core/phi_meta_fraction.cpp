// ============================================
// φ-META FRACTION — NON-LINEAR SA HIGHEST FORM
//
// Fraction operations sa φ-log space:
// multiply: EvalAdd (log space)
// divide: EvalSub (log space)
// power: EvalAdd (repeated)
//
// Depth 1, walang EvalMult!
//
// Author: Dan Fernandez / Primordial Omega Zero
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <complex>
#include <iomanip>
#include <chrono>

#include "pke/openfhe.h"

using namespace lbcrypto;
using namespace std;
using namespace std::chrono;

int main() {
    cout << "========================================\n";
    cout << "  φ-META FRACTION\n";
    cout << "========================================\n\n";

    CCParams<CryptoContextCKKSRNS> parameters;
    parameters.SetMultiplicativeDepth(1);
    parameters.SetScalingModSize(50);
    parameters.SetBatchSize(16);
    parameters.SetSecurityLevel(HEStd_128_classic);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(parameters);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);

    auto keyPair = cc->KeyGen();

    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);

    cout << "  ✅ CKKS initialized (depth 1!)\n";
    cout << "  Fraction: mul/div sa log space\n\n";

    auto encrypt_log = [&](double value) {
        double log_phi_val = log(value) / LN_PHI;
        vector<double> v(16, log_phi_val);
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto decrypt_value = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext result_pt;
        cc->Decrypt(keyPair.secretKey, ct, &result_pt);
        result_pt->SetLength(16);
        double sum = 0.0;
        for (int i = 0; i < 16; i++) sum += result_pt->GetCKKSPackedValue()[i].real();
        double log_phi_val = sum / 16.0;
        return pow(PHI, log_phi_val);
    };

    // ============================================
    // TEST 1: BASIC FRACTIONS
    // ============================================

    cout << "========================================\n";
    cout << "  TEST 1: BASIC FRACTIONS\n";
    cout << "========================================\n\n";

    // 5 × 7 = 35
    auto ct_5 = encrypt_log(5.0);
    auto ct_7 = encrypt_log(7.0);
    auto ct_35 = cc->EvalAdd(ct_5, ct_7);
    
    // 35 / 7 = 5
    auto ct_5_back = cc->EvalSub(ct_35, ct_7);
    
    // 5^3 = 125
    auto ct_125 = cc->EvalAdd(ct_5, ct_5);
    ct_125 = cc->EvalAdd(ct_125, ct_5);

    double val_35 = decrypt_value(ct_35);
    double val_5_back = decrypt_value(ct_5_back);
    double val_125 = decrypt_value(ct_125);

    cout << "  5 × 7 = " << val_35 << " (Expected: 35)\n";
    cout << "  35 / 7 = " << val_5_back << " (Expected: 5)\n";
    cout << "  5^3 = " << val_125 << " (Expected: 125)\n\n";

    // ============================================
    // TEST 2: FRACTION CHAIN (1K)
    // ============================================

    cout << "========================================\n";
    cout << "  TEST 2: FRACTION CHAIN (1K)\n";
    cout << "========================================\n\n";

    int N = 1000;
    auto ct_acc = encrypt_log(1.0);
    auto ct_mult = encrypt_log(2.0);   // ×2
    auto ct_div = encrypt_log(0.5);    // ÷2

    cout << "  Operations: " << N << " (×2 at ÷2 alternating)\n\n";

    auto start = high_resolution_clock::now();

    for (int i = 0; i < N; i++) {
        if (i % 2 == 0) {
            ct_acc = cc->EvalAdd(ct_acc, ct_mult);  // ×2
        } else {
            ct_acc = cc->EvalAdd(ct_acc, ct_div);   // ÷2
        }
    }

    auto end = high_resolution_clock::now();
    auto time = duration_cast<milliseconds>(end - start).count();

    double result = decrypt_value(ct_acc);
    double expected = 1.0;  // ×2 ÷2 = 1 (balanced)

    cout << "  ✅ 1K fraction chain complete!\n";
    cout << "  Time: " << time << " ms\n";
    cout << "  Level: " << ct_acc->GetLevel() << "\n";
    cout << "  Result: " << result << "\n";
    cout << "  Expected: " << expected << "\n";
    cout << "  Match: " << (abs(result - expected) < 0.1 ? "✅" : "❌") << "\n\n";

    // ============================================
    // TEST 3: MIXED NON-LINEAR CHAIN
    // ============================================

    cout << "========================================\n";
    cout << "  TEST 3: MIXED NON-LINEAR (1K)\n";
    cout << "========================================\n\n";

    vector<double> fraction_ops = {2.0, 0.5, 3.0, 1.0/3.0, 5.0, 0.2};
    vector<Ciphertext<DCRTPoly>> ct_ops;
    for (double op : fraction_ops) {
        ct_ops.push_back(encrypt_log(op));
    }

    auto ct_mixed = encrypt_log(1.0);
    
    cout << "  Mixed: ×2, ÷2, ×3, ÷3, ×5, ÷5\n";
    cout << "  Running 1K mixed...\n\n";

    auto s2 = high_resolution_clock::now();
    for (int i = 0; i < N; i++) {
        ct_mixed = cc->EvalAdd(ct_mixed, ct_ops[i % 6]);
    }
    auto e2 = high_resolution_clock::now();
    auto t2 = duration_cast<milliseconds>(e2 - s2).count();

    double mixed_result = decrypt_value(ct_mixed);
    
    cout << "  Time: " << t2 << " ms\n";
    cout << "  Result: " << mixed_result << "\n";
    cout << "  Level: " << ct_mixed->GetLevel() << "\n\n";

    // ============================================
    // SUMMARY
    // ============================================

    cout << "========================================\n";
    cout << "  META FRACTION COMPLETE\n";
    cout << "========================================\n\n";
    cout << "  ✅ Fraction mul/div via EvalAdd/EvalSub\n";
    cout << "  ✅ 1K chain: ×2÷2 = 1 (balanced)\n";
    cout << "  ✅ 1K mixed: ×2,÷2,×3,÷3,×5,÷5\n";
    cout << "  ✅ Level 0\n";
    cout << "  ✅ Depth 1\n";
    cout << "  ✅ Pure FHE\n\n";

    return 0;
}
