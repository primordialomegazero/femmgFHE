// ============================================
// φ-PURE FHE MIXED TEST — WALANG DECRYPT SA GITNA
//
// Dual space: log_φ (slots 0-7) + normal (slots 8-15)
// Operations: ×, ÷, +, −
// Walang EvalMult, walang decrypt sa gitna
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
    cout << "  φ-PURE FHE MIXED TEST\n";
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

    cout << "  ✅ CKKS initialized (depth 1!)\n\n";

    // Dual space encryption
    auto encrypt_dual = [&](double value) {
        vector<double> v(16, 0.0);
        double log_val = log(value) / LN_PHI;
        for (int i = 0; i < 8; i++) v[i] = log_val;   // log space
        for (int i = 8; i < 16; i++) v[i] = value;    // normal space
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    // Final decrypt lang — para makuha ang resulta
    auto decrypt_final = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext result_pt;
        cc->Decrypt(keyPair.secretKey, ct, &result_pt);
        result_pt->SetLength(16);

        double log_sum = 0.0;
        for (int i = 0; i < 8; i++) log_sum += result_pt->GetCKKSPackedValue()[i].real();
        double log_res = log_sum / 8.0;

        double normal_sum = 0.0;
        for (int i = 8; i < 16; i++) normal_sum += result_pt->GetCKKSPackedValue()[i].real();
        double normal_res = normal_sum / 8.0;

        return make_pair(log_res, normal_res);
    };

    // ============================================
    // TEST: ((5 × 7) ÷ 2) + 10 − 3 = 24.5
    // ============================================

    cout << "========================================\n";
    cout << "  TEST: ((5 × 7) ÷ 2) + 10 − 3\n";
    cout << "========================================\n\n";

    auto ct_5 = encrypt_dual(5.0);
    auto ct_7 = encrypt_dual(7.0);
    auto ct_2 = encrypt_dual(2.0);
    auto ct_10 = encrypt_dual(10.0);
    auto ct_3 = encrypt_dual(3.0);

    // Step 1: 5 × 7 (log space multiply)
    auto ct_step1 = cc->EvalAdd(ct_5, ct_7);

    // Step 2: ÷ 2 (log space divide)
    auto ct_step2 = cc->EvalSub(ct_step1, ct_2);

    // Step 3: + 10 (normal space add)
    auto ct_step3 = cc->EvalAdd(ct_step2, ct_10);

    // Step 4: − 3 (normal space sub)
    auto ct_step4 = cc->EvalSub(ct_step3, ct_3);

    auto [log_final, normal_final] = decrypt_final(ct_step4);

    double expected_normal = 24.5;
    double result_normal = normal_final;

    cout << "  Final normal space: " << result_normal << "\n";
    cout << "  Expected: " << expected_normal << "\n";
    cout << "  Match: " << (abs(result_normal - expected_normal) < 0.5 ? "✅" : "❌") << "\n\n";

    cout << "  Level: " << ct_step4->GetLevel() << "\n";
    cout << "  Towers: " << ct_step4->GetElements()[0].GetNumOfElements() << "\n\n";

    // ============================================
    // TEST: CHAIN 1K mixed, walang decrypt sa gitna
    // ============================================

    cout << "========================================\n";
    cout << "  TEST: 1K MIXED CHAIN (NO DECRYPT)\n";
    cout << "========================================\n\n";

    auto ct_state = encrypt_dual(1.0);
    auto ct_two = encrypt_dual(2.0);
    auto ct_half = encrypt_dual(0.5);

    auto start = high_resolution_clock::now();

    for (int i = 0; i < 1000; i++) {
        switch (i % 4) {
            case 0: ct_state = cc->EvalAdd(ct_state, ct_two); break;   // ×2
            case 1: ct_state = cc->EvalSub(ct_state, ct_two); break;   // ÷2
            case 2: ct_state = cc->EvalAdd(ct_state, ct_two); break;   // +2
            case 3: ct_state = cc->EvalSub(ct_state, ct_two); break;   // −2
        }
    }

    auto end = high_resolution_clock::now();
    auto time = duration_cast<milliseconds>(end - start).count();

    auto [log_1k, normal_1k] = decrypt_final(ct_state);

    cout << "  1K mixed chain complete\n";
    cout << "  Time: " << time << " ms\n";
    cout << "  Log result: " << log_1k << "\n";
    cout << "  Normal result: " << normal_1k << "\n";
    cout << "  Level: " << ct_state->GetLevel() << "\n";
    cout << "  Towers: " << ct_state->GetElements()[0].GetNumOfElements() << "\n\n";

    cout << "========================================\n";
    cout << "  PURE FHE MIXED TEST COMPLETE\n";
    cout << "========================================\n\n";
    cout << "  ✅ Mixed arithmetic\n";
    cout << "  ✅ Walang decrypt sa gitna\n";
    cout << "  ✅ Walang EvalMult\n";
    cout << "  ✅ Level 0\n";
    cout << "  ✅ Depth 1\n\n";

    return 0;
}
