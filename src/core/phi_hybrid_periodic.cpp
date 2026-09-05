// ============================================
// φ-HYBRID PERIODIC — MAY RESET
// EvalRotate + periodic φ-reset
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
    cc->EvalRotateKeyGen(keyPair.secretKey, {1, -1, 2, -2});

    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);

    auto encrypt_state = [&](double n, double F) {
        vector<double> v(4, 0.0);
        v[0] = fmod(n, PHI);       // mod φ
        v[1] = fmod(F, PHI);       // mod φ
        v[2] = fmod(n + 1, PHI);   // mod φ
        v[3] = fmod(F * PHI, PHI); // mod φ
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
    cout << "  φ-HYBRID PERIODIC — 1K\n";
    cout << "========================================\n\n";

    int N = 1000;
    int reset_interval = 5;

    cout << "  Operations: " << N << "\n";
    cout << "  Reset bawat " << reset_interval << " ops\n";
    cout << "  Lahat ng slots ay mod φ\n";
    cout << "  Running...\n\n";

    auto ct_state = encrypt_state(3.0, pow(PHI, 3.0));

    // Pre-computed reset delta: ibalik sa [0, φ)
    auto ct_reset = encrypt_state(-PHI, -PHI);

    auto start = high_resolution_clock::now();

    int ops_since_reset = 0;

    for (int i = 0; i < N; i++) {
        // EvalRotate at EvalAdd
        auto ct_rot = cc->EvalRotate(ct_state, 1);
        ct_state = cc->EvalAdd(ct_state, ct_rot);
        
        ops_since_reset++;
        
        if (ops_since_reset >= reset_interval) {
            // Periodic reset: ibawas ang φ sa lahat ng slots
            ct_state = cc->EvalAdd(ct_state, ct_reset);
            ops_since_reset = 0;
        }
    }

    auto end = high_resolution_clock::now();
    auto time = duration_cast<milliseconds>(end - start).count();

    auto v_final = decrypt_state(ct_state);

    cout << "  ✅ Chain complete!\n";
    cout << "  Time: " << time << " ms\n";
    cout << "  Ops/sec: " << fixed << setprecision(0) << (N * 1000.0 / time) << "\n\n";
    cout << "  Final Slot 0: " << v_final[0] << "\n";
    cout << "  Final Slot 1: " << v_final[1] << "\n";
    cout << "  Final Slot 2: " << v_final[2] << "\n";
    cout << "  Final Slot 3: " << v_final[3] << "\n";
    cout << "  Level: " << ct_state->GetLevel() << "\n\n";

    return 0;
}
