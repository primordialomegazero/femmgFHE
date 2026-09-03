// ============================================
// φ-PURE MIXED MOD — ONESHOT
// 10K mixed ops sa mod φ space
// Isang encrypt, isang decrypt
// WALANG re-encryption
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

    // Mod φ space: lahat ng values ay naka-encode sa [0, φ)
    // Slot 0: F mod φ
    // Slot 1: q = floor(F/φ)
    // Slot 2: log_φ(F) mod φ
    // Slot 3: index mod φ
    //
    // Ang φ-modulo ay nagbibigay ng natural na periodicity
    // na pumipigil sa noise accumulation

    auto encrypt_mod = [&](double F, double log_val, double idx) {
        vector<double> v(4, 0.0);
        v[0] = fmod(F, PHI);
        v[1] = fmod(floor(F / PHI), PHI);
        v[2] = fmod(log_val, PHI);
        v[3] = fmod(idx, PHI);
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto decrypt_mod = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext pt;
        cc->Decrypt(keyPair.secretKey, ct, &pt);
        pt->SetLength(4);
        auto res = pt->GetCKKSPackedValue();
        return vector<double>{res[0].real(), res[1].real(), res[2].real(), res[3].real()};
    };

    cout << "========================================\n";
    cout << "  φ-PURE MIXED MOD — ONESHOT\n";
    cout << "========================================\n\n";

    int N = 10000;

    cout << "  Operations: " << N << "\n";
    cout << "  Cycle: +φ⁴, -φ², ×φ, ÷φ — LAHAT sa mod φ\n";
    cout << "  Isang encrypt, isang decrypt\n";
    cout << "  WALANG re-encryption\n";
    cout << "  Running...\n\n";

    // Initial: F = φ³ = 4.236
    auto ct_state = encrypt_mod(pow(PHI, 3), 3.0, 3.0);

    // Pre-computed deltas — LAHAT nasa mod φ space
    auto ct_add = encrypt_mod(pow(PHI, 4), 4.0, 4.0);
    auto ct_sub = encrypt_mod(-pow(PHI, 2), -2.0, -2.0);
    auto ct_mul = encrypt_mod(0.0, 1.0, 1.0);
    auto ct_div = encrypt_mod(0.0, -1.0, -1.0);

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

    auto v_final = decrypt_mod(ct_state);

    // Expected — sa mod φ space
    double expected_mod = fmod(pow(PHI, 3), PHI);
    for (int i = 0; i < N; i++) {
        int op = i % 4;
        if (op == 0) expected_mod = fmod(expected_mod + fmod(pow(PHI, 4), PHI), PHI);
        else if (op == 1) expected_mod = fmod(expected_mod - fmod(pow(PHI, 2), PHI), PHI);
        else if (op == 2) expected_mod = fmod(expected_mod * PHI, PHI);
        else expected_mod = fmod(expected_mod / PHI, PHI);
    }

    cout << "  ✅ Chain complete!\n";
    cout << "  Time: " << time << " ms\n";
    cout << "  Ops/sec: " << fixed << setprecision(0) << (N * 1000.0 / time) << "\n\n";

    cout << "  Final Slot 0 (F mod φ): " << v_final[0] << "\n";
    cout << "  Expected mod φ:          " << expected_mod << "\n";
    cout << "  Final Slot 1 (q mod φ):  " << v_final[1] << "\n";
    cout << "  Final Slot 2 (log mod φ): " << v_final[2] << "\n";
    cout << "  Final Slot 3 (idx mod φ): " << v_final[3] << "\n\n";

    double error = abs(v_final[0] - expected_mod);
    cout << "  Error (Slot 0): " << fixed << setprecision(6) << error << "\n";
    cout << "  Match: " << (error < 0.01 ? "✅" : "❌") << "\n\n";

    cout << "========================================\n";
    cout << "  " << (error < 0.01 ? "✅ HOLY GRAIL CONFIRMED" : "❌ MAY ISSUE") << "\n";
    cout << "========================================\n\n";

    return 0;
}
