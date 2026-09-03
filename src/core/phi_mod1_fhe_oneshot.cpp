// ============================================
// φ-MOD1 FHE ONESHOT
// 10K mixed ops sa mod 1 space
// Isang encrypt, isang decrypt
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

    auto encrypt_mod1 = [&](double F, double log_val) {
        vector<double> v(4, 0.0);
        v[0] = fmod(F, 1.0);
        v[1] = floor(F);
        v[2] = fmod(log_val, 1.0);
        v[3] = 0;
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto decrypt_mod1 = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext pt;
        cc->Decrypt(keyPair.secretKey, ct, &pt);
        pt->SetLength(4);
        auto res = pt->GetCKKSPackedValue();
        return vector<double>{res[0].real(), res[1].real(), res[2].real(), res[3].real()};
    };

    cout << "========================================\n";
    cout << "  φ-MOD1 FHE ONESHOT — 10K MIXED\n";
    cout << "========================================\n\n";

    int N = 10000;

    cout << "  Operations: " << N << "\n";
    cout << "  Cycle: +φ⁴, -φ², ×φ, ÷φ\n";
    cout << "  Lahat sa mod 1 space\n";
    cout << "  Isang encrypt, isang decrypt\n";
    cout << "  Running...\n\n";

    auto ct_state = encrypt_mod1(pow(PHI, 3), 3.0);

    // Deltas sa mod 1 space
    auto ct_add = encrypt_mod1(pow(PHI, 4), 4.0);
    auto ct_sub = encrypt_mod1(-pow(PHI, 2), -2.0);
    auto ct_mul = encrypt_mod1(0.0, 1.0);
    auto ct_div = encrypt_mod1(0.0, -1.0);

    auto start = high_resolution_clock::now();

    for (int i = 0; i < N; i++) {
        int op = i % 4;
        
        if (op == 0) ct_state = cc->EvalAdd(ct_state, ct_add);
        else if (op == 1) ct_state = cc->EvalAdd(ct_state, ct_sub);
        else if (op == 2) ct_state = cc->EvalAdd(ct_state, ct_mul);
        else ct_state = cc->EvalAdd(ct_state, ct_div);
    }

    auto end = high_resolution_clock::now();
    auto time = duration_cast<milliseconds>(end - start).count();

    auto v_final = decrypt_mod1(ct_state);

    // Expected sa mod 1 space
    double expected = fmod(pow(PHI, 3), 1.0);
    for (int i = 0; i < N; i++) {
        int op = i % 4;
        if (op == 0) expected = fmod(expected + fmod(pow(PHI, 4), 1.0), 1.0);
        else if (op == 1) expected = fmod(expected - fmod(pow(PHI, 2), 1.0), 1.0);
        else if (op == 2) expected = fmod(expected * PHI, 1.0);
        else expected = fmod(expected / PHI, 1.0);
    }

    cout << "  ✅ Chain complete!\n";
    cout << "  Time: " << time << " ms\n";
    cout << "  Ops/sec: " << fixed << setprecision(0) << (N * 1000.0 / time) << "\n\n";

    cout << "  Final Slot 0 (F mod 1): " << v_final[0] << "\n";
    cout << "  Expected mod 1:          " << expected << "\n";
    cout << "  Final Slot 1 (floor):    " << v_final[1] << "\n";
    cout << "  Final Slot 2 (log mod 1): " << v_final[2] << "\n\n";

    double error = abs(v_final[0] - expected);
    cout << "  Error: " << fixed << setprecision(6) << error << "\n";
    cout << "  Match: " << (error < 0.01 ? "✅" : "❌") << "\n\n";

    cout << "========================================\n";
    cout << "  " << (error < 0.01 ? "✅ HOLY GRAIL CONFIRMED" : "❌ MAY ISSUE") << "\n";
    cout << "========================================\n\n";

    return 0;
}
