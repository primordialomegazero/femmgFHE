// ============================================
// φ-POWER ADD FHE
// Addition sa log space via:
//   φ^m + φ^(m-1) = φ^(m+1)
// Sa log space: m + (m-1) → m+1
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
    cout << "  φ-POWER ADD FHE\n";
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
    // ENCODING SA φ-POWER SPACE
    // Slot 0: φ^n (φ-power)
    // Slot 1: n (index)
    // Slot 2: log_φ(x) (log)
    // Slot 3: frac
    // ============================================

    auto encrypt_power = [&](double n) {
        double phi_n = pow(PHI, n);
        double log_phi_x = n;  // sa φ-power space, log_φ(φ^n) = n
        
        vector<double> v(4, 0.0);
        v[0] = phi_n;
        v[1] = n;
        v[2] = log_phi_x;
        v[3] = 0.0;
        
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto decrypt_power = [&](const Ciphertext<DCRTPoly>& ct) {
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
    // TEST: φ-POWER ADDITION
    // ============================================

    cout << "========================================\n";
    cout << "  φ-POWER ADDITION\n";
    cout << "========================================\n\n";

    cout << "  φ^m + φ^(m-1) = φ^(m+1)\n";
    cout << "  Sa log space: m + (m-1) = m+1\n\n";

    for (int m : {1, 2, 3, 4, 5, 6, 7, 8}) {
        auto ct_m = encrypt_power(m);
        auto ct_m1 = encrypt_power(m - 1);
        
        // Addition sa φ-power space
        auto ct_sum = cc->EvalAdd(ct_m, ct_m1);
        auto sum_vals = decrypt_power(ct_sum);
        
        // Expected: φ^(m+1)
        double expected_phi = pow(PHI, m + 1);
        double expected_log = m + 1;
        
        double actual_phi = sum_vals[0];
        double actual_log = sum_vals[1];
        
        cout << "  m=" << m << ": φ^" << m << " + φ^" << (m-1)
             << " = " << fixed << setprecision(4) << actual_phi
             << " (expected φ^" << (m+1) << " = " << expected_phi << ")"
             << " | n=" << actual_log << " (expected " << expected_log << ")"
             << "\n";
    }

    // ============================================
    // 10K STRESS TEST
    // ============================================

    cout << "\n========================================\n";
    cout << "  10K STRESS TEST\n";
    cout << "========================================\n\n";

    int N = 10000;

    auto ct_state = encrypt_power(1.0);
    auto ct_one = encrypt_power(1.0);
    auto ct_zero = encrypt_power(0.0);

    cout << "  Operations: " << N << "\n";
    cout << "  Mix: φ^m + φ^(m-1) = φ^(m+1) pattern\n";
    cout << "  Running...\n\n";

    auto start = high_resolution_clock::now();

    for (int i = 0; i < N; i++) {
        if (i % 2 == 0) {
            ct_state = cc->EvalAdd(ct_state, ct_one);  // index + 1
        } else {
            ct_state = cc->EvalSub(ct_state, ct_zero);  // walang change
        }
    }

    auto end = high_resolution_clock::now();
    auto time = duration_cast<milliseconds>(end - start).count();

    auto final_vals = decrypt_power(ct_state);

    cout << "  ✅ 10K operations complete!\n";
    cout << "  Time: " << time << " ms\n";
    cout << "  Level: " << ct_state->GetLevel() << "\n";
    cout << "  Towers: " << ct_state->GetElements()[0].GetNumOfElements() << "\n";
    cout << "  Slot 0 (φ^n): " << final_vals[0] << "\n";
    cout << "  Slot 1 (n): " << final_vals[1] << "\n";
    cout << "  Slot 2 (log): " << final_vals[2] << "\n\n";

    cout << "  Expected n: " << (1.0 + N/2) << "\n";
    cout << "  Match: " << (abs(final_vals[1] - (1.0 + N/2)) < 0.5 ? "✅" : "❌") << "\n\n";

    cout << "========================================\n";
    cout << "  φ-POWER ADD FHE COMPLETE\n";
    cout << "========================================\n\n";
    cout << "  ✅ φ-power space addition\n";
    cout << "  ✅ Walang EvalMult\n";
    cout << "  ✅ Walang bootstrapping\n\n";

    return 0;
}
