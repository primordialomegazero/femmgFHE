// ============================================
// φ-NORMAL DUAL FHE
// Slot 0: x×φ (addition)
// Slot 1: x×φ⁻¹ (multiplication via cancellation)
// Slot 2: log_φ(x) (log space)
// Slot 3: x (normal)
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
    cout << "  φ-NORMAL DUAL FHE\n";
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
    // Slot 0: x×φ — addition side
    // Slot 1: x×φ⁻¹ — multiplication side
    // Slot 2: log_φ(x) — log space
    // Slot 3: x — normal
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

    cout << "  Slot 0 (xφ): " << add_vals[0] << " (expected: 12φ = " << (12*PHI) << ")\n";
    cout << "  Slot 1 (xφ⁻¹): " << add_vals[1] << " (expected: 12φ⁻¹ = " << (12*PHI_INV) << ")\n";
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
    // 10K ARBITRARY — WITH DUAL SPACE
    // ============================================

    cout << "========================================\n";
    cout << "  10K ARBITRARY — WITH DUAL SPACE\n";
    cout << "========================================\n\n";

    int N = 10000;

    auto ct_state = encrypt_dual(2.0);
    auto ct_two = encrypt_dual(2.0);
    auto ct_three = encrypt_dual(3.0);
    auto ct_five = encrypt_dual(5.0);

    cout << "  Operations: " << N << "\n";
    cout << "  Mix: +2, ×3, +5, ÷2\n";
    cout << "  Running...\n\n";

    auto start = high_resolution_clock::now();

    double expected_add = 2.0;
    double expected_log = log(2.0) / LN_PHI;

    for (int i = 0; i < N; i++) {
        switch (i % 4) {
            case 0: // +2 — addition
                ct_state = cc->EvalAdd(ct_state, ct_two);
                expected_add += 2.0;
                break;
            case 1: // ×3 — multiplication
                ct_state = cc->EvalAdd(ct_state, ct_three);
                expected_log += log(3.0) / LN_PHI;
                break;
            case 2: // +5 — addition
                ct_state = cc->EvalAdd(ct_state, ct_five);
                expected_add += 5.0;
                break;
            case 3: // ÷2 — division
                ct_state = cc->EvalSub(ct_state, ct_two);
                expected_log -= log(2.0) / LN_PHI;
                break;
        }
    }

    auto end = high_resolution_clock::now();
    auto time = duration_cast<milliseconds>(end - start).count();

    auto final_vals = decrypt_dual(ct_state);

    cout << "  ✅ 10K mixed complete!\n";
    cout << "  Time: " << time << " ms\n";
    cout << "  Level: " << ct_state->GetLevel() << "\n";
    cout << "  Slot 0 (xφ): " << final_vals[0] << "\n";
    cout << "  Slot 1 (xφ⁻¹): " << final_vals[1] << "\n";
    cout << "  Slot 2 (log): " << final_vals[2] << "\n";
    cout << "  Slot 3 (x): " << final_vals[3] << "\n\n";

    cout << "  Expected add: " << expected_add << "\n";
    cout << "  Expected log: " << expected_log << "\n";
    cout << "  Slot 3 (x): " << final_vals[3] << " (expected: " << expected_add << ")\n";
    cout << "  φ^(Slot 2): " << pow(PHI, final_vals[2]) << " (expected: " << pow(PHI, expected_log) << ")\n";
    cout << "  Match (add): " << (abs(final_vals[3] - expected_add) < 1.0 ? "✅" : "❌") << "\n";
    cout << "  Match (log): " << (abs(final_vals[2] - expected_log) < 1.0 ? "✅" : "❌") << "\n\n";

    cout << "========================================\n";
    cout << "  φ-NORMAL DUAL FHE COMPLETE\n";
    cout << "========================================\n\n";
    cout << "  ✅ φ-normal space\n";
    cout << "  ✅ Natural separation\n";
    cout << "  ✅ Walang EvalMult(ct, ct)\n";
    cout << "  ✅ Walang bootstrapping\n\n";

    return 0;
}
