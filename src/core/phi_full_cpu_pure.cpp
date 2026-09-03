// ============================================
// φ-FULL CPU PURE
// Bumalik sa full CPU test — walang daya
// (2 + 3) × 2 = 10 — totoong mixed chain
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
    cout << "  φ-FULL CPU PURE\n";
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
    const double PHI_INV = 1.0 / PHI;
    const double LN_PHI = log(PHI);

    cout << "  ✅ CKKS initialized (depth 1, 4 slots)\n\n";

    // ============================================
    // ENCODING — simple dual space
    // Slot 0: x × φ
    // Slot 1: x × φ⁻¹
    // Slot 2: log_φ(x)
    // Slot 3: x (normal)
    // ============================================

    auto encrypt_val = [&](double x) {
        double log_phi_x = log(x) / LN_PHI;
        
        vector<double> v(4, 0.0);
        v[0] = x * PHI;
        v[1] = x * PHI_INV;
        v[2] = log_phi_x;
        v[3] = x;
        
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto decrypt_val = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext result_pt;
        cc->Decrypt(keyPair.secretKey, ct, &result_pt);
        result_pt->SetLength(4);
        auto results = result_pt->GetCKKSPackedValue();
        vector<double> vals(4);
        for (int i = 0; i < 4; i++) vals[i] = results[i].real();
        return vals;
    };

    // ============================================
    // TEST: (2 + 3) × 2 = 10
    // ============================================

    cout << "========================================\n";
    cout << "  TEST: (2 + 3) × 2 = 10\n";
    cout << "========================================\n\n";

    auto ct_2 = encrypt_val(2.0);
    auto ct_3 = encrypt_val(3.0);

    // Step 1: 2 + 3 — addition sa Slot 3
    auto ct_sum = cc->EvalAdd(ct_2, ct_3);
    auto sum_vals = decrypt_val(ct_sum);
    
    cout << "  Step 1: 2 + 3:\n";
    cout << "  Slot 3 (normal): " << sum_vals[3] << " (expected: 5)\n";
    cout << "  Slot 2 (log): " << sum_vals[2] << " (log_φ(6) = " << (log(6.0)/LN_PHI) << ")\n\n";

    // Step 2: × 2 — multiplication sa Slot 2
    auto ct_result = cc->EvalAdd(ct_sum, ct_2);
    auto result_vals = decrypt_val(ct_result);

    double result_log = result_vals[2];
    double result_val = pow(PHI, result_log);
    double result_normal = result_vals[3];

    cout << "  Step 2: (2+3) × 2:\n";
    cout << "  Slot 3 (normal): " << result_normal << " (ito ay 5+2=7 — addition)\n";
    cout << "  Slot 2 (log): " << result_log << " → φ^ = " << result_val << "\n";
    cout << "  Expected: 10\n";
    cout << "  Match (log): " << (abs(result_val - 10.0) < 1.0 ? "✅" : "❌") << "\n\n";

    // ============================================
    // ANG TOTOO
    // ============================================

    cout << "========================================\n";
    cout << "  ANG TOTOO\n";
    cout << "========================================\n\n";

    cout << "  Slot 3 (normal): 7 — addition result\n";
    cout << "  Slot 2 (log): " << result_val << " — multiplication result\n";
    cout << "  Expected: 10 (kailangan ng bridge)\n\n";

    cout << "  Ang bridge ay kailangan para i-convert:\n";
    cout << "  - Slot 3 (5) → Slot 2 (log_φ(5)) para sa multiplication\n";
    cout << "  - O Slot 2 (log_φ(6)) → Slot 3 (6) para sa addition\n\n";

    cout << "  Level: " << ct_result->GetLevel() << "\n";
    cout << "  Towers: " << ct_result->GetElements()[0].GetNumOfElements() << "\n\n";

    return 0;
}
