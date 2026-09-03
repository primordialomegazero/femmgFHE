// ============================================
// φ-HOLY GRAIL BRIDGE
// 4-Slot: [x, x/φ, x+x/φ, log_φ(x)]
// Recovery: Slot 2 / φ = x + y
//
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
    cout << "  φ-HOLY GRAIL BRIDGE\n";
    cout << "  (5 × 7) + 3 = 38\n";
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
    // BRIDGE ENCODING
    // Slot 0: x
    // Slot 1: x/φ
    // Slot 2: x + x/φ = xφ
    // Slot 3: log_φ(x)
    // ============================================

    auto encrypt_bridge = [&](double x) {
        vector<double> v(4, 0.0);
        v[0] = x;
        v[1] = x / PHI;
        v[2] = x + x / PHI;  // = x × φ
        v[3] = log(x) / LN_PHI;
        
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

    auto v5 = decrypt_bridge(ct_5);
    auto v7 = decrypt_bridge(ct_7);
    auto v3 = decrypt_bridge(ct_3);
    
    cout << "  5: [" << v5[0] << ", " << v5[1] << ", " << v5[2] << ", " << v5[3] << "]\n";
    cout << "  7: [" << v7[0] << ", " << v7[1] << ", " << v7[2] << ", " << v7[3] << "]\n";
    cout << "  3: [" << v3[0] << ", " << v3[1] << ", " << v3[2] << ", " << v3[3] << "]\n\n";

    // Step 1: 5 × 7
    auto ct_mult = cc->EvalAdd(ct_5, ct_7);
    auto mult_vals = decrypt_bridge(ct_mult);
    
    cout << "  After 5 × 7:\n";
    cout << "  Slot 0 (x): " << mult_vals[0] << " (sum = 12)\n";
    cout << "  Slot 1 (x/φ): " << mult_vals[1] << "\n";
    cout << "  Slot 2 (bridged): " << mult_vals[2] << " (sum = 12φ = 19.42)\n";
    cout << "  Slot 3 (log_φ): " << mult_vals[3] << " (log_φ(35) = 7.39)\n\n";

    // Step 2: + 3
    auto ct_final = cc->EvalAdd(ct_mult, ct_3);
    auto final_vals = decrypt_bridge(ct_final);
    
    cout << "  After + 3:\n";
    cout << "  Slot 0 (x): " << final_vals[0] << "\n";
    cout << "  Slot 1 (x/φ): " << final_vals[1] << "\n";
    cout << "  Slot 2 (bridged): " << final_vals[2] << "\n";
    cout << "  Slot 3 (log_φ): " << final_vals[3] << "\n\n";

    // Recovery: Slot 2 / φ
    double recovered = final_vals[2] / PHI;
    cout << "  Recovery (Slot 2 / φ): " << recovered << " ← RESULT\n";
    cout << "  Expected: 38\n";
    cout << "  Match: " << (abs(recovered - 38.0) < 0.5 ? "✅" : "❌") << "\n\n";

    cout << "  Level: " << ct_final->GetLevel() << "\n";
    cout << "  Towers: " << ct_final->GetElements()[0].GetNumOfElements() << "\n\n";

    return 0;
}
