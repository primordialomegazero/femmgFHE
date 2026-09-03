// ============================================
// φ-LOOKUP CONVERSION
// Pre-computed φ^n values naka-encode sa slots
// para sa instant conversion nang walang EvalMult
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
    cout << "  φ-LOOKUP CONVERSION\n";
    cout << "========================================\n\n";

    CCParams<CryptoContextCKKSRNS> parameters;
    parameters.SetMultiplicativeDepth(1);
    parameters.SetScalingModSize(50);
    parameters.SetBatchSize(16);
    parameters.SetSecurityLevel(HEStd_128_classic);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(parameters);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);

    auto keyPair = cc->KeyGen();

    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);

    cout << "  ✅ CKKS initialized (depth 1, 16 slots)\n\n";

    // ============================================
    // LOOKUP TABLE: φ^0, φ^1, φ^2, ..., φ^15
    // ============================================

    vector<double> phi_powers(16, 0.0);
    for (int i = 0; i < 16; i++) {
        phi_powers[i] = pow(PHI, i);
    }

    cout << "  φ^n lookup table (slots 0-15):\n";
    cout << "  Slot | φ^n\n";
    cout << "  -----|------\n";
    for (int i = 0; i < 16; i++) {
        cout << "  " << setw(4) << i << " | " << phi_powers[i] << "\n";
    }
    cout << "\n";

    // ============================================
    // DUAL ENCODING NA MAY LOOKUP
    // ============================================

    auto encrypt_lookup = [&](double value) {
        vector<double> v(16, 0.0);
        
        // Slot 0: Normal value
        v[0] = value;
        
        // Slots 1-15: φ^n lookup table
        for (int i = 1; i < 16; i++) {
            v[i] = phi_powers[i];
        }
        
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto decrypt_lookup = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext result_pt;
        cc->Decrypt(keyPair.secretKey, ct, &result_pt);
        result_pt->SetLength(16);
        auto results = result_pt->GetCKKSPackedValue();
        vector<double> vals(16);
        for (int i = 0; i < 16; i++) {
            vals[i] = results[i].real();
        }
        return vals;
    };

    // ============================================
    // TEST: (5 × 7) + 3
    // ============================================

    cout << "========================================\n";
    cout << "  TEST: (5 × 7) + 3\n";
    cout << "========================================\n\n";

    auto ct_5 = encrypt_lookup(5.0);
    auto ct_7 = encrypt_lookup(7.0);
    auto ct_3 = encrypt_lookup(3.0);

    // Step 1: 5 × 7 sa log space
    auto ct_mult = cc->EvalAdd(ct_5, ct_7);
    
    auto mult_vals = decrypt_lookup(ct_mult);
    
    cout << "  After 5 × 7:\n";
    cout << "  Slot 0 (Normal): " << mult_vals[0] << "\n";
    cout << "  Slot 1 (φ^1): " << mult_vals[1] << "\n";
    cout << "  Slot 7 (φ^7): " << mult_vals[7] << "\n";
    cout << "  Slot 8 (φ^8): " << mult_vals[8] << "\n\n";

    // Step 2: + 3
    auto ct_final = cc->EvalAdd(ct_mult, ct_3);
    
    auto final_vals = decrypt_lookup(ct_final);
    
    cout << "  After + 3:\n";
    cout << "  Slot 0 (Normal): " << final_vals[0] << "\n";
    cout << "  Slot 1 (φ^1): " << final_vals[1] << "\n";
    cout << "  Slot 7 (φ^7): " << final_vals[7] << "\n";
    cout << "  Slot 8 (φ^8): " << final_vals[8] << "\n\n";

    cout << "  Level: " << ct_final->GetLevel() << "\n";
    cout << "  Towers: " << ct_final->GetElements()[0].GetNumOfElements() << "\n\n";

    return 0;
}
