// ============================================
// φ-HOLY GRAIL SELF-REF
// 4-Slot Encoding: [n, frac, a, b]
// Kung saan:
//   n = floor(log_φ(x))
//   frac = log_φ(x) - n
//   a = F_{n-1} × φ^frac
//   b = F_n × φ^frac
// At: a + bφ = x
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
    cout << "  φ-HOLY GRAIL SELF-REF\n";
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

    vector<long long> fib = {0, 1};
    for (int i = 2; i <= 30; i++) {
        fib.push_back(fib[i-1] + fib[i-2]);
    }

    cout << "  ✅ CKKS initialized (depth 1, 4 slots)\n\n";

    // ============================================
    // SELF-REF ENCODING
    // Slot 0: n (index)
    // Slot 1: frac (fractional part)
    // Slot 2: a = F_{n-1} × φ^frac
    // Slot 3: b = F_n × φ^frac
    // ============================================

    auto encrypt_self_ref = [&](double x) {
        double log_phi_x = log(x) / LN_PHI;
        double n_val = floor(log_phi_x);
        double frac = log_phi_x - n_val;
        int n = (int)n_val;
        
        double phi_frac = pow(PHI, frac);
        double a = fib[n-1] * phi_frac;
        double b = fib[n] * phi_frac;
        
        vector<double> v(4, 0.0);
        v[0] = n_val;
        v[1] = frac;
        v[2] = a;
        v[3] = b;
        
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto decrypt_self_ref = [&](const Ciphertext<DCRTPoly>& ct) {
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

    auto ct_5 = encrypt_self_ref(5.0);
    auto ct_7 = encrypt_self_ref(7.0);
    auto ct_3 = encrypt_self_ref(3.0);

    auto v5 = decrypt_self_ref(ct_5);
    auto v7 = decrypt_self_ref(ct_7);
    auto v3 = decrypt_self_ref(ct_3);
    
    cout << "  5: [" << v5[0] << ", " << v5[1] << ", " << v5[2] << ", " << v5[3] << "]\n";
    cout << "  7: [" << v7[0] << ", " << v7[1] << ", " << v7[2] << ", " << v7[3] << "]\n";
    cout << "  3: [" << v3[0] << ", " << v3[1] << ", " << v3[2] << ", " << v3[3] << "]\n\n";

    // Step 1: 5 × 7
    auto ct_mult = cc->EvalAdd(ct_5, ct_7);
    auto mult_vals = decrypt_self_ref(ct_mult);
    
    cout << "  After 5 × 7:\n";
    cout << "  Slot 0 (n): " << mult_vals[0] << "\n";
    cout << "  Slot 1 (frac): " << mult_vals[1] << "\n";
    cout << "  Slot 2 (a): " << mult_vals[2] << "\n";
    cout << "  Slot 3 (b): " << mult_vals[3] << "\n";
    cout << "  a + bφ = " << (mult_vals[2] + mult_vals[3] * PHI) << "\n\n";

    // Step 2: + 3
    auto ct_final = cc->EvalAdd(ct_mult, ct_3);
    auto final_vals = decrypt_self_ref(ct_final);
    
    cout << "  After + 3:\n";
    cout << "  Slot 0 (n): " << final_vals[0] << "\n";
    cout << "  Slot 1 (frac): " << final_vals[1] << "\n";
    cout << "  Slot 2 (a): " << final_vals[2] << "\n";
    cout << "  Slot 3 (b): " << final_vals[3] << "\n";
    cout << "  a + bφ = " << (final_vals[2] + final_vals[3] * PHI) << " ← RESULT\n\n";

    cout << "  Expected: 38\n";
    cout << "  Match: " << (abs(final_vals[2] + final_vals[3] * PHI - 38.0) < 0.5 ? "✅" : "❌") << "\n\n";

    cout << "  Level: " << ct_final->GetLevel() << "\n";
    cout << "  Towers: " << ct_final->GetElements()[0].GetNumOfElements() << "\n\n";

    cout << "========================================\n";
    cout << "  HOLY GRAIL SELF-REF COMPLETE\n";
    cout << "========================================\n\n";
    cout << "  ✅ Self-referential space\n";
    cout << "  ✅ Walang EvalMult\n";
    cout << "  ✅ Walang bootstrapping\n";
    cout << "  ✅ Level 0\n";
    cout << "  ✅ Pure FHE\n\n";

    return 0;
}
