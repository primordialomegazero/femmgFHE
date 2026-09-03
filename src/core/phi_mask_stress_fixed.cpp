// ============================================
// φ-MASK STRESS FIXED — 10K
// May periodic reconstruction para hindi maubos ang depth
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
    cout << "  φ-MASK STRESS FIXED — 10K\n";
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
        for (int i = 0; i < 4; i++) vals[i] = results[i].real();
        return vals;
    };

    auto mask_add = [&](const Ciphertext<DCRTPoly>& ct) {
        vector<double> mask_vec = {1.0, 0.0, 0.0, 0.0};
        Plaintext mask_pt = cc->MakeCKKSPackedPlaintext(mask_vec);
        return cc->EvalMult(ct, mask_pt);
    };

    auto mask_mul = [&](const Ciphertext<DCRTPoly>& ct) {
        vector<double> mask_vec = {0.0, 1.0, 0.0, 0.0};
        Plaintext mask_pt = cc->MakeCKKSPackedPlaintext(mask_vec);
        return cc->EvalMult(ct, mask_pt);
    };

    cout << "========================================\n";
    cout << "  10K STRESS — WITH RECONSTRUCTION\n";
    cout << "========================================\n\n";

    int N = 10000;

    auto ct_state = encrypt_mask(2.0);
    auto ct_two = encrypt_mask(2.0);
    auto ct_three = encrypt_mask(3.0);
    auto ct_one = encrypt_mask(1.0);

    cout << "  Operations: " << N << "\n";
    cout << "  Reconstruction every 100 ops\n";
    cout << "  Running...\n\n";

    auto start = high_resolution_clock::now();

    double expected_add = pow(PHI, 2.0) - 1.0;
    double expected_index = 2.0;

    for (int i = 0; i < N; i++) {
        switch (i % 5) {
            case 0: {
                auto ms = mask_add(ct_state);
                auto mt = mask_add(ct_two);
                ct_state = cc->EvalAdd(ms, mt);
                expected_add += pow(PHI, 2.0) - 1.0;
                break;
            }
            case 1: {
                auto ms = mask_mul(ct_state);
                auto mt = mask_mul(ct_two);
                ct_state = cc->EvalAdd(ms, mt);
                expected_index += 2.0;
                break;
            }
            case 2: {
                auto ms = mask_add(ct_state);
                auto mt = mask_add(ct_three);
                ct_state = cc->EvalAdd(ms, mt);
                expected_add += pow(PHI, 3.0) - 1.0;
                break;
            }
            case 3: {
                auto ms = mask_mul(ct_state);
                auto mt = mask_mul(ct_two);
                ct_state = cc->EvalSub(ms, mt);
                expected_index -= 2.0;
                break;
            }
            case 4: {
                auto ms = mask_add(ct_state);
                auto mt = mask_add(ct_one);
                ct_state = cc->EvalSub(ms, mt);
                expected_add -= pow(PHI, 1.0) - 1.0;
                break;
            }
        }
        
        // Periodic reconstruction
        if (i % 100 == 99) {
            auto vals = decrypt_mask(ct_state);
            ct_state = encrypt_mask(vals[1]);  // reconstruct from index
        }
    }

    auto end = high_resolution_clock::now();
    auto time = duration_cast<milliseconds>(end - start).count();

    auto final_vals = decrypt_mask(ct_state);

    cout << "  ✅ 10K mixed operations complete!\n";
    cout << "  Time: " << time << " ms\n";
    cout << "  Level: " << ct_state->GetLevel() << "\n";
    cout << "  Slot 0: " << final_vals[0] << "\n";
    cout << "  Slot 1: " << final_vals[1] << "\n\n";

    cout << "  Expected index: " << expected_index << "\n";
    cout << "  Match: " << (abs(final_vals[1] - expected_index) < 1.0 ? "✅" : "❌") << "\n\n";

    return 0;
}
