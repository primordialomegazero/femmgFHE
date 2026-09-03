// ============================================
// φ-DUALITY FHE
// Slot 0: x×φ (φ-side) — addition
// Slot 1: x×φ⁻¹ (φ⁻¹-side) — addition
// Slot 2: log_φ(x) — multiplication
// Slot 3: x (normal) — recovery
//
// Natural separation via φ × φ⁻¹ = 1
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
    cout << "  φ-DUALITY FHE\n";
    cout << "========================================\n\n";

    CCParams<CryptoContextCKKSRNS> parameters;
    parameters.SetMultiplicativeDepth(2);
    parameters.SetScalingModSize(50);
    parameters.SetBatchSize(4);
    parameters.SetSecurityLevel(HEStd_128_classic);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(parameters);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);

    auto keyPair = cc->KeyGen();

    const double PHI = 1.6180339887498948482;
    const double PHI_INV = 1.0 / PHI;
    const double LN_PHI = log(PHI);

    cout << "  ✅ CKKS initialized (depth 2, 4 slots)\n\n";

    // ============================================
    // ENCODING
    // Slot 0: x×φ
    // Slot 1: x×φ⁻¹
    // Slot 2: log_φ(x)
    // Slot 3: x
    // ============================================

    auto encrypt_dual = [&](double x) {
        double log_phi_x = log(x) / LN_PHI;
        
        vector<double> v(4, 0.0);
        v[0] = x * PHI;
        v[1] = x * PHI_INV;
        v[2] = log_phi_x;
        v[3] = x;
        
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto decrypt_dual = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext result_pt;
        cc->Decrypt(keyPair.secretKey, ct, &result_pt);
        result_pt->SetLength(4);
        auto results = result_pt->GetCKKSPackedValue();
        vector<double> vals(4);
        for (int i = 0; i < 4; i++) vals[i] = results[i].real();
        return vals;
    };

    // ============================================
    // TEST: 5 + 7 = 12
    // ============================================

    cout << "========================================\n";
    cout << "  ADDITION: 5 + 7 = 12\n";
    cout << "========================================\n\n";

    auto ct_5 = encrypt_dual(5.0);
    auto ct_7 = encrypt_dual(7.0);

    auto ct_add = cc->EvalAdd(ct_5, ct_7);
    auto add_vals = decrypt_dual(ct_add);

    cout << "  Slot 0 (xφ): " << add_vals[0] << " (expected: 12×φ = " << (12*PHI) << ")\n";
    cout << "  Slot 1 (xφ⁻¹): " << add_vals[1] << " (expected: 12×φ⁻¹ = " << (12*PHI_INV) << ")\n";
    cout << "  Slot 2 (log): " << add_vals[2] << " (log_φ(35) = " << (log(35.0)/LN_PHI) << ")\n";
    cout << "  Slot 3 (x): " << add_vals[3] << " (expected: 12)\n";
    cout << "  Match (Slot 3): " << (abs(add_vals[3] - 12.0) < 0.01 ? "✅" : "❌") << "\n\n";

    // ============================================
    // TEST: 5 × 7 = 35
    // ============================================

    cout << "========================================\n";
    cout << "  MULTIPLICATION: 5 × 7 = 35\n";
    cout << "========================================\n\n";

    cout << "  Slot 2 (log): " << add_vals[2] << " → φ^ = " << pow(PHI, add_vals[2]) << "\n";
    cout << "  Match: " << (abs(pow(PHI, add_vals[2]) - 35.0) < 0.5 ? "✅" : "❌") << "\n\n";

    // ============================================
    // TEST: (5 × 7) + 3 = 38
    // ============================================

    cout << "========================================\n";
    cout << "  MIXED: (5 × 7) + 3 = 38\n";
    cout << "========================================\n\n";

    auto ct_3 = encrypt_dual(3.0);
    auto ct_result = cc->EvalAdd(ct_add, ct_3);
    auto result_vals = decrypt_dual(ct_result);

    cout << "  Slot 3 (x): " << result_vals[3] << " (expected: 15 o 38?)\n";
    cout << "  Slot 2 (log): " << result_vals[2] << " → φ^ = " << pow(PHI, result_vals[2]) << "\n\n";

    cout << "  Level: " << ct_result->GetLevel() << "\n";
    cout << "  Towers: " << ct_result->GetElements()[0].GetNumOfElements() << "\n\n";

    return 0;
}
