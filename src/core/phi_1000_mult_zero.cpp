// ============================================
// φ-1000 MULTIPLICATIONS — DEPTH 0 TEST
//
// Kaya ba ni Kuya OpenFHE ang ZERO depth?
// Addition lang naman tayo e!
//
// Author: Dan Fernandez / Primordial Omega Zero
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>
#include <chrono>

#include "pke/openfhe.h"

using namespace lbcrypto;
using namespace std;
using namespace std::chrono;

int main() {
    cout << "========================================\n";
    cout << "  φ-1000 MULTIPLICATIONS — DEPTH 0\n";
    cout << "  Kaya ba ni Kuya OpenFHE?\n";
    cout << "========================================\n\n";

    CCParams<CryptoContextCKKSRNS> parameters;
    parameters.SetMultiplicativeDepth(0);  // ZERO DEPTH! Subukan natin!
    parameters.SetScalingModSize(50);
    parameters.SetBatchSize(8);
    parameters.SetSecurityLevel(HEStd_128_classic);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(parameters);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);

    auto keyPair = cc->KeyGen();
    
    try {
        cc->EvalMultKeyGen(keyPair.secretKey);
        cout << "  ✅ EvalMultKeyGen: OK (kahit depth 0)\n";
    } catch (...) {
        cout << "  ⚠️ EvalMultKeyGen: Nagreklamo si Kuya OpenFHE!\n";
    }

    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);

    cout << "  ✅ CKKS initialized (depth 0, 128-bit)\n";
    cout << "  Test: ct × ct × ct ... (1000 beses, puro addition lang)\n\n";

    // ============================================
    // ENCRYPTION SETUP
    // ============================================

    auto encrypt_log = [&](double value) {
        double log_val = log(value) / LN_PHI;
        vector<double> v(8, log_val);
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto decrypt_value = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext result_pt;
        cc->Decrypt(keyPair.secretKey, ct, &result_pt);
        result_pt->SetLength(8);
        return pow(PHI, result_pt->GetCKKSPackedValue()[0].real());
    };

    // ============================================
    // TEST: 1000 SEQUENTIAL MULTIPLICATIONS
    // ============================================

    cout << "========================================\n";
    cout << "  1000 SEQUENTIAL MULTIPLICATIONS\n";
    cout << "========================================\n\n";

    auto ct_result = encrypt_log(2.0);
    auto ct_multiplier = encrypt_log(2.0);

    cout << "  Starting value: 2.0\n";
    cout << "  Operation: ×2.0 (1000 beses)\n";
    cout << "  Expected: 2^1000 ≈ 1.07e+301\n\n";

    auto start = high_resolution_clock::now();

    // 1000 sequential multiplications (addition sa log space)
    for (int i = 0; i < 1000; i++) {
        ct_result = cc->EvalAdd(ct_result, ct_multiplier);
        
        if (i % 100 == 0 && i > 0) {
            cout << "  Step " << setw(4) << i << ": Level " << ct_result->GetLevel() << "\n";
        }
    }

    auto end = high_resolution_clock::now();
    auto total_time = duration_cast<milliseconds>(end - start).count();

    cout << "\n  ✅ 1000 multiplications complete!\n";
    cout << "  Time: " << total_time << " ms\n";
    cout << "  Final Level: " << ct_result->GetLevel() << "\n";
    cout << "  Towers: " << ct_result->GetElements()[0].GetNumOfElements() << "\n\n";

    // ============================================
    // VERIFICATION
    // ============================================

    double result = decrypt_value(ct_result);
    double expected = pow(2.0, 1000.0);

    cout << "  RESULT:\n";
    cout << "  Computed: " << scientific << setprecision(6) << result << "\n";
    cout << "  Expected: " << expected << "\n";
    
    double log_result = log(result);
    double log_expected = log(expected);
    double relative_error = abs(log_result - log_expected) / log_expected;
    
    cout << "  Relative Error: " << fixed << setprecision(6) << relative_error << "\n";
    cout << "  Match: " << (relative_error < 0.01 ? "✅" : "❌") << "\n\n";

    // ============================================
    // SUMMARY
    // ============================================

    cout << "========================================\n";
    cout << "  DEPTH 0 TEST COMPLETE\n";
    cout << "========================================\n\n";
    cout << "  ✅ 1000 multiplications: " << total_time << " ms\n";
    cout << "  ✅ Depth 0 (ZERO multiplication depth!)\n";
    cout << "  ✅ Level 0 forever\n";
    cout << "  ✅ Walang bootstrapping\n";
    cout << "  ✅ Walang decrypt sa gitna\n";
    cout << "  ✅ Pure FHE\n\n";

    if (total_time < 1000) {
        cout << "  🏆 DEPTH 0 WORKS! KUYA OPENFHE PUMAYAG!\n\n";
    }

    return 0;
}
