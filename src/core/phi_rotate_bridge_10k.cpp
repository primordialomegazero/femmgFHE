// ============================================
// φ-ROTATE BRIDGE — 10K
// Gumamit ng EvalRotate para sa bridge
// sa pagitan ng log at normal space
// Walang decrypt, walang EvalMult(constant)
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
    cout << "  φ-ROTATE BRIDGE — 10K\n";
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

    cout << "  ✅ CKKS initialized (depth 2, 4 slots)\n";
    cout << "  Rotation keys generated\n\n";

    // ============================================
    // ENCODING
    // Slot 0: x (normal)
    // Slot 1: log_φ(x) (log)
    // Slot 2: φ^frac (reconstruction multiplier)
    // Slot 3: n (index)
    // ============================================

    auto encrypt_rot = [&](double x) {
        double log_phi_x = log(x) / LN_PHI;
        double n_val = floor(log_phi_x);
        double frac = log_phi_x - n_val;
        double phi_frac = pow(PHI, frac);
        
        vector<double> v(4, 0.0);
        v[0] = x;
        v[1] = log_phi_x;
        v[2] = phi_frac;
        v[3] = n_val;
        
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto decrypt_rot = [&](const Ciphertext<DCRTPoly>& ct) {
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
    // TEST: ROTATE PARA SA BRIDGE
    // ============================================

    cout << "========================================\n";
    cout << "  ROTATE PARA SA BRIDGE\n";
    cout << "========================================\n\n";

    auto ct_5 = encrypt_rot(5.0);
    auto ct_7 = encrypt_rot(7.0);
    auto ct_3 = encrypt_rot(3.0);

    cout << "  Original 5: ";
    auto v5 = decrypt_rot(ct_5);
    cout << "[" << v5[0] << ", " << v5[1] << ", " << v5[2] << ", " << v5[3] << "]\n";
    
    cout << "  Original 7: ";
    auto v7 = decrypt_rot(ct_7);
    cout << "[" << v7[0] << ", " << v7[1] << ", " << v7[2] << ", " << v7[3] << "]\n\n";

    // Step 1: 5 × 7 sa log space
    auto ct_mult = cc->EvalAdd(ct_5, ct_7);
    auto mult_vals = decrypt_rot(ct_mult);
    
    cout << "  After 5 × 7 (EvalAdd):\n";
    cout << "  [" << mult_vals[0] << ", " << mult_vals[1] << ", " << mult_vals[2] << ", " << mult_vals[3] << "]\n\n";

    // Step 2: BRIDGE via ROTATE
    // I-rotate ang Slot 1 (log) papuntang Slot 0 (normal)
    auto ct_rot = cc->EvalRotate(ct_mult, -1);
    auto rot_vals = decrypt_rot(ct_rot);
    
    cout << "  After EvalRotate(-1):\n";
    cout << "  [" << rot_vals[0] << ", " << rot_vals[1] << ", " << rot_vals[2] << ", " << rot_vals[3] << "]\n\n";

    // Step 3: + 3
    auto ct_final = cc->EvalAdd(ct_rot, ct_3);
    auto final_vals = decrypt_rot(ct_final);
    
    cout << "  After + 3:\n";
    cout << "  [" << final_vals[0] << ", " << final_vals[1] << ", " << final_vals[2] << ", " << final_vals[3] << "]\n\n";

    cout << "  Level: " << ct_final->GetLevel() << "\n";
    cout << "  Towers: " << ct_final->GetElements()[0].GetNumOfElements() << "\n\n";

    return 0;
}
