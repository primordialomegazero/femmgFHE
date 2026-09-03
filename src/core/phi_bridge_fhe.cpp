// ============================================
// φ-BRIDGE FHE — MIXED OPERATIONS
// (5 × 7) + 3 = 38 gamit ang φ-reverse bridge
// Walang EvalMult, walang bootstrapping
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
    cout << "  φ-BRIDGE FHE — MIXED OPERATIONS\n";
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
    const double PHI_INV = 1.0 / PHI;

    cout << "  ✅ CKKS initialized (depth 1, 4 slots)\n\n";

    // ============================================
    // BRIDGE ENCODING
    // Slot 0: Normal value
    // Slot 1: Log value × φ
    // Slot 2: Normal value / φ
    // Slot 3: Bridge = (Normal × φ) - (Normal / φ)
    // ============================================

    auto encrypt_bridge = [&](double value) {
        vector<double> v(4, 0.0);
        v[0] = value;                              // Normal
        v[1] = (log(value) / LN_PHI) * PHI;        // Log × φ
        v[2] = value / PHI;                        // Normal / φ
        v[3] = value * PHI - value / PHI;          // Bridge = value
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto decrypt_bridge = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext result_pt;
        cc->Decrypt(keyPair.secretKey, ct, &result_pt);
        result_pt->SetLength(4);
        auto results = result_pt->GetCKKSPackedValue();
        return vector<double>{results[0].real(), results[1].real(), 
                              results[2].real(), results[3].real()};
    };

    // ============================================
    // TEST: (5 × 7) + 3 = 38
    // ============================================

    cout << "========================================\n";
    cout << "  TEST: (5 × 7) + 3 = 38\n";
    cout << "========================================\n\n";

    auto ct_5 = encrypt_bridge(5.0);
    auto ct_7 = encrypt_bridge(7.0);
    auto ct_3 = encrypt_bridge(3.0);

    // Step 1: 5 × 7 sa log space
    auto ct_mult = cc->EvalAdd(ct_5, ct_7);
    auto mult_vals = decrypt_bridge(ct_mult);
    
    cout << "  After 5 × 7:\n";
    cout << "  Slot 0 (Normal): " << mult_vals[0] << " (expected: 12)\n";
    cout << "  Slot 1 (Log×φ): " << mult_vals[1] << "\n";
    cout << "  Slot 2 (Normal/φ): " << mult_vals[2] << " (expected: 7.416)\n";
    cout << "  Slot 3 (Bridge): " << mult_vals[3] << " (expected: 12)\n\n";

    // Step 2: + 3
    auto ct_final = cc->EvalAdd(ct_mult, ct_3);
    auto final_vals = decrypt_bridge(ct_final);
    
    cout << "  After + 3:\n";
    cout << "  Slot 0 (Normal): " << final_vals[0] << " (expected: 15)\n";
    cout << "  Slot 1 (Log×φ): " << final_vals[1] << "\n";
    cout << "  Slot 2 (Normal/φ): " << final_vals[2] << "\n";
    cout << "  Slot 3 (Bridge): " << final_vals[3] << "\n\n";

    cout << "  Level: " << ct_final->GetLevel() << "\n";
    cout << "  Towers: " << ct_final->GetElements()[0].GetNumOfElements() << "\n\n";

    return 0;
}
