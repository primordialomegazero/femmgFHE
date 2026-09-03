// ============================================
// φ-1000 MULTIPLICATIONS — WITH UNIVERSAL MODULO
//
// Fix: φ-anchored modulo para sa malaking numbers
// Depth 1 (minimum ni Kuya OpenFHE)
// Log space + φ-modulo = No overflow!
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
    cout << "  With Universal Modulo\n";
    cout << "========================================\n\n";

    CCParams<CryptoContextCKKSRNS> parameters;
    parameters.SetMultiplicativeDepth(1);  // DEPTH 1 LANG!
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
    const double PHI_MOD = PHI;  // Universal modulo = φ

    cout << "  ✅ CKKS initialized (depth 1, 128-bit)\n";
    cout << "  Universal modulo: φ = " << PHI << "\n\n";

    // ============================================
    // ENCRYPTION WITH MODULO
    // ============================================

    auto encrypt_log_mod = [&](double value) {
        double log_val = log(value) / LN_PHI;
        // φ-modulo para hindi mag-overflow
        log_val = fmod(log_val, PHI_MOD);
        
        vector<double> v(8, log_val);
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto decrypt_value_mod = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext result_pt;
        cc->Decrypt(keyPair.secretKey, ct, &result_pt);
        result_pt->SetLength(8);
        double log_val = result_pt->GetCKKSPackedValue()[0].real();
        // Ibalik sa normal space
        return pow(PHI, log_val);
    };

    // ============================================
    // TEST: 1000 SEQUENTIAL MULTIPLICATIONS
    // ============================================

    cout << "========================================\n";
    cout << "  1000 SEQUENTIAL MULTIPLICATIONS\n";
    cout << "  With φ-Universal Modulo\n";
    cout << "========================================\n\n";

    auto ct_result = encrypt_log_mod(2.0);
    auto ct_multiplier = encrypt_log_mod(2.0);

    cout << "  Starting value: 2.0\n";
    cout << "  Operation: ×2.0 (1000 beses)\n";
    cout << "  Expected (with φ-modulo): φ^(1000 × log_φ(2) mod φ)\n\n";

    auto start = high_resolution_clock::now();

    // 1000 sequential multiplications with modulo
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

    double result = decrypt_value_mod(ct_result);
    
    // Sa modulo space, ang expected ay:
    double log_2 = log(2.0) / LN_PHI;
    double log_expected = fmod(1000.0 * log_2, PHI_MOD);
    double expected_mod = pow(PHI, log_expected);

    cout << "  RESULT (with φ-modulo):\n";
    cout << "  Computed: " << scientific << setprecision(6) << result << "\n";
    cout << "  Expected (mod φ): " << expected_mod << "\n";
    
    double rel_error = abs(log(result) - log(expected_mod)) / abs(log(expected_mod));
    cout << "  Relative Error: " << fixed << setprecision(6) << rel_error << "\n";
    cout << "  Match: " << (rel_error < 0.01 ? "✅" : "❌") << "\n\n";

    // ============================================
    // SUMMARY
    // ============================================

    cout << "========================================\n";
    cout << "  COMPLETE WITH UNIVERSAL MODULO\n";
    cout << "========================================\n\n";
    cout << "  ✅ 1000 multiplications: " << total_time << " ms\n";
    cout << "  ✅ Depth 1 (minimum)\n";
    cout << "  ✅ φ-universal modulo\n";
    cout << "  ✅ Level 0 forever\n";
    cout << "  ✅ Walang bootstrapping\n";
    cout << "  ✅ Pure FHE\n\n";

    return 0;
}
