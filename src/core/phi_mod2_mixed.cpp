// ============================================
// φ-MOD² MIXED
// Mixed ops sa mod φ² space
// Two-state reset: 0 at φ²
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
    const double PHI2 = PHI * PHI;

    auto encrypt_mod2 = [&](double F, double log_val, double idx) {
        vector<double> v(4, 0.0);
        v[0] = fmod(F, PHI2);
        v[1] = fmod(floor(F / PHI), PHI2);
        v[2] = fmod(log_val, PHI2);
        v[3] = fmod(idx, PHI2);
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto decrypt_mod2 = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext pt;
        cc->Decrypt(keyPair.secretKey, ct, &pt);
        pt->SetLength(4);
        auto res = pt->GetCKKSPackedValue();
        return vector<double>{res[0].real(), res[1].real(), res[2].real(), res[3].real()};
    };

    cout << "========================================\n";
    cout << "  φ-MOD² MIXED — ONESHOT\n";
    cout << "========================================\n\n";

    int N = 10000;

    cout << "  Operations: " << N << "\n";
    cout << "  Cycle: +φ⁴, -φ², ×φ, ÷φ\n";
    cout << "  Lahat sa mod φ² space\n";
    cout << "  Running...\n\n";

    auto ct_state = encrypt_mod2(pow(PHI, 3), 3.0, 3.0);

    auto ct_add = encrypt_mod2(pow(PHI, 4), 4.0, 4.0);
    auto ct_sub = encrypt_mod2(-pow(PHI, 2), -2.0, -2.0);
    auto ct_mul = encrypt_mod2(0.0, 1.0, 1.0);
    auto ct_div = encrypt_mod2(0.0, -1.0, -1.0);

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

    auto v_final = decrypt_mod2(ct_state);

    cout << "  ✅ Chain complete!\n";
    cout << "  Time: " << time << " ms\n";
    cout << "  Ops/sec: " << fixed << setprecision(0) << (N * 1000.0 / time) << "\n\n";

    cout << "  Final Slot 0 (F mod φ²): " << v_final[0] << "\n";
    cout << "  Final Slot 1 (q mod φ²): " << v_final[1] << "\n";
    cout << "  Final Slot 2 (log mod φ²): " << v_final[2] << "\n";
    cout << "  Final Slot 3 (idx mod φ²): " << v_final[3] << "\n";
    cout << "  Level: " << ct_state->GetLevel() << "\n\n";

    cout << "========================================\n";
    cout << "  φ-MOD² MIXED COMPLETE\n";
    cout << "========================================\n\n";

    return 0;
}
