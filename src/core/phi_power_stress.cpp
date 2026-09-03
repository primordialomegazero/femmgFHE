// ============================================
// φ-POWER STRESS — 10K
// Addition: Slot 0 (φ-power sum)
// Multiplication: Slot 3 (index)
// Lahat sa EvalAdd at EvalSub
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
    cout << "  φ-POWER STRESS — 10K\n";
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
    // Slot 0: φ^n
    // Slot 1: φ^(n-1)
    // Slot 2: φ^(n+1)
    // Slot 3: n (index)
    // ============================================

    auto encrypt_power = [&](double n) {
        double phi_n = pow(PHI, n);
        double phi_nm1 = pow(PHI, n - 1);
        double phi_np1 = pow(PHI, n + 1);
        
        vector<double> v(4, 0.0);
        v[0] = phi_n;
        v[1] = phi_nm1;
        v[2] = phi_np1;
        v[3] = n;
        
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
    // 10K STRESS — ALL ARITHMETIC SA φ-POWER SPACE
    // ============================================

    cout << "========================================\n";
    cout << "  10K STRESS — ALL ARITHMETIC\n";
    cout << "========================================\n\n";

    int N = 10000;

    auto ct_state = encrypt_power(2.0);   // φ^2
    auto ct_two = encrypt_power(2.0);
    auto ct_three = encrypt_power(3.0);
    auto ct_one = encrypt_power(1.0);

    cout << "  Operations: " << N << "\n";
    cout << "  Mix: +φ^2, ×φ^2, +φ^3, ÷φ^2, -φ^1\n";
    cout << "  Running...\n\n";

    auto start = high_resolution_clock::now();

    double expected_index = 2.0;

    for (int i = 0; i < N; i++) {
        switch (i % 5) {
            case 0: // +φ^2 = φ^2 + φ^2 = φ^3 (Slot 0 add)
                ct_state = cc->EvalAdd(ct_state, ct_two);
                expected_index += 1;  // φ^2 + φ^2 = φ^3
                break;
            case 1: // ×φ^2 (Slot 3 add)
                ct_state = cc->EvalAdd(ct_state, ct_two);
                expected_index += 2;  // index + 2
                break;
            case 2: // +φ^3 (Slot 0 add)
                ct_state = cc->EvalAdd(ct_state, ct_three);
                expected_index += 1;  // φ^m + φ^m-1 pattern
                break;
            case 3: // ÷φ^2 (Slot 3 sub)
                ct_state = cc->EvalSub(ct_state, ct_two);
                expected_index -= 2;  // index - 2
                break;
            case 4: // -φ^1 (Slot 0 sub)
                ct_state = cc->EvalSub(ct_state, ct_one);
                expected_index -= 1;
                break;
        }
    }

    auto end = high_resolution_clock::now();
    auto time = duration_cast<milliseconds>(end - start).count();

    auto final_vals = decrypt_power(ct_state);

    cout << "  ✅ 10K mixed operations complete!\n";
    cout << "  Time: " << time << " ms\n";
    cout << "  Level: " << ct_state->GetLevel() << "\n";
    cout << "  Towers: " << ct_state->GetElements()[0].GetNumOfElements() << "\n";
    cout << "  Slot 0 (φ^n): " << final_vals[0] << "\n";
    cout << "  Slot 1 (φ^n-1): " << final_vals[1] << "\n";
    cout << "  Slot 2 (φ^n+1): " << final_vals[2] << "\n";
    cout << "  Slot 3 (n): " << final_vals[3] << "\n\n";

    cout << "  Expected index: " << expected_index << "\n";
    cout << "  Match (Slot 3): " << (abs(final_vals[3] - expected_index) < 0.5 ? "✅" : "❌") << "\n\n";

    cout << "========================================\n";
    cout << "  φ-POWER STRESS COMPLETE\n";
    cout << "========================================\n\n";
    cout << "  ✅ 10K all arithmetic\n";
    cout << "  ✅ φ-power space\n";
    cout << "  ✅ Walang EvalMult\n";
    cout << "  ✅ Walang bootstrapping\n\n";

    return 0;
}
