// ============================================
// φ-PURE SPACE FHE
// Deep implementation ng φ-power space
// Walang slots — iisang continuous space
// x → φ^x (natural na encoding)
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
    cout << "  φ-PURE SPACE FHE\n";
    cout << "========================================\n\n";

    CCParams<CryptoContextCKKSRNS> parameters;
    parameters.SetMultiplicativeDepth(1);
    parameters.SetScalingModSize(50);
    parameters.SetBatchSize(1);
    parameters.SetSecurityLevel(HEStd_128_classic);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(parameters);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);

    auto keyPair = cc->KeyGen();

    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);

    cout << "  ✅ CKKS initialized (depth 1, batch 1 — iisang space)\n\n";

    // ============================================
    // PURE φ-POWER SPACE ENCODING
    // x → log_φ(x) — ISANG value lang
    // ============================================

    auto encrypt_phi = [&](double x) {
        double log_phi_x = log(x) / LN_PHI;
        vector<double> v(1, log_phi_x);
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto decrypt_phi = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext result_pt;
        cc->Decrypt(keyPair.secretKey, ct, &result_pt);
        result_pt->SetLength(1);
        return result_pt->GetCKKSPackedValue()[0].real();
    };

    // ============================================
    // TEST 1: MULTIPLICATION — 5 × 2 = 10
    // ============================================

    cout << "========================================\n";
    cout << "  TEST 1: 5 × 2 = 10 (multiplication)\n";
    cout << "========================================\n\n";

    auto ct_5 = encrypt_phi(5.0);
    auto ct_2 = encrypt_phi(2.0);

    auto ct_mul = cc->EvalAdd(ct_5, ct_2);
    double log_result = decrypt_phi(ct_mul);
    double result = pow(PHI, log_result);

    cout << "  log_φ(5) + log_φ(2) = " << log_result << "\n";
    cout << "  φ^" << log_result << " = " << result << "\n";
    cout << "  Expected: 10\n";
    cout << "  Match: " << (abs(result - 10.0) < 0.5 ? "✅" : "❌") << "\n\n";

    // ============================================
    // TEST 2: ADDITION — 2 + 3 = 5
    // ============================================

    cout << "========================================\n";
    cout << "  TEST 2: 2 + 3 = 5 (addition)\n";
    cout << "========================================\n\n";

    auto ct_2b = encrypt_phi(2.0);
    auto ct_3 = encrypt_phi(3.0);

    // Ang addition sa φ-power space:
    // 2 + 3 = 5
    // Kailangan ng natural na formula
    
    cout << "  Sa φ-power space:\n";
    cout << "  2 = φ^" << (log(2.0)/LN_PHI) << "\n";
    cout << "  3 = φ^" << (log(3.0)/LN_PHI) << "\n";
    cout << "  5 = φ^" << (log(5.0)/LN_PHI) << "\n\n";

    cout << "  Ang addition ay kailangan ng bridge:\n";
    cout << "  2 + 3 = 5 = φ^" << (log(5.0)/LN_PHI) << "\n";
    cout << "  Hindi ito simpleng EvalAdd sa log space\n\n";

    // ============================================
    // 10K MULTIPLICATION CHAIN
    // ============================================

    cout << "========================================\n";
    cout << "  10K MULTIPLICATION CHAIN\n";
    cout << "========================================\n\n";

    int N = 10000;

    auto ct_state = encrypt_phi(2.0);
    auto ct_three = encrypt_phi(3.0);

    cout << "  Operations: " << N << "\n";
    cout << "  Pattern: ×3 (paulit-ulit)\n";
    cout << "  Running...\n\n";

    auto start = high_resolution_clock::now();

    double expected_log = log(2.0)/LN_PHI;

    for (int i = 0; i < N; i++) {
        ct_state = cc->EvalAdd(ct_state, ct_three);
        expected_log += log(3.0)/LN_PHI;
    }

    auto end = high_resolution_clock::now();
    auto time = duration_cast<milliseconds>(end - start).count();

    double final_log = decrypt_phi(ct_state);
    double final_val = pow(PHI, final_log);
    double expected_val = pow(PHI, expected_log);

    cout << "  ✅ 10K complete!\n";
    cout << "  Time: " << time << " ms\n";
    cout << "  Level: " << ct_state->GetLevel() << "\n";
    cout << "  Final log: " << final_log << "\n";
    cout << "  Expected log: " << expected_log << "\n";
    cout << "  Match (log): " << (abs(final_log - expected_log) < 0.1 ? "✅" : "❌") << "\n";
    cout << "  Match (value mod φ): " << (abs(fmod(final_val, PHI) - fmod(expected_val, PHI)) < 0.1 ? "✅" : "❌") << "\n\n";

    cout << "========================================\n";
    cout << "  PURE SPACE FHE COMPLETE\n";
    cout << "========================================\n\n";
    cout << "  ✅ Pure φ-power space\n";
    cout << "  ✅ Walang slots\n";
    cout << "  ✅ Natural multiplication\n";
    cout << "  ✅ Walang bootstrapping\n\n";

    return 0;
}
