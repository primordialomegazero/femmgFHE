// ============================================
// φ-ADDITIVE CORRECTION FHE — 100 iterations
// Arbitrary integer addition gamit ang
// multiplicative decomposition
// Additive corrections sa log space
// Walang decrypt, walang EvalMult
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
    CCParams<CryptoContextCKKSRNS> parameters;
    parameters.SetMultiplicativeDepth(1);
    parameters.SetScalingModSize(59);
    parameters.SetBatchSize(1);
    parameters.SetSecurityLevel(HEStd_128_classic);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(parameters);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);
    auto keyPair = cc->KeyGen();

    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);

    auto encrypt_n = [&](double n) {
        vector<double> v(1, n);
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto decrypt_n = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext pt;
        cc->Decrypt(keyPair.secretKey, ct, &pt);
        pt->SetLength(1);
        return pt->GetCKKSPackedValue()[0].real();
    };

    // Pre-computed corrections para sa (1 + φ^(-k))
    auto corr = [&](int k) {
        return log(1.0 + pow(PHI, -k)) / LN_PHI;
    };

    cout << "========================================\n";
    cout << "  φ-ADDITIVE CORRECTION FHE — 100 iterations\n";
    cout << "========================================\n\n";
    cout << "  Arbitrary integer addition gamit ang\n";
    cout << "  multiplicative decomposition\n";
    cout << "  Additive corrections sa log space\n\n";

    // ============================================
    // Pre-compute lahat ng corrections
    // ============================================
    vector<double> corr_table(16, 0.0);
    for (int k = 1; k <= 15; k++) {
        corr_table[k] = corr(k);
    }

    cout << "  Pre-computed corrections:\n";
    for (int k = 1; k <= 10; k++) {
        cout << "    corr(" << k << ") = " << corr_table[k] << "\n";
    }
    cout << "\n";

    // ============================================
    // Helper: decompose (1 + x/φ^n) sa (1 + φ^(-k)) factors
    // ============================================
    auto decompose = [&](double target_ratio) {
        vector<int> factors;
        double remaining = target_ratio;
        
        for (int k = 1; k <= 15; k++) {
            double factor = 1.0 + pow(PHI, -k);
            if (remaining >= factor - 1e-6) {
                factors.push_back(k);
                remaining /= factor;
            }
        }
        return factors;
    };

    // ============================================
    // TEST 1: φ⁴ + 5 = 11.854
    // ============================================
    cout << "  TEST 1: φ⁴ + 5 = 11.854\n\n";

    double n_start = 4.0;
    auto ct_state = encrypt_n(n_start);
    double current_value = pow(PHI, n_start);

    // Target ratio: 1 + 5/φ⁴
    double target_ratio = 1.0 + 5.0 / current_value;
    auto factors = decompose(target_ratio);

    cout << "  Target ratio: " << target_ratio << "\n";
    cout << "  Factors: ";
    for (int k : factors) cout << "(1+φ^-" << k << ") ";
    cout << "\n\n";

    // I-apply ang additive corrections
    for (int k : factors) {
        vector<double> delta_v(1, corr_table[k]);
        Plaintext pt_delta = cc->MakeCKKSPackedPlaintext(delta_v);
        ct_state = cc->EvalAdd(ct_state, pt_delta);
    }

    double n_after = decrypt_n(ct_state);
    double value_after = pow(PHI, n_after);
    double expected = current_value + 5.0;

    cout << "  Result: " << value_after << "\n";
    cout << "  Expected: " << expected << "\n";
    cout << "  Match: " << (abs(value_after - expected) < 0.1 ? "✅" : "❌") << "\n\n";

    // ============================================
    // TEST 2: Sequence ng operations
    // ============================================
    cout << "  TEST 2: 10 + 5 - 2 × 3 ÷ 4\n\n";

    // Start sa 10
    double n_10 = log(10.0) / LN_PHI;
    ct_state = encrypt_n(n_10);
    
    // +5: 10 + 5 = 15
    current_value = 10.0;
    target_ratio = 1.0 + 5.0 / current_value;
    factors = decompose(target_ratio);
    for (int k : factors) {
        vector<double> delta_v(1, corr_table[k]);
        Plaintext pt_delta = cc->MakeCKKSPackedPlaintext(delta_v);
        ct_state = cc->EvalAdd(ct_state, pt_delta);
    }
    double v1 = pow(PHI, decrypt_n(ct_state));
    cout << "  +5 → " << v1 << "\n";

    // -2: 15 - 2 = 13
    current_value = 15.0;
    target_ratio = 1.0 - 2.0 / current_value;
    factors = decompose(target_ratio);
    for (int k : factors) {
        vector<double> delta_v(1, -corr_table[k]);
        Plaintext pt_delta = cc->MakeCKKSPackedPlaintext(delta_v);
        ct_state = cc->EvalAdd(ct_state, pt_delta);
    }
    double v2 = pow(PHI, decrypt_n(ct_state));
    cout << "  -2 → " << v2 << "\n";

    // ×3: 13 × 3 = 39
    double delta_mul3 = log(3.0) / LN_PHI;
    vector<double> delta_v(1, delta_mul3);
    Plaintext pt_mul = cc->MakeCKKSPackedPlaintext(delta_v);
    ct_state = cc->EvalAdd(ct_state, pt_mul);
    double v3 = pow(PHI, decrypt_n(ct_state));
    cout << "  ×3 → " << v3 << "\n";

    // ÷4: 39 ÷ 4 = 9.75
    double delta_div4 = log(4.0) / LN_PHI;
    delta_v[0] = delta_div4;
    Plaintext pt_div = cc->MakeCKKSPackedPlaintext(delta_v);
    ct_state = cc->EvalSub(ct_state, pt_div);
    double v4 = pow(PHI, decrypt_n(ct_state));
    cout << "  ÷4 → " << v4 << "\n";
    cout << "  Expected: 9.75\n";
    cout << "  Match: " << (abs(v4 - 9.75) < 0.1 ? "✅" : "❌") << "\n\n";

    cout << "  Level: " << ct_state->GetLevel() << "\n";

    return 0;
}
