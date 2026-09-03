// ============================================
// φ-INNER PRODUCT RECON
// Gumamit ng EvalInnerProduct para sa
// direct reconstruction mula sa slots
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
    cout << "  φ-INNER PRODUCT RECON\n";
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

    vector<long long> fib = {0, 1};
    for (int i = 2; i <= 50; i++) {
        fib.push_back(fib[i-1] + fib[i-2]);
    }

    cout << "  ✅ CKKS initialized (depth 2, 8 slots)\n\n";

    // ============================================
    // ENCODING
    // ============================================

    auto encrypt_hg = [&](double x) {
        double log_phi_x = log(x) / LN_PHI;
        double n_val = floor(log_phi_x);
        double frac = log_phi_x - n_val;
        int n = (int)n_val;
        
        double phi_frac = pow(PHI, frac);
        double F_nm1 = fib[n-1];
        double F_n = fib[n];
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

    auto decrypt_hg = [&](const Ciphertext<DCRTPoly>& ct) {
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
    // TEST: (5 × 7) + 3 = 38 GAMIT ANG INNER PRODUCT
    // ============================================

    cout << "========================================\n";
    cout << "  TEST: (5 × 7) + 3 = 38\n";
    cout << "========================================\n\n";

    auto ct_5 = encrypt_hg(5.0);
    auto ct_7 = encrypt_hg(7.0);
    auto ct_3 = encrypt_hg(3.0);

    // Step 1: 5 × 7
    auto ct_mult = cc->EvalAdd(ct_5, ct_7);
    auto mult_vals = decrypt_hg(ct_mult);
    
    cout << "  After 5 × 7:\n";
    cout << "  Slot 1 (n): " << mult_vals[1] << "\n";
    cout << "  Slot 2 (frac): " << mult_vals[2] << "\n";
    cout << "  Slot 3 (log_φ): " << mult_vals[3] << "\n";
    cout << "  Slot 4 (φ^frac): " << mult_vals[4] << "\n";
    cout << "  Slot 5 (F_{n-1}): " << mult_vals[5] << "\n";
    cout << "  Slot 6 (F_n): " << mult_vals[6] << "\n";
    cout << "  Slot 7 (recon): " << mult_vals[7] << "\n\n";

    // Step 2: RECONSTRUCTION VIA INNER PRODUCT
    // Ang reconstruction: 35 = F_{n-1}×φ^frac + F_n×φ×φ^frac
    // = Slot 5 × Slot 4 + Slot 6 × φ × Slot 4
    // = Slot 4 × (Slot 5 + Slot 6 × φ)
    
    // Gumamit ng EvalInnerProduct na may weights
    // weights = [0, 0, 0, 0, 0, 1, φ, 0]
    // Kasi: recon = 0×x + 0×n + 0×frac + 0×log + 0×φ^frac + 1×F_{n-1} + φ×F_n + 0×recon
    // = F_{n-1} + φ×F_n
    
    vector<double> weights = {0, 0, 0, 0, 0, 1.0, PHI, 0};
    Plaintext pt_weights = cc->MakeCKKSPackedPlaintext(weights);
    
    auto ct_inner = cc->EvalInnerProduct(ct_mult, pt_weights, 8);
    auto inner_vals = decrypt_hg(ct_inner);
    
    cout << "  Inner product (F_{n-1} + φ×F_n): " << inner_vals[0] << "\n";
    cout << "  Expected: φ^n = " << (fib[6] + fib[7] * PHI) << "\n\n";

    // Step 3: I-multiply sa φ^frac para makuha ang 35
    double phi_frac_35 = mult_vals[4];
    auto ct_35 = cc->EvalMult(ct_inner, phi_frac_35);
    auto val_35 = decrypt_hg(ct_35);
    
    cout << "  Reconstructed: " << val_35[0] << " (expected: 35)\n";
    cout << "  Match: " << (abs(val_35[0] - 35.0) < 0.5 ? "✅" : "❌") << "\n\n";

    // Step 4: + 3
    auto ct_38 = cc->EvalAdd(ct_35, ct_3);
    auto final_vals = decrypt_hg(ct_38);
    
    cout << "  Final: " << final_vals[0] << " (expected: 38)\n";
    cout << "  Match: " << (abs(final_vals[0] - 38.0) < 0.5 ? "✅" : "❌") << "\n\n";

    cout << "  Level: " << ct_38->GetLevel() << "\n";
    cout << "  Towers: " << ct_38->GetElements()[0].GetNumOfElements() << "\n\n";

    return 0;
}
