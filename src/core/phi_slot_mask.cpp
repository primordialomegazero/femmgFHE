// ============================================
// φ-SLOT MASK
// Ihiwalay ang slots para sa addition at multiplication
// gamit ang masking technique
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
    cout << "  φ-SLOT MASK\n";
    cout << "========================================\n\n";

    CCParams<CryptoContextCKKSRNS> parameters;
    parameters.SetMultiplicativeDepth(2);
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

    cout << "  ✅ CKKS initialized (depth 2, 4 slots)\n\n";

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

    // ============================================
    // MASKING FUNCTION
    // ============================================

    // Mask para sa addition (Slot 0 lang)
    auto mask_add = [&](const Ciphertext<DCRTPoly>& ct) -> Ciphertext<DCRTPoly> {
        vector<double> mask_vec = {1.0, 0.0, 0.0, 0.0};
        Plaintext mask_pt = cc->MakeCKKSPackedPlaintext(mask_vec);
        auto ct_masked = cc->EvalMult(ct, mask_pt);
        return ct_masked;
    };

    // Mask para sa multiplication (Slot 1 lang)
    auto mask_mul = [&](const Ciphertext<DCRTPoly>& ct) -> Ciphertext<DCRTPoly> {
        vector<double> mask_vec = {0.0, 1.0, 0.0, 0.0};
        Plaintext mask_pt = cc->MakeCKKSPackedPlaintext(mask_vec);
        auto ct_masked = cc->EvalMult(ct, mask_pt);
        return ct_masked;
    };

    // ============================================
    // TEST: ADDITION SA SLOT 0
    // ============================================

    cout << "========================================\n";
    cout << "  ADDITION SA SLOT 0 (with mask)\n";
    cout << "========================================\n\n";

    auto ct_2 = encrypt_mask(2.0);
    auto ct_3 = encrypt_mask(3.0);

    // Mask para sa addition
    auto ct_2_add = mask_add(ct_2);
    auto ct_3_add = mask_add(ct_3);
    
    auto ct_add = cc->EvalAdd(ct_2_add, ct_3_add);
    auto add_vals = decrypt_mask(ct_add);
    
    cout << "  Slot 0: " << add_vals[0] << " (expected: φ^2+φ^3-2 = " 
         << (pow(PHI,2)+pow(PHI,3)-2) << ")\n";
    cout << "  Slot 1: " << add_vals[1] << " (dapat 0 — masked)\n\n";

    // ============================================
    // TEST: MULTIPLICATION SA SLOT 1
    // ============================================

    cout << "========================================\n";
    cout << "  MULTIPLICATION SA SLOT 1 (with mask)\n";
    cout << "========================================\n\n";

    auto ct_2_mul = mask_mul(ct_2);
    auto ct_3_mul = mask_mul(ct_3);
    
    auto ct_mul = cc->EvalAdd(ct_2_mul, ct_3_mul);
    auto mul_vals = decrypt_mask(ct_mul);
    
    cout << "  Slot 0: " << mul_vals[0] << " (dapat 0 — masked)\n";
    cout << "  Slot 1: " << mul_vals[1] << " (expected: 5)\n\n";

    cout << "  Level: " << ct_add->GetLevel() << "\n";
    cout << "  Towers: " << ct_add->GetElements()[0].GetNumOfElements() << "\n\n";

    return 0;
}
