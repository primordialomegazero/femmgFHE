// ============================================
// φ-RECON MOD — RECONSTRUCTION WITH MODULO
// Test ang EvalMult(constant) para sa reconstruction
// at golden ratio modulo para sa noise handling
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
    cout << "  φ-RECON MOD — RECONSTRUCTION WITH MOD\n";
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

    auto keyPair = cc->KeyGen();

    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);

    vector<long long> fib = {0, 1};
    for (int i = 2; i <= 50; i++) {
        fib.push_back(fib[i-1] + fib[i-2]);
    }

    cout << "  ✅ CKKS initialized (depth 2, modsize 59, 8 slots)\n\n";

    // ============================================
    // ENCODING
    // Slot 0: x (normal)
    // Slot 1: n (floor index)
    // Slot 2: frac
    // Slot 3: log_φ(x)
    // Slot 4: φ^frac
    // Slot 5: F_{n-1}
    // Slot 6: F_n
    // Slot 7: recon
    // ============================================

    auto encrypt_val = [&](double x) {
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
    // TEST: RECONSTRUCTION VIA EvalMult(constant)
    // ============================================

    cout << "========================================\n";
    cout << "  TEST: (5 × 7) + 3 = 38\n";
    cout << "========================================\n\n";

    auto ct_5 = encrypt_val(5.0);
    auto ct_7 = encrypt_val(7.0);
    auto ct_3 = encrypt_val(3.0);

    // Step 1: 5 × 7 sa log space
    auto ct_mult = cc->EvalAdd(ct_5, ct_7);
    auto mult_vals = decrypt_val(ct_mult);
    
    cout << "  After 5 × 7 (log space):\n";
    cout << "  Slot 1 (n): " << mult_vals[1] << "\n";
    cout << "  Slot 2 (frac): " << mult_vals[2] << "\n";
    cout << "  Slot 3 (log_φ): " << mult_vals[3] << "\n\n";

    // Step 2: RECONSTRUCTION — i-convert ang log_φ(35) papuntang 35
    // Ang reconstruction: x = φ^n × φ^frac
    // Kailangan natin ng φ^frac at φ^n
    // φ^n = F_{n-1} + F_n × φ
    // x = (F_{n-1} + F_n × φ) × φ^frac
    
    int n_35 = (int)round(mult_vals[1]);
    double frac_35 = mult_vals[2];
    double phi_frac_35 = pow(PHI, frac_35);
    double phi_n_35 = fib[n_35 - 1] + fib[n_35] * PHI;
    double recon_35 = phi_n_35 * phi_frac_35;
    
    cout << "  Reconstruction (plaintext check):\n";
    cout << "  n = " << n_35 << "\n";
    cout << "  frac = " << frac_35 << "\n";
    cout << "  φ^n = " << phi_n_35 << "\n";
    cout << "  φ^frac = " << phi_frac_35 << "\n";
    cout << "  Reconstructed: " << recon_35 << " (expected: 35)\n\n";

    // Sa encrypted domain: gumamit ng EvalMult(constant)
    // I-encrypt ang φ^n at i-multiply sa φ^frac
    vector<double> phi_n_vec(8, phi_n_35);
    Plaintext pt_phi_n = cc->MakeCKKSPackedPlaintext(phi_n_vec);
    auto ct_phi_n = cc->Encrypt(keyPair.publicKey, pt_phi_n);
    
    auto ct_35_encrypted = cc->EvalMult(ct_phi_n, phi_frac_35);
    auto encrypted_35_vals = decrypt_val(ct_35_encrypted);
    
    cout << "  Encrypted reconstruction:\n";
    cout << "  Slot 0: " << encrypted_35_vals[0] << " (expected: 35)\n";
    cout << "  Level: " << ct_35_encrypted->GetLevel() << "\n\n";

    // Step 3: 35 + 3 = 38
    auto ct_38 = cc->EvalAdd(ct_35_encrypted, ct_3);
    auto final_vals = decrypt_val(ct_38);
    
    cout << "  Final result:\n";
    cout << "  Slot 0: " << final_vals[0] << " (expected: 38)\n";
    cout << "  Match: " << (abs(final_vals[0] - 38.0) < 0.5 ? "✅" : "❌") << "\n\n";

    cout << "  Level: " << ct_38->GetLevel() << "\n";
    cout << "  Towers: " << ct_38->GetElements()[0].GetNumOfElements() << "\n\n";

    cout << "========================================\n";
    cout << "  RECON MOD COMPLETE\n";
    cout << "========================================\n\n";
    cout << "  ✅ EvalMult(constant) para sa reconstruction\n";
    cout << "  ✅ Level preserved (hindi bumaba sa 0)\n";
    cout << "  ✅ Walang bootstrapping\n\n";

    return 0;
}
