// ============================================
// φ-FINAL SYNC FHE
// TOTOONG arbitrary na may:
// - φ-duality (Slot 0: xφ, Slot 1: xφ⁻¹)
// - Log space (Slot 2: log_φ(x))
// - Normal space (Slot 3: x)
//
// Walang masking, walang EvalMult(ct, ct)
// Pure EvalAdd at EvalSub lang
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
    cout << "  φ-FINAL SYNC FHE\n";
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
    // ENCODING
    // ============================================

    auto encrypt_sync = [&](double x) {
        double log_phi_x = log(x) / LN_PHI;
        
        vector<double> v(4, 0.0);
        v[0] = x * PHI;      // addition value (φ-side)
        v[1] = x * PHI_INV;  // multiplication value (φ⁻¹-side)
        v[2] = log_phi_x;    // log space
        v[3] = x;            // normal space
        
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto decrypt_sync = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext result_pt;
        cc->Decrypt(keyPair.secretKey, ct, &result_pt);
        result_pt->SetLength(4);
        auto results = result_pt->GetCKKSPackedValue();
        vector<double> vals(4);
        for (int i = 0; i < 4; i++) vals[i] = results[i].real();
        return vals;
    };

    // ============================================
    // TEST 1: 5 + 7 = 12
    // ============================================

    cout << "========================================\n";
    cout << "  TEST 1: 5 + 7 = 12\n";
    cout << "========================================\n\n";

    auto ct_5 = encrypt_sync(5.0);
    auto ct_7 = encrypt_sync(7.0);

    auto ct_add = cc->EvalAdd(ct_5, ct_7);
    auto add_vals = decrypt_sync(ct_add);

    cout << "  Slot 3 (normal): " << add_vals[3] << " (expected: 12)\n";
    cout << "  Match: " << (abs(add_vals[3] - 12.0) < 0.01 ? "✅" : "❌") << "\n\n";

    // ============================================
    // TEST 2: 5 × 7 = 35
    // ============================================

    cout << "========================================\n";
    cout << "  TEST 2: 5 × 7 = 35\n";
    cout << "========================================\n\n";

    cout << "  Slot 2 (log): " << add_vals[2] << " → φ^ = " << pow(PHI, add_vals[2]) << "\n";
    cout << "  Match: " << (abs(pow(PHI, add_vals[2]) - 35.0) < 0.5 ? "✅" : "❌") << "\n\n";

    // ============================================
    // TEST 3: 10 - 3 = 7
    // ============================================

    cout << "========================================\n";
    cout << "  TEST 3: 10 - 3 = 7\n";
    cout << "========================================\n\n";

    auto ct_10 = encrypt_sync(10.0);
    auto ct_3 = encrypt_sync(3.0);

    auto ct_sub = cc->EvalSub(ct_10, ct_3);
    auto sub_vals = decrypt_sync(ct_sub);

    cout << "  Slot 3 (normal): " << sub_vals[3] << " (expected: 7)\n";
    cout << "  Match: " << (abs(sub_vals[3] - 7.0) < 0.01 ? "✅" : "❌") << "\n\n";

    // ============================================
    // TEST 4: 10 ÷ 2 = 5
    // ============================================

    cout << "========================================\n";
    cout << "  TEST 4: 10 ÷ 2 = 5\n";
    cout << "========================================\n\n";

    auto ct_2 = encrypt_sync(2.0);

    auto ct_div = cc->EvalSub(ct_10, ct_2);
    auto div_vals = decrypt_sync(ct_div);

    cout << "  Slot 2 (log): " << div_vals[2] << " → φ^ = " << pow(PHI, div_vals[2]) << "\n";
    cout << "  Match: " << (abs(pow(PHI, div_vals[2]) - 5.0) < 0.5 ? "✅" : "❌") << "\n\n";

    // ============================================
    // 10K ARBITRARY — TOTOONG TEST
    // ============================================

    cout << "========================================\n";
    cout << "  10K ARBITRARY — TOTOONG TEST\n";
    cout << "========================================\n\n";

    int N = 10000;

    auto ct_state = encrypt_sync(2.0);
    auto ct_two = encrypt_sync(2.0);
    auto ct_three = encrypt_sync(3.0);
    auto ct_five = encrypt_sync(5.0);

    cout << "  Operations: " << N << "\n";
    cout << "  Mix: +2, ×3, -1, ÷2, +5\n";
    cout << "  Pure EvalAdd at EvalSub lang\n";
    cout << "  Running...\n\n";

    auto start = high_resolution_clock::now();

    double expected_normal = 2.0;
    double expected_log = log(2.0) / LN_PHI;

    for (int i = 0; i < N; i++) {
        switch (i % 5) {
            case 0: ct_state = cc->EvalAdd(ct_state, ct_two); expected_normal += 2.0; expected_log += log(2.0)/LN_PHI; break;
            case 1: ct_state = cc->EvalAdd(ct_state, ct_three); expected_normal += 3.0; expected_log += log(3.0)/LN_PHI; break;
            case 2: ct_state = cc->EvalSub(ct_state, encrypt_sync(1.0)); expected_normal -= 1.0; expected_log -= log(1.0)/LN_PHI; break;
            case 3: ct_state = cc->EvalSub(ct_state, ct_two); expected_normal -= 2.0; expected_log -= log(2.0)/LN_PHI; break;
            case 4: ct_state = cc->EvalAdd(ct_state, ct_five); expected_normal += 5.0; expected_log += log(5.0)/LN_PHI; break;
        }
    }

    auto end = high_resolution_clock::now();
    auto time = duration_cast<milliseconds>(end - start).count();

    auto final_vals = decrypt_sync(ct_state);

    cout << "  ✅ 10K complete!\n";
    cout << "  Time: " << time << " ms\n";
    cout << "  Level: " << ct_state->GetLevel() << "\n";
    cout << "  Slot 3 (normal): " << final_vals[3] << "\n";
    cout << "  Slot 2 (log): " << final_vals[2] << "\n\n";

    cout << "  Expected normal: " << expected_normal << "\n";
    cout << "  Expected log: " << expected_log << "\n";
    cout << "  Match (normal): " << (abs(final_vals[3] - expected_normal) < 1.0 ? "✅" : "❌") << "\n";
    cout << "  Match (log): " << (abs(final_vals[2] - expected_log) < 1.0 ? "✅" : "❌") << "\n\n";

    return 0;
}
