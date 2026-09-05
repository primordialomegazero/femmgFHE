// ============================================
// φ-ARBITRARY BRIDGE FHE — 100 iterations
// Arbitrary values sa iisang φ-space
// EvalAdd = multiply, EvalSub = divide
// EvalAdd ng correction = addition sa normal
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
    parameters.SetBatchSize(2);
    parameters.SetSecurityLevel(HEStd_128_classic);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(parameters);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);
    auto keyPair = cc->KeyGen();

    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);

    auto encrypt_log = [&](double value) {
        double log_val = log(value) / LN_PHI;
        vector<double> v(2, 0.0);
        v[0] = log_val;
        v[1] = value;
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto decrypt_log = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext pt;
        cc->Decrypt(keyPair.secretKey, ct, &pt);
        pt->SetLength(2);
        auto res = pt->GetCKKSPackedValue();
        return vector<double>{res[0].real(), res[1].real()};
    };

    cout << "========================================\n";
    cout << "  φ-ARBITRARY BRIDGE FHE — 100 iterations\n";
    cout << "========================================\n\n";
    cout << "  Slot 0: log_φ(value)\n";
    cout << "  Slot 1: value (reference)\n";
    cout << "  Operations: ×2, ÷3, +5, -2\n\n";

    // Start sa value = 10
    auto ct_state = encrypt_log(10.0);

    // Deltas sa log space:
    double delta_mul2 = log(2.0) / LN_PHI;    // ×2
    double delta_div3 = log(3.0) / LN_PHI;    // ÷3
    double delta_add5 = log(1.0 + 5.0 / 10.0) / LN_PHI;  // +5 sa normal (correction)
    double delta_sub2 = log(1.0 - 2.0 / 10.0) / LN_PHI;  // -2 sa normal (correction)
    
    vector<double> deltas = {delta_mul2, delta_div3, delta_add5, delta_sub2};
    string op_names[] = {"×2", "÷3", "+5", "-2"};

    cout << "  Initial value: 10\n";
    cout << "  Expected: ((10 × 2 ÷ 3) + 5 - 2) × 2...\n\n";

    auto start = high_resolution_clock::now();

    for (int i = 0; i < 100; i++) {
        int op_idx = i % 4;
        vector<double> delta_v(2, 0.0);
        delta_v[0] = deltas[op_idx];
        Plaintext pt_delta = cc->MakeCKKSPackedPlaintext(delta_v);
        
        if (op_idx < 2) {
            // ×2 o ÷3 — log space EvalAdd/EvalSub
            if (op_idx == 0) {
                ct_state = cc->EvalAdd(ct_state, pt_delta);
            } else {
                ct_state = cc->EvalSub(ct_state, pt_delta);
            }
        } else {
            // +5 o -2 — correction sa log
            ct_state = cc->EvalAdd(ct_state, pt_delta);
        }
        
        if (i < 10 || i % 20 == 0) {
            auto v = decrypt_log(ct_state);
            cout << "  Step " << setw(3) << i << " [" << op_names[op_idx] << "]: "
                 << "log=" << setw(8) << v[0] 
                 << ", value=" << setw(10) << pow(PHI, v[0]) << "\n";
        }
    }

    auto end = high_resolution_clock::now();
    auto time = duration_cast<milliseconds>(end - start).count();

    auto v_final = decrypt_log(ct_state);
    double final_value = pow(PHI, v_final[0]);

    cout << "\n  ✅ Complete!\n";
    cout << "  Time: " << time << " ms\n\n";
    cout << "  Final log: " << v_final[0] << "\n";
    cout << "  Final value: " << final_value << "\n";
    cout << "  Level: " << ct_state->GetLevel() << "\n";

    return 0;
}
