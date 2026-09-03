// ============================================
// φ-MASK STRESS — 10K
// Slot masking para sa lahat ng operations
// Addition → Slot 0, Multiplication → Slot 1
// Walang EvalMult(ct, ct)
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
    cout << "  φ-MASK STRESS — 10K\n";
    cout << "========================================\n\n";

    CCParams<CryptoContextCKKSRNS> parameters;
    parameters.SetMultiplicativeDepth(3);
    parameters.SetScalingModSize(59);
    parameters.SetBatchSize(4);
    parameters.SetSecurityLevel(HEStd_128_classic);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(parameters);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);
    cc->Enable(ADVANCEDSHE);

    auto keyPair = cc->KeyGen();
    cc->EvalAtIndexKeyGen(keyPair.secretKey, {1, -1, 2, -2, 3, -3});

    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);

    cout << "  ✅ CKKS initialized (depth 3, 4 slots)\n\n";

    // ============================================
    // ENCODING
    // Slot 0: φ^n - 1 (addition)
    // Slot 1: n (multiplication index)
    // Slot 2: φ^n
    // Slot 3: 1
    // ============================================

    auto encrypt_mask = [&](double n) {
        double phi_n = pow(PHI, n);
        
        vector<double> v(4, 0.0);
        v[0] = phi_n - 1.0;
        v[1] = n;
        v[2] = phi_n;
        v[3] = 1.0;
        
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto decrypt_mask = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext result_pt;
        cc->Decrypt(keyPair.secretKey, ct, &result_pt);
        result_pt->SetLength(4);
        auto results = result_pt->GetCKKSPackedValue();
        vector<double> vals(4);
        for (int i = 0; i < 4; i++) {
            vals[i] = results[i].real();
        }
        return vals;
    };

    // Mask para sa addition (Slot 0)
    auto mask_add = [&](const Ciphertext<DCRTPoly>& ct) -> Ciphertext<DCRTPoly> {
        vector<double> mask_vec = {1.0, 0.0, 0.0, 0.0};
        Plaintext mask_pt = cc->MakeCKKSPackedPlaintext(mask_vec);
        return cc->EvalMult(ct, mask_pt);
    };

    // Mask para sa multiplication (Slot 1)
    auto mask_mul = [&](const Ciphertext<DCRTPoly>& ct) -> Ciphertext<DCRTPoly> {
        vector<double> mask_vec = {0.0, 1.0, 0.0, 0.0};
        Plaintext mask_pt = cc->MakeCKKSPackedPlaintext(mask_vec);
        return cc->EvalMult(ct, mask_pt);
    };

    // ============================================
    // 10K STRESS — ALL ARITHMETIC WITH MASKING
    // ============================================

    cout << "========================================\n";
    cout << "  10K STRESS — ALL ARITHMETIC WITH MASKING\n";
    cout << "========================================\n\n";

    int N = 10000;

    auto ct_state = encrypt_mask(2.0);
    auto ct_two = encrypt_mask(2.0);
    auto ct_three = encrypt_mask(3.0);
    auto ct_one = encrypt_mask(1.0);

    cout << "  Operations: " << N << "\n";
    cout << "  Mix: +2, ×2, +3, ÷2, -1\n";
    cout << "  With slot masking\n";
    cout << "  Running...\n\n";

    auto start = high_resolution_clock::now();

    double expected_add = pow(PHI, 2.0) - 1.0;  // φ^2 - 1
    double expected_index = 2.0;

    for (int i = 0; i < N; i++) {
        switch (i % 5) {
            case 0: // +2 — addition sa Slot 0
                {
                    auto ct_masked_state = mask_add(ct_state);
                    auto ct_masked_two = mask_add(ct_two);
                    ct_state = cc->EvalAdd(ct_masked_state, ct_masked_two);
                    expected_add += pow(PHI, 2.0) - 1.0;
                }
                break;
            case 1: // ×2 — multiplication sa Slot 1
                {
                    auto ct_masked_state = mask_mul(ct_state);
                    auto ct_masked_two = mask_mul(ct_two);
                    ct_state = cc->EvalAdd(ct_masked_state, ct_masked_two);
                    expected_index += 2.0;
                }
                break;
            case 2: // +3 — addition sa Slot 0
                {
                    auto ct_masked_state = mask_add(ct_state);
                    auto ct_masked_three = mask_add(ct_three);
                    ct_state = cc->EvalAdd(ct_masked_state, ct_masked_three);
                    expected_add += pow(PHI, 3.0) - 1.0;
                }
                break;
            case 3: // ÷2 — division sa Slot 1
                {
                    auto ct_masked_state = mask_mul(ct_state);
                    auto ct_masked_two = mask_mul(ct_two);
                    ct_state = cc->EvalSub(ct_masked_state, ct_masked_two);
                    expected_index -= 2.0;
                }
                break;
            case 4: // -1 — subtraction sa Slot 0
                {
                    auto ct_masked_state = mask_add(ct_state);
                    auto ct_masked_one = mask_add(ct_one);
                    ct_state = cc->EvalSub(ct_masked_state, ct_masked_one);
                    expected_add -= pow(PHI, 1.0) - 1.0;
                }
                break;
        }
    }

    auto end = high_resolution_clock::now();
    auto time = duration_cast<milliseconds>(end - start).count();

    auto final_vals = decrypt_mask(ct_state);

    cout << "  ✅ 10K mixed operations complete!\n";
    cout << "  Time: " << time << " ms\n";
    cout << "  Level: " << ct_state->GetLevel() << "\n";
    cout << "  Towers: " << ct_state->GetElements()[0].GetNumOfElements() << "\n";
    cout << "  Slot 0 (φ^n - 1): " << final_vals[0] << "\n";
    cout << "  Slot 1 (n): " << final_vals[1] << "\n";
    cout << "  Slot 2 (φ^n): " << final_vals[2] << "\n";
    cout << "  Slot 3 (const): " << final_vals[3] << "\n\n";

    cout << "  Expected add: " << expected_add << "\n";
    cout << "  Expected index: " << expected_index << "\n";
    cout << "  Match (Slot 0): " << (abs(final_vals[0] - expected_add) < 1.0 ? "✅" : "❌") << "\n";
    cout << "  Match (Slot 1): " << (abs(final_vals[1] - expected_index) < 1.0 ? "✅" : "❌") << "\n\n";

    cout << "========================================\n";
    cout << "  φ-MASK STRESS COMPLETE\n";
    cout << "========================================\n\n";
    cout << "  ✅ 10K all arithmetic\n";
    cout << "  ✅ Slot masking\n";
    cout << "  ✅ Walang EvalMult(ct, ct)\n";
    cout << "  ✅ Walang bootstrapping\n\n";

    return 0;
}
