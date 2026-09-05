// ============================================
// φ-ALL FREE FUNCTIONS — 1K
// EvalAdd + EvalSub + EvalMult(constant) + EvalRotate
// Lahat ng libreng CKKS functions
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
    parameters.SetBatchSize(4);
    parameters.SetSecurityLevel(HEStd_128_classic);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(parameters);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);
    auto keyPair = cc->KeyGen();
    cc->EvalRotateKeyGen(keyPair.secretKey, {1, -1, 2, -2});
    cc->EvalMultKeyGen(keyPair.secretKey);

    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);
    const double PHI_INV = 1.0 / PHI;

    auto encrypt_state = [&](double val) {
        vector<double> v(4, 0.0);
        v[0] = val;                        // raw value
        v[1] = fmod(val, PHI);            // φ-mod
        v[2] = fmod(val + 1, PHI);       // shifted φ-mod
        v[3] = fmod(val - 1, PHI);       // shifted pabalik
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
    cout << "  φ-ALL FREE FUNCTIONS — 1K\n";
    cout << "========================================\n\n";

    int N = 1000;

    cout << "  Operations: " << N << "\n";
    cout << "  EvalAdd + EvalSub + EvalMult(φ⁻¹) + EvalRotate\n";
    cout << "  Lahat ng free functions\n";
    cout << "  Running...\n\n";

    auto ct_state = encrypt_state(1.0);
    
    // Pre-computed constants
    vector<double> v_phi_inv(4, 0.0);
    v_phi_inv[0] = PHI_INV;
    v_phi_inv[1] = fmod(PHI_INV, PHI);
    v_phi_inv[2] = fmod(PHI_INV + 1, PHI);
    v_phi_inv[3] = fmod(PHI_INV - 1, PHI);
    Plaintext pt_phi_inv = cc->MakeCKKSPackedPlaintext(v_phi_inv);
    auto ct_phi_inv = cc->Encrypt(keyPair.publicKey, pt_phi_inv);

    auto start = high_resolution_clock::now();

    for (int i = 0; i < N; i++) {
        int op = i % 4;
        
        if (op == 0) {
            // EvalMult sa φ⁻¹ para sa periodic scaling
            ct_state = cc->EvalMult(ct_state, ct_phi_inv);
        } else if (op == 1) {
            // EvalRotate para sa φ-shifted na access
            auto ct_rot = cc->EvalRotate(ct_state, 1);
            ct_state = cc->EvalAdd(ct_state, ct_rot);
        } else if (op == 2) {
            // EvalSub ng φ-mod na value
            auto ct_rot_back = cc->EvalRotate(ct_state, -1);
            ct_state = cc->EvalSub(ct_state, ct_rot_back);
        } else {
            // EvalAdd ng shifted
            auto ct_rot2 = cc->EvalRotate(ct_state, 2);
            ct_state = cc->EvalAdd(ct_state, ct_rot2);
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
