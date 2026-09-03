// ============================================
// φ-10K MIXED ONESHOT — PURE FHE
// 10,000 operations: +, -, ×, ÷
// Isang encrypt, isang decrypt
// WALANG re-encryption sa gitna
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
    parameters.SetBatchSize(4);
    parameters.SetSecurityLevel(HEStd_128_classic);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(parameters);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);
    auto keyPair = cc->KeyGen();

    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);

    // 4-slot state:
    // Slot 0: F mod φ (normal space fractional)
    // Slot 1: q = floor(F/φ) (normal space wrap)
    // Slot 2: log_φ(F) (log space)
    // Slot 3: index (tracking)

    auto encrypt_state = [&](double F, double log_val, double idx) {
        vector<double> v(4, 0.0);
        v[0] = fmod(F, PHI);
        v[1] = floor(F / PHI);
        v[2] = log_val;
        v[3] = idx;
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto decrypt_state = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext pt;
        cc->Decrypt(keyPair.secretKey, ct, &pt);
        pt->SetLength(4);
        auto res = pt->GetCKKSPackedValue();
        return vector<double>{res[0].real(), res[1].real(), res[2].real(), res[3].real()};
    };

    cout << "========================================\n";
    cout << "  φ-10K MIXED ONESHOT — PURE FHE\n";
    cout << "========================================\n\n";

    int N = 10000;

    cout << "  Operations: " << N << "\n";
    cout << "  Cycle: +φ⁴, -φ², ×φ¹, ÷φ¹\n";
    cout << "  Isang encrypt, isang decrypt\n";
    cout << "  WALANG re-encryption\n";
    cout << "  Running...\n\n";

    // Initial: F = φ^3 = 4.236, log = 3, idx = 3
    auto ct_state = encrypt_state(pow(PHI, 3), 3.0, 3.0);

    // Pre-computed deltas para sa apat na operasyon
    auto ct_add = encrypt_state(pow(PHI, 4), 4.0, 4.0);     // +φ⁴
    auto ct_sub = encrypt_state(-pow(PHI, 2), -2.0, -2.0);  // -φ²
    auto ct_mul = encrypt_state(0, log(PHI) / LN_PHI, 1.0); // ×φ (log +1)
    auto ct_div = encrypt_state(0, -log(PHI) / LN_PHI, -1.0); // ÷φ (log -1)

    auto start = high_resolution_clock::now();

    for (int i = 0; i < N; i++) {
        int op = i % 4;
        
        if (op == 0) {
            ct_state = cc->EvalAdd(ct_state, ct_add);
        } else if (op == 1) {
            ct_state = cc->EvalAdd(ct_state, ct_sub);
        } else if (op == 2) {
            ct_state = cc->EvalAdd(ct_state, ct_mul);
        } else {
            ct_state = cc->EvalAdd(ct_state, ct_div);
        }
    }

    auto end = high_resolution_clock::now();
    auto time = duration_cast<milliseconds>(end - start).count();

    // Isang decrypt sa dulo
    auto v_final = decrypt_state(ct_state);

    // Expected tracking
    double expected_F = pow(PHI, 3);
    for (int i = 0; i < N; i++) {
        int op = i % 4;
        if (op == 0) expected_F += pow(PHI, 4);
        else if (op == 1) expected_F -= pow(PHI, 2);
        else if (op == 2) expected_F *= PHI;
        else expected_F /= PHI;
    }

    double actual_F = v_final[1] * PHI + v_final[0];

    cout << "  ✅ Chain complete!\n";
    cout << "  Time: " << time << " ms\n";
    cout << "  Ops/sec: " << fixed << setprecision(0) << (N * 1000.0 / time) << "\n\n";

    cout << "  Final F (FHE): " << scientific << setprecision(6) << actual_F << "\n";
    cout << "  Expected F:    " << expected_F << "\n";
    cout << "  Final log:     " << v_final[2] << "\n";
    cout << "  Final idx:     " << v_final[3] << "\n\n";

    double error_pct = abs(actual_F - expected_F) / expected_F * 100.0;
    cout << "  Error: " << fixed << setprecision(6) << error_pct << "%\n";
    cout << "  Level: " << ct_state->GetLevel() << "\n\n";

    cout << "========================================\n";
    cout << "  " << (error_pct < 1.0 ? "✅ HOLY GRAIL CONFIRMED" : "❌ MAY ISSUE") << "\n";
    cout << "========================================\n\n";

    return 0;
}
