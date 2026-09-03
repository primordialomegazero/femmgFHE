// ============================================
// φ-EMERGENT DUAL MIXED
// Subok ng non-traditional approach:
// Ang log space at normal space ay naka-encode
// para awtomatikong mag-sync pagkatapos ng operasyon
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
    cout << "  φ-EMERGENT DUAL MIXED\n";
    cout << "========================================\n\n";

    CCParams<CryptoContextCKKSRNS> parameters;
    parameters.SetMultiplicativeDepth(1);
    parameters.SetScalingModSize(50);
    parameters.SetBatchSize(2);
    parameters.SetSecurityLevel(HEStd_128_classic);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(parameters);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);

    auto keyPair = cc->KeyGen();

    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);

    cout << "  ✅ CKKS initialized (depth 1, 2 slots)\n\n";

    // ============================================
    // EMERGENT DUAL ENCODING
    // ============================================

    auto encrypt_emergent = [&](double value) {
        vector<double> v(2, 0.0);
        v[0] = value;                        // Normal space
        v[1] = log(value) / LN_PHI;          // Log space (φ-base)
        
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto decrypt_dual = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext result_pt;
        cc->Decrypt(keyPair.secretKey, ct, &result_pt);
        result_pt->SetLength(2);
        auto results = result_pt->GetCKKSPackedValue();
        
        double normal = results[0].real();
        double log_val = results[1].real();
        double log_result = pow(PHI, log_val);
        
        return make_pair(normal, log_result);
    };

    // ============================================
    // TEST: (5 × 7) + 3 = 38
    // ============================================

    cout << "========================================\n";
    cout << "  TEST: (5 × 7) + 3\n";
    cout << "========================================\n\n";

    auto ct_5 = encrypt_emergent(5.0);
    auto ct_7 = encrypt_emergent(7.0);
    auto ct_3 = encrypt_emergent(3.0);

    // Step 1: 5 × 7 sa log space
    auto ct_mult = cc->EvalAdd(ct_5, ct_7);
    auto [mult_normal, mult_log] = decrypt_dual(ct_mult);
    
    cout << "  After 5 × 7:\n";
    cout << "  Normal: " << mult_normal << " (expected: 12)\n";
    cout << "  Log: " << mult_log << " (expected: 35)\n\n";

    // Step 2: + 3
    // Subok: EvalAdd ng ct_3 sa ct_mult
    auto ct_result = cc->EvalAdd(ct_mult, ct_3);
    auto [result_normal, result_log] = decrypt_dual(ct_result);
    
    cout << "  After + 3:\n";
    cout << "  Normal: " << result_normal << " (expected: 38)\n";
    cout << "  Log: " << result_log << " (expected: 105)\n";
    cout << "  Match (normal): " << (abs(result_normal - 38.0) < 0.5 ? "✅" : "❌") << "\n\n";

    cout << "  Level: " << ct_result->GetLevel() << "\n";
    cout << "  Towers: " << ct_result->GetElements()[0].GetNumOfElements() << "\n\n";

    // ============================================
    // OBSERVATION
    // ============================================

    cout << "========================================\n";
    cout << "  OBSERVATION\n";
    cout << "========================================\n\n";
    cout << "  Ang normal space ay nag-add (12 + 3 = 15)\n";
    cout << "  Ang log space ay nag-multiply (35 × 3 = 105)\n";
    cout << "  Walang automatic sync na nangyari.\n\n";
    cout << "  Kailangan ng paraan para i-extract\n";
    cout << "  ang 35 mula sa log space papuntang normal\n";
    cout << "  space nang walang decrypt.\n\n";

    return 0;
}
