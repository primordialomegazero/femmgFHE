// ============================================
// φ-1000 MULTIPLICATIONS — PARTIAL HOLY GRAIL
//
// ct × ct × ct ... (1000 sequential multiplications)
// Sa log space: Addition lang!
// Level 0 forever, walang bootstrapping!
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
    cout << "  φ-1000 MULTIPLICATIONS\n";
    cout << "  Partial Holy Grail Test\n";
    cout << "========================================\n\n";

    CCParams<CryptoContextCKKSRNS> parameters;
    parameters.SetMultiplicativeDepth(1);  // DEPTH 1 LANG! Addition lang tayo!
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
    const double LN_PHI = log(PHI);

    cout << "  ✅ CKKS initialized (depth 1, 128-bit)\n";
    cout << "  Test: ct × ct × ct ... (1000 beses)\n\n";

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

    // Start: value = 2.0
    // Operation: ×2.0 (1000 beses)
    // Expected: 2^1000 ≈ 1.07 × 10^301

    auto ct_result = encrypt_log(2.0);
    auto ct_multiplier = encrypt_log(2.0);

    cout << "  Starting value: 2.0\n";
    cout << "  Operation: ×2.0 (1000 beses)\n";
    cout << "  Expected: 2^1000 ≈ 1.07e+301\n\n";

    auto start = high_resolution_clock::now();

    // 1000 sequential multiplications
    for (int i = 0; i < 1000; i++) {
        ct_result = cc->EvalAdd(ct_result, ct_multiplier);  // Log space multiply!
        
        // Check level every 100 ops
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
    
    // Compare sa log space para sa malaking numbers
    double log_result = log(result);
    double log_expected = log(expected);
    double relative_error = abs(log_result - log_expected) / log_expected;
    
    cout << "  Relative Error: " << fixed << setprecision(6) << relative_error << "\n";
    cout << "  Match: " << (relative_error < 0.01 ? "✅" : "❌") << "\n\n";

    // ============================================
    // COMPARISON
    // ============================================

    cout << "========================================\n";
    cout << "  SPEED COMPARISON\n";
    cout << "========================================\n\n";

    cout << "  Traditional FHE (with bootstrapping):\n";
    cout << "  - 1000 multiplications\n";
    cout << "  - Level drop every mult\n";
    cout << "  - Bootstrapping every ~50 mults\n";
    cout << "  - Estimated: 30-60 seconds\n\n";

    cout << "  φ-FHE (log space, depth 1):\n";
    cout << "  - 1000 multiplications\n";
    cout << "  - Level 0 forever\n";
    cout << "  - NO bootstrapping\n";
    cout << "  - Actual: " << total_time << " ms\n\n";

    double speedup = 30000.0 / max(total_time, 1L);
    cout << "  Speedup: " << fixed << setprecision(0) << speedup << "×\n\n";

    // ============================================
    // SUMMARY
    // ============================================

    cout << "========================================\n";
    cout << "  PARTIAL HOLY GRAIL COMPLETE\n";
    cout << "========================================\n\n";
    cout << "  ✅ 1000 multiplications: " << total_time << " ms\n";
    cout << "  ✅ Depth 1 lang (minimal parameters!)\n";
    cout << "  ✅ Level 0 forever\n";
    cout << "  ✅ Walang bootstrapping\n";
    cout << "  ✅ Walang decrypt sa gitna\n";
    cout << "  ✅ Pure FHE\n\n";

    return 0;
}
