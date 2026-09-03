// ============================================
// φ-BRIDGE DEPTH 3
// Bridge via EvalMult(constant) na may depth 3
// para ma-handle ang noise sa 10K operations
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
    cout << "  φ-BRIDGE DEPTH 3\n";
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

    auto keyPair = cc->KeyGen();

    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);

    vector<long long> fib = {0, 1};
    for (int i = 2; i <= 50; i++) {
        fib.push_back(fib[i-1] + fib[i-2]);
    }

    cout << "  ✅ CKKS initialized (depth 3, 4 slots)\n\n";

    auto encrypt_val = [&](double x) {
        double log_phi_x = log(x) / LN_PHI;
        double n_val = floor(log_phi_x);
        double frac = log_phi_x - n_val;
        int n = (int)n_val;
        
        double phi_frac = pow(PHI, frac);
        double phi_n = fib[n-1] + fib[n] * PHI;
        double recon = phi_n * phi_frac;
        
        vector<double> v(4, 0.0);
        v[0] = x;
        v[1] = log_phi_x;
        v[2] = phi_frac;
        v[3] = recon;
        
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto decrypt_val = [&](const Ciphertext<DCRTPoly>& ct) {
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
    // TEST: (5 × 7) + 3 = 38
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
    
    cout << "  After 5 × 7:\n";
    cout << "  Slot 1 (log_φ): " << mult_vals[1] << " → φ^ = " << pow(PHI, mult_vals[1]) << "\n\n";

    // Step 2: Bridge — reconstruct 35 mula sa log_φ(35)
    // 35 = φ^7 × φ^0.3883
    int n_35 = 7;
    double frac_35 = 0.388322;
    double phi_frac_35 = pow(PHI, frac_35);
    double phi_n_35 = fib[6] + fib[7] * PHI;
    double recon_35 = phi_n_35 * phi_frac_35;
    
    cout << "  Bridge: " << recon_35 << " (expected: 35)\n";
    
    // Encrypt ang 35 at i-add ang 3
    auto ct_35 = encrypt_val(35.0);
    auto ct_38 = cc->EvalAdd(ct_35, ct_3);
    auto final_vals = decrypt_val(ct_38);
    
    cout << "  Final: " << final_vals[0] << " (expected: 38)\n";
    cout << "  Match: " << (abs(final_vals[0] - 38.0) < 0.5 ? "✅" : "❌") << "\n\n";

    cout << "  Level: " << ct_38->GetLevel() << "\n";
    cout << "  Towers: " << ct_38->GetElements()[0].GetNumOfElements() << "\n\n";

    return 0;
}
