// ============================================
// φ-RULE 110 META FHE — Walang Collision
// Weights: φ⁴, φ², 1 — unique sa lahat ng 8 patterns
// EvalLinearWSum para sa weighted sum
// Walang EvalMult sa transition
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
    parameters.SetMultiplicativeDepth(2);
    parameters.SetScalingModSize(59);
    parameters.SetBatchSize(8);
    parameters.SetSecurityLevel(HEStd_128_classic);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(parameters);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);
    cc->Enable(ADVANCEDSHE);
    auto keyPair = cc->KeyGen();
    cc->EvalMultKeyGen(keyPair.secretKey);
    cc->EvalRotateKeyGen(keyPair.secretKey, {1, -1});

    const double PHI = 1.6180339887498948482;
    const double W_L = pow(PHI, 4);
    const double W_C = pow(PHI, 2);
    const double W_R = 1.0;

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
    cout << "  φ-RULE 110 META FHE — Walang Collision\n";
    cout << "========================================\n\n";
    cout << "  Weights: L=φ⁴, C=φ², R=1\n";
    cout << "  Unique sa lahat ng 8 patterns\n\n";

    // Initial: 11010101 — parity ±1
    vector<double> init = {1.0, 1.0, -1.0, 1.0, -1.0, 1.0, -1.0, 1.0};
    Plaintext pt_init = cc->MakeCKKSPackedPlaintext(init);
    auto ct_state = cc->Encrypt(keyPair.publicKey, pt_init);

    cout << "  Initial: 11010101\n\n";

    int N = 10;

    auto start = high_resolution_clock::now();

    for (int step = 0; step < N; step++) {
        auto ct_left = cc->EvalRotate(ct_state, -1);
        auto ct_right = cc->EvalRotate(ct_state, 1);

        vector<ReadOnlyCiphertext<DCRTPoly>> cts;
        cts.push_back(ct_left);
        cts.push_back(ct_state);
        cts.push_back(ct_right);
        
        vector<double> weights = {W_L, W_C, W_R};
        auto ct_wsum = cc->EvalLinearWSum(cts, weights);
        
        ct_state = ct_wsum;
    }

    auto end = high_resolution_clock::now();
    auto time = duration_cast<milliseconds>(end - start).count();

    auto v_final = decrypt_state(ct_state);
    
    cout << "  Final state (" << N << " steps):\n  ";
    for (int i = 0; i < 8; i++) {
        cout << setw(10) << v_final[i];
    }
    cout << "\n\n";
    cout << "  Time: " << time << " ms\n";
    cout << "  Steps/sec: " << (N * 1000.0) / time << "\n";
    cout << "  Level: " << ct_state->GetLevel() << "\n";

    return 0;
}
