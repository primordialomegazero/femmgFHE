// ============================================
// φ-PERIODIC RESET
// 10K mixed ops na may periodic mod φ reset
// Reset bawat 5 ops gamit ang pre-computed mask
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
    cout << "  φ-PERIODIC RESET\n";
    cout << "========================================\n\n";

    int N = 10000;
    int reset_interval = 5;

    cout << "  Operations: " << N << "\n";
    cout << "  Reset bawat " << reset_interval << " ops\n";
    cout << "  Cycle: +φ⁴, -φ², ×φ, ÷φ\n";
    cout << "  Running...\n\n";

    auto ct_state = encrypt_mod(pow(PHI, 3), 3.0, 3.0);

    // Deltas
    auto ct_add = encrypt_mod(pow(PHI, 4), 4.0, 4.0);
    auto ct_sub = encrypt_mod(-pow(PHI, 2), -2.0, -2.0);
    auto ct_mul = encrypt_mod(0.0, 1.0, 1.0);
    auto ct_div = encrypt_mod(0.0, -1.0, -1.0);

    // Pre-computed reset mask: ibalik sa mod φ range
    // Para sa Slot 0: -φ × floor(current_slot0 / φ)
    // Pero ito ay nangangailangan ng pag-alam sa current value
    // Subukan natin ang simpleng approximation:
    // Kung ang value ay lumampas ng φ, ibawas ang φ
    // Ito ay periodic reset na may φ interval

    auto start = high_resolution_clock::now();

    int ops_since_reset = 0;

    for (int i = 0; i < N; i++) {
        int op = i % 4;
        
        if (op == 0) ct_state = cc->EvalAdd(ct_state, ct_add);
        else if (op == 1) ct_state = cc->EvalAdd(ct_state, ct_sub);
        else if (op == 2) ct_state = cc->EvalAdd(ct_state, ct_mul);
        else ct_state = cc->EvalAdd(ct_state, ct_div);
        
        ops_since_reset++;
        
        if (ops_since_reset >= reset_interval) {
            // Periodic reset: hindi ito gumagana nang walang homomorphic mod
            // Subukan natin ang simpleng subtraction ng φ sa Slot 0
            auto ct_reset = encrypt_mod(-PHI, 0.0, 0.0);
            ct_state = cc->EvalAdd(ct_state, ct_reset);
            ops_since_reset = 0;
        }
    }

    auto end = high_resolution_clock::now();
    auto time = duration_cast<milliseconds>(end - start).count();

    auto v_final = decrypt_mod(ct_state);

    cout << "  ✅ Chain complete!\n";
    cout << "  Time: " << time << " ms\n";
    cout << "  Ops/sec: " << fixed << setprecision(0) << (N * 1000.0 / time) << "\n\n";

    cout << "  Final Slot 0: " << v_final[0] << "\n";
    cout << "  Final Slot 1: " << v_final[1] << "\n";
    cout << "  Final Slot 2: " << v_final[2] << "\n";
    cout << "  Final Slot 3: " << v_final[3] << "\n";
    cout << "  Level: " << ct_state->GetLevel() << "\n\n";

    cout << "========================================\n";
    cout << "  φ-PERIODIC RESET COMPLETE\n";
    cout << "========================================\n\n";

    return 0;
}
