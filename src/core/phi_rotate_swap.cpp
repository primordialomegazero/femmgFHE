// ============================================
// φ-ROTATE SWAP
// Subok kung ang EvalRotate ay pwedeng
// mag-swap ng slots para sa bridge
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
    cout << "  φ-ROTATE SWAP\n";
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

    cout << "  ✅ CKKS initialized (depth 2, 8 slots)\n\n";

    // ============================================
    // ENCODING
    // Slot 0: x (normal)
    // Slot 1: n (index)
    // Slot 2: frac
    // Slot 3: log_φ(x)
    // Slot 4: φ^frac
    // Slot 5: F_{n-1}
    // Slot 6: F_n
    // Slot 7: recon
    // ============================================

    auto encrypt_rs = [&](double x) {
        double log_phi_x = log(x) / LN_PHI;
        double n_val = floor(log_phi_x);
        double frac = log_phi_x - n_val;
        int n = (int)n_val;
        
        double phi_frac = pow(PHI, frac);
        double F_nm1 = (n > 0) ? round(pow(PHI, n-1) / sqrt(5)) : 0;
        double F_n = round(pow(PHI, n) / sqrt(5));
        double phi_n = F_nm1 + F_n * PHI;
        double recon = phi_n * phi_frac;
        
        vector<double> v(8, 0.0);
        v[0] = x;
        v[1] = n_val;
        v[2] = frac;
        v[3] = log_phi_x;
        v[4] = phi_frac;
        v[5] = F_nm1;
        v[6] = F_n;
        v[7] = recon;
        
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto decrypt_rs = [&](const Ciphertext<DCRTPoly>& ct) {
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
    // TEST: ROTATE PARA SA SWAPPING
    // ============================================

    cout << "========================================\n";
    cout << "  ROTATE PARA SA SWAPPING\n";
    cout << "========================================\n\n";

    auto ct_35 = encrypt_rs(35.0);
    
    cout << "  Original (35):\n";
    auto orig = decrypt_rs(ct_35);
    cout << "  [" << orig[0] << ", " << orig[1] << ", " << orig[2] << ", " << orig[3] 
         << ", " << orig[4] << ", " << orig[5] << ", " << orig[6] << ", " << orig[7] << "]\n\n";

    // Rotate by 4 — i-swap ang unang 4 slots sa huling 4
    auto ct_rot4 = cc->EvalRotate(ct_35, 4);
    auto rot4 = decrypt_rs(ct_rot4);
    
    cout << "  After EvalRotate(4):\n";
    cout << "  [" << rot4[0] << ", " << rot4[1] << ", " << rot4[2] << ", " << rot4[3] 
         << ", " << rot4[4] << ", " << rot4[5] << ", " << rot4[6] << ", " << rot4[7] << "]\n\n";

    // Rotate by -1 — i-shift pababa
    auto ct_rot_neg1 = cc->EvalRotate(ct_35, -1);
    auto rot_neg1 = decrypt_rs(ct_rot_neg1);
    
    cout << "  After EvalRotate(-1):\n";
    cout << "  [" << rot_neg1[0] << ", " << rot_neg1[1] << ", " << rot_neg1[2] << ", " << rot_neg1[3] 
         << ", " << rot_neg1[4] << ", " << rot_neg1[5] << ", " << rot_neg1[6] << ", " << rot_neg1[7] << "]\n\n";

    // ============================================
    // TEST: ROTATE + ADD PARA SA BRIDGE
    // ============================================

    cout << "========================================\n";
    cout << "  ROTATE + ADD PARA SA BRIDGE\n";
    cout << "========================================\n\n";

    auto ct_5 = encrypt_rs(5.0);
    auto ct_7 = encrypt_rs(7.0);

    // Subok: i-rotate ang ct_7 para i-align ang slots
    auto ct_7_rot = cc->EvalRotate(ct_7, 1);
    auto ct_sum = cc->EvalAdd(ct_5, ct_7_rot);
    auto sum_vals = decrypt_rs(ct_sum);

    cout << "  5 + rotate(7,1):\n";
    cout << "  [" << sum_vals[0] << ", " << sum_vals[1] << ", " << sum_vals[2] << ", " << sum_vals[3] 
         << ", " << sum_vals[4] << ", " << sum_vals[5] << ", " << sum_vals[6] << ", " << sum_vals[7] << "]\n\n";

    cout << "  Level: " << ct_sum->GetLevel() << "\n";
    cout << "  Towers: " << ct_sum->GetElements()[0].GetNumOfElements() << "\n\n";

    return 0;
}
