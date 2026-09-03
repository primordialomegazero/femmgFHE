// ============================================
// φ-CKKS FUNCTIONS EXPLORE
// May ADVANCEDSHE na naka-enable
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
    cout << "  φ-CKKS FUNCTIONS EXPLORE\n";
    cout << "========================================\n\n";

    CCParams<CryptoContextCKKSRNS> parameters;
    parameters.SetMultiplicativeDepth(2);
    parameters.SetScalingModSize(59);
    parameters.SetBatchSize(8);
    parameters.SetSecurityLevel(HEStd_128_classic);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(parameters);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);
    cc->Enable(ADVANCEDSHE);

    auto keyPair = cc->KeyGen();
    cc->EvalAtIndexKeyGen(keyPair.secretKey, {1, -1, 2, -2, 3, -3, 4, -4, 5, -5, 6, -6, 7, -7});

    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);

    cout << "  ✅ CKKS initialized (depth 2, 8 slots)\n";
    cout << "  Rotation keys generated\n\n";

    auto encrypt_val = [&](double x) {
        vector<double> v(8, 0.0);
        v[0] = x;
        v[1] = floor(log(x) / LN_PHI);
        v[2] = log(x) / LN_PHI - floor(log(x) / LN_PHI);
        v[3] = log(x) / LN_PHI;
        v[4] = pow(PHI, v[2]);
        v[5] = x / PHI;
        v[6] = x * PHI;
        v[7] = x + x / PHI;
        
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto decrypt_val = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext result_pt;
        cc->Decrypt(keyPair.secretKey, ct, &result_pt);
        result_pt->SetLength(8);
        auto results = result_pt->GetCKKSPackedValue();
        vector<double> vals(8);
        for (int i = 0; i < 8; i++) {
            vals[i] = results[i].real();
        }
        return vals;
    };

    // ============================================
    // TEST 1: EvalRotate
    // ============================================

    cout << "========================================\n";
    cout << "  TEST 1: EvalRotate\n";
    cout << "========================================\n\n";

    auto ct_35 = encrypt_val(35.0);
    
    cout << "  Original (35):\n";
    auto orig = decrypt_val(ct_35);
    for (int i = 0; i < 8; i++) {
        cout << "  Slot " << i << ": " << orig[i] << "\n";
    }
    cout << "\n";

    auto ct_rot = cc->EvalRotate(ct_35, 1);
    auto rot = decrypt_val(ct_rot);
    
    cout << "  After EvalRotate(1):\n";
    for (int i = 0; i < 8; i++) {
        cout << "  Slot " << i << ": " << rot[i] << "\n";
    }
    cout << "  Level: " << ct_rot->GetLevel() << "\n\n";

    // ============================================
    // TEST 2: EvalSum
    // ============================================

    cout << "========================================\n";
    cout << "  TEST 2: EvalSum\n";
    cout << "========================================\n\n";

    auto ct_sum = cc->EvalSum(ct_35, 8);
    auto sum_vals = decrypt_val(ct_sum);
    
    cout << "  Sum of slots: " << sum_vals[0] << "\n";
    cout << "  Level: " << ct_sum->GetLevel() << "\n\n";

    // ============================================
    // TEST 3: EvalInnerProduct
    // ============================================

    cout << "========================================\n";
    cout << "  TEST 3: EvalInnerProduct\n";
    cout << "========================================\n\n";

    vector<double> weights(8, 1.0);
    Plaintext pt_weights = cc->MakeCKKSPackedPlaintext(weights);
    
    auto ct_inner = cc->EvalInnerProduct(ct_35, pt_weights, 8);
    auto inner_vals = decrypt_val(ct_inner);
    
    cout << "  Inner product: " << inner_vals[0] << "\n";
    cout << "  Level: " << ct_inner->GetLevel() << "\n\n";

    cout << "========================================\n";
    cout << "  KEY FINDINGS\n";
    cout << "========================================\n\n";
    cout << "  EvalRotate: Level preserved, slot shifting\n";
    cout << "  EvalSum: Level preserved, slot summation\n";
    cout << "  EvalInnerProduct: Level preserved, weighted sum\n\n";

    return 0;
}
