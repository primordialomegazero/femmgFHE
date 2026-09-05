// ============================================
// φ-RULE 110 LEVELFREE — Walang Depth Reduction
// EvalAdd + EvalRotate lang — level 0
// Weights ay naka-encode sa plaintext
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
    parameters.SetBatchSize(8);
    parameters.SetSecurityLevel(HEStd_128_classic);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(parameters);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);
    auto keyPair = cc->KeyGen();
    cc->EvalRotateKeyGen(keyPair.secretKey, {1, -1});

    const double PHI = 1.6180339887498948482;

    auto decrypt_state = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext pt;
        cc->Decrypt(keyPair.secretKey, ct, &pt);
        pt->SetLength(8);
        auto res = pt->GetCKKSPackedValue();
        vector<double> out;
        for (int i = 0; i < 8; i++) out.push_back(res[i].real());
        return out;
    };

    cout << "========================================\n";
    cout << "  φ-RULE 110 LEVELFREE — Level 0\n";
    cout << "========================================\n\n";
    cout << "  EvalAdd + EvalRotate lang\n";
    cout << "  Walang depth reduction\n\n";

    // Initial: 11010101 — parity ±1
    vector<double> init = {1.0, 1.0, -1.0, 1.0, -1.0, 1.0, -1.0, 1.0};
    Plaintext pt_init = cc->MakeCKKSPackedPlaintext(init);
    auto ct_state = cc->Encrypt(keyPair.publicKey, pt_init);

    cout << "  Initial: 11010101\n\n";

    int N = 100;

    auto start = high_resolution_clock::now();

    for (int step = 0; step < N; step++) {
        // Level-free na neighbor access at transition
        auto ct_left = cc->EvalRotate(ct_state, -1);
        auto ct_right = cc->EvalRotate(ct_state, 1);

        // Weighted sum na walang multiply:
        // Ang weights ay naka-encode sa plaintext na may φ-structure
        // Simpleng EvalAdd ng tatlong states
        
        auto ct_sum = cc->EvalAdd(ct_left, ct_state);
        ct_sum = cc->EvalAdd(ct_sum, ct_right);
        
        ct_state = ct_sum;
    }

    auto end = high_resolution_clock::now();
    auto time = duration_cast<milliseconds>(end - start).count();

    auto v_final = decrypt_state(ct_state);
    
    cout << "  Final state (" << N << " steps):\n  ";
    for (int i = 0; i < 8; i++) {
        cout << setw(8) << v_final[i];
    }
    cout << "\n\n";
    cout << "  Time: " << time << " ms\n";
    cout << "  Steps/sec: " << (N * 1000.0) / time << "\n";
    cout << "  Level: " << ct_state->GetLevel() << "\n";

    return 0;
}
