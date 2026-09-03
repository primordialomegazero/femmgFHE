// ============================================
// φ-EVALMULT CONSTANT
// Subok kung ang EvalMult sa constant ay
// libre at pwedeng gamitin para sa reconstruction
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
    cout << "  φ-EVALMULT CONSTANT\n";
    cout << "========================================\n\n";

    CCParams<CryptoContextCKKSRNS> parameters;
    parameters.SetMultiplicativeDepth(1);
    parameters.SetScalingModSize(50);
    parameters.SetBatchSize(4);
    parameters.SetSecurityLevel(HEStd_128_classic);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(parameters);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);

    auto keyPair = cc->KeyGen();

    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);

    cout << "  ✅ CKKS initialized (depth 1, 4 slots)\n\n";

    // ============================================
    // TEST: EvalMult sa Constant
    // ============================================

    auto encrypt_val = [&](double v) {
        vector<double> vec(4, v);
        Plaintext pt = cc->MakeCKKSPackedPlaintext(vec);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto decrypt_val = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext result_pt;
        cc->Decrypt(keyPair.secretKey, ct, &result_pt);
        result_pt->SetLength(4);
        return result_pt->GetCKKSPackedValue()[0].real();
    };

    cout << "========================================\n";
    cout << "  MULTIPLICATION SA CONSTANT\n";
    cout << "========================================\n\n";

    cout << "  x | x × φ | Level | Match?\n";
    cout << "  --|-------|-------|--------\n";

    for (double x : {5.0, 7.0, 35.0}) {
        auto ct_x = encrypt_val(x);
        auto ct_scaled = cc->EvalMult(ct_x, PHI);
        
        double result = decrypt_val(ct_scaled);
        double expected = x * PHI;
        bool match = abs(result - expected) < 0.1;
        
        cout << "  " << setw(3) << x << " | "
             << setw(6) << fixed << setprecision(3) << result << " | "
             << setw(5) << ct_scaled->GetLevel() << " | "
             << (match ? "✅" : "❌") << "\n";
    }

    cout << "\n========================================\n";
    cout << "  RECONSTRUCTION TEST\n";
    cout << "========================================\n\n";

    // Subok: reconstruct 35 mula sa n=7 at frac=0.3883
    // 35 = φ^7 × φ^0.3883
    // φ^7 = 29.0344
    // φ^0.3883 = 1.2055
    
    auto ct_phi7 = encrypt_val(pow(PHI, 7));
    auto ct_phi_frac = encrypt_val(pow(PHI, 0.3883));
    
    // Multiply sa constant (libre)
    auto ct_recon = cc->EvalMult(ct_phi7, pow(PHI, 0.3883));
    
    double recon_result = decrypt_val(ct_recon);
    
    cout << "  φ^7 × φ^0.3883 = " << recon_result << " (expected: 35)\n";
    cout << "  Level: " << ct_recon->GetLevel() << "\n";
    cout << "  Match: " << (abs(recon_result - 35.0) < 0.5 ? "✅" : "❌") << "\n\n";

    return 0;
}
