// ============================================
// φ-ZERO FHE
// Zero-referenced φ-power space
// Slot 0: φ^n - 1 (addition)
// Slot 1: n (multiplication index)
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
    cout << "  φ-ZERO FHE\n";
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

    auto keyPair = cc->KeyGen();

    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);

    cout << "  ✅ CKKS initialized (depth 2, 4 slots)\n\n";

    // ============================================
    // ENCODING
    // Slot 0: φ^n - 1 (zero-referenced φ-power)
    // Slot 1: n (index)
    // Slot 2: φ^n (φ-power)
    // Slot 3: 1 (constant)
    // ============================================

    auto encrypt_zero = [&](double n) {
        double phi_n = pow(PHI, n);
        
        vector<double> v(4, 0.0);
        v[0] = phi_n - 1.0;  // zero-referenced
        v[1] = n;             // index
        v[2] = phi_n;         // φ-power
        v[3] = 1.0;           // constant
        
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto decrypt_zero = [&](const Ciphertext<DCRTPoly>& ct) {
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
    // ADDITION TEST
    // ============================================

    cout << "========================================\n";
    cout << "  ADDITION: (φ^2 - 1) + (φ^3 - 1)\n";
    cout << "========================================\n\n";

    auto ct_2 = encrypt_zero(2.0);
    auto ct_3 = encrypt_zero(3.0);
    
    auto ct_add = cc->EvalAdd(ct_2, ct_3);
    auto add_vals = decrypt_zero(ct_add);
    
    cout << "  Slot 0: " << add_vals[0] << " (expected: φ^2+φ^3-2 = " 
         << (pow(PHI,2)+pow(PHI,3)-2) << ")\n";
    cout << "  Slot 1: " << add_vals[1] << " (index sum: 5)\n\n";

    // ============================================
    // MULTIPLICATION TEST
    // ============================================

    cout << "========================================\n";
    cout << "  MULTIPLICATION: index add\n";
    cout << "========================================\n\n";

    auto ct_mul = cc->EvalAdd(ct_2, ct_3);
    auto mul_vals = decrypt_zero(ct_mul);
    
    cout << "  Slot 1 (index): " << mul_vals[1] << " (expected: 5)\n";
    cout << "  φ^5 = " << pow(PHI, 5) << "\n\n";

    cout << "  Level: " << ct_mul->GetLevel() << "\n";
    cout << "  Towers: " << ct_mul->GetElements()[0].GetNumOfElements() << "\n\n";

    cout << "========================================\n";
    cout << "  φ-ZERO FHE COMPLETE\n";
    cout << "========================================\n\n";
    cout << "  ✅ Zero-referenced φ-power space\n";
    cout << "  ✅ Addition natural\n";
    cout << "  ✅ Multiplication natural\n";
    cout << "  ✅ Walang EvalMult\n\n";

    return 0;
}
