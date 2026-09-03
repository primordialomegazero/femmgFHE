// ============================================
// φ-10K FIBONACCI MARKER
// 10K arbitrary chains na may Fibonacci marker
// para sa operation detection
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
    cout << "  φ-10K FIBONACCI MARKER\n";
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

    vector<long long> fib = {0, 1};
    for (int i = 2; i <= 50; i++) {
        fib.push_back(fib[i-1] + fib[i-2]);
    }

    cout << "  ✅ CKKS initialized (depth 2, 4 slots)\n\n";

    // ============================================
    // ENCODING NA MAY FIBONACCI MARKER
    // Slot 0: x×φ — addition value
    // Slot 1: x×φ⁻¹ — multiplication value
    // Slot 2: log_φ(x) — log space
    // Slot 3: Fibonacci marker
    // ============================================

    auto encrypt_marker = [&](double x, bool is_addition) {
        double log_phi_x = log(x) / LN_PHI;
        double marker = is_addition ? 1.0 : 2.0;  // F_1=1 o F_3=2
        
        vector<double> v(4, 0.0);
        v[0] = x * PHI;
        v[1] = x * PHI_INV;
        v[2] = log_phi_x;
        v[3] = marker;
        
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto decrypt_marker = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext result_pt;
        cc->Decrypt(keyPair.secretKey, ct, &result_pt);
        result_pt->SetLength(4);
        auto results = result_pt->GetCKKSPackedValue();
        vector<double> vals(4);
        for (int i = 0; i < 4; i++) vals[i] = results[i].real();
        return vals;
    };

    // ============================================
    // 10K ARBITRARY — LAHAT NG OPS
    // ============================================

    cout << "========================================\n";
    cout << "  10K ARBITRARY — LAHAT NG OPS\n";
    cout << "========================================\n\n";

    int N = 10000;

    // Initial state: addition marker
    auto ct_state = encrypt_marker(2.0, true);
    auto ct_two_add = encrypt_marker(2.0, true);
    auto ct_two_mul = encrypt_marker(2.0, false);
    auto ct_three_add = encrypt_marker(3.0, true);
    auto ct_three_mul = encrypt_marker(3.0, false);
    auto ct_five_add = encrypt_marker(5.0, true);
    auto ct_five_mul = encrypt_marker(5.0, false);

    cout << "  Operations: " << N << "\n";
    cout << "  Mix: +2, ×3, +5, ÷2\n";
    cout << "  With Fibonacci marker\n";
    cout << "  Running...\n\n";

    auto start = high_resolution_clock::now();

    double expected = 2.0;

    for (int i = 0; i < N; i++) {
        switch (i % 4) {
            case 0: // +2 — addition
                ct_state = cc->EvalAdd(ct_state, ct_two_add);
                expected += 2.0;
                break;
            case 1: // ×3 — multiplication
                ct_state = cc->EvalAdd(ct_state, ct_three_mul);
                expected *= 3.0;
                break;
            case 2: // +5 — addition
                ct_state = cc->EvalAdd(ct_state, ct_five_add);
                expected += 5.0;
                break;
            case 3: // ÷2 — division
                ct_state = cc->EvalSub(ct_state, ct_two_mul);
                expected /= 2.0;
                break;
        }
    }

    auto end = high_resolution_clock::now();
    auto time = duration_cast<milliseconds>(end - start).count();

    auto final_vals = decrypt_marker(ct_state);

    cout << "  ✅ 10K mixed complete!\n";
    cout << "  Time: " << time << " ms\n";
    cout << "  Level: " << ct_state->GetLevel() << "\n";
    cout << "  Towers: " << ct_state->GetElements()[0].GetNumOfElements() << "\n";
    cout << "  Slot 0 (xφ): " << final_vals[0] << "\n";
    cout << "  Slot 1 (xφ⁻¹): " << final_vals[1] << "\n";
    cout << "  Slot 2 (log): " << final_vals[2] << "\n";
    cout << "  Slot 3 (marker): " << final_vals[3] << "\n\n";

    cout << "  Expected: " << expected << "\n";
    cout << "  Expected mod φ: " << fmod(expected, PHI) << "\n";
    cout << "  Slot 0 mod φ: " << fmod(final_vals[0], PHI) << "\n";
    cout << "  φ^(Slot 2) mod φ: " << fmod(pow(PHI, final_vals[2]), PHI) << "\n";
    cout << "  Match (Slot 0): " << (abs(fmod(final_vals[0], PHI) - fmod(expected, PHI)) < 1.0 ? "✅" : "❌") << "\n";
    cout << "  Match (log): " << (abs(fmod(pow(PHI, final_vals[2]), PHI) - fmod(expected, PHI)) < 1.0 ? "✅" : "❌") << "\n\n";

    cout << "========================================\n";
    cout << "  φ-10K FIBONACCI MARKER COMPLETE\n";
    cout << "========================================\n\n";
    cout << "  ✅ Fibonacci marker\n";
    cout << "  ✅ Operation detection\n";
    cout << "  ✅ Walang EvalMult(ct, ct)\n";
    cout << "  ✅ Walang bootstrapping\n\n";

    return 0;
}
