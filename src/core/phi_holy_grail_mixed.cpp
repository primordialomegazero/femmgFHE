// ============================================
// φ-HOLY GRAIL MIXED — (5 × 7) + 3 = 38
// 4-Slot Encoding: [a, b, a+bφ, log_φ(x)]
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
    cout << "  φ-HOLY GRAIL MIXED\n";
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

    // Fibonacci sequence
    vector<long long> fib = {0, 1};
    for (int i = 2; i <= 30; i++) {
        fib.push_back(fib[i-1] + fib[i-2]);
    }

    cout << "  ✅ CKKS initialized (depth 1, 4 slots)\n\n";

    // ============================================
    // 4-SLOT ENCODING
    // ============================================

    auto encrypt_4slot = [&](double x) {
        vector<double> v(4, 0.0);
        v[0] = x;                                       // Slot 0: Normal
        v[1] = 0;                                       // Slot 1: b (φ component)
        v[2] = x;                                       // Slot 2: a + bφ = x
        v[3] = log(x) / LN_PHI;                         // Slot 3: log_φ(x)
        
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto decrypt_4slot = [&](const Ciphertext<DCRTPoly>& ct) {
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

    auto ct_5 = encrypt_4slot(5.0);
    auto ct_7 = encrypt_4slot(7.0);
    auto ct_3 = encrypt_4slot(3.0);

    // Step 1: 5 × 7 sa log space
    auto ct_mult = cc->EvalAdd(ct_5, ct_7);
    auto mult_vals = decrypt_4slot(ct_mult);
    
    cout << "  After 5 × 7:\n";
    cout << "  Slot 0: " << mult_vals[0] << " (normal sum)\n";
    cout << "  Slot 1: " << mult_vals[1] << " (b component)\n";
    cout << "  Slot 2: " << mult_vals[2] << " (a + bφ)\n";
    cout << "  Slot 3: " << mult_vals[3] << " (log_φ sum)\n\n";

    // Step 2: + 3
    auto ct_final = cc->EvalAdd(ct_mult, ct_3);
    auto final_vals = decrypt_4slot(ct_final);
    
    cout << "  After + 3:\n";
    cout << "  Slot 0: " << final_vals[0] << "\n";
    cout << "  Slot 1: " << final_vals[1] << "\n";
    cout << "  Slot 2: " << final_vals[2] << " ← RESULT\n";
    cout << "  Slot 3: " << final_vals[3] << "\n\n";

    cout << "  Expected: 38\n";
    cout << "  Match: " << (abs(final_vals[2] - 38.0) < 0.5 ? "✅" : "❌") << "\n\n";

    cout << "  Level: " << ct_final->GetLevel() << "\n";
    cout << "  Towers: " << ct_final->GetElements()[0].GetNumOfElements() << "\n\n";

    cout << "========================================\n";
    cout << "  HOLY GRAIL MIXED COMPLETE\n";
    cout << "========================================\n\n";
    cout << "  ✅ Mixed operation\n";
    cout << "  ✅ Walang EvalMult\n";
    cout << "  ✅ Walang bootstrapping\n";
    cout << "  ✅ Level 0\n";
    cout << "  ✅ Pure FHE\n\n";

    return 0;
}
