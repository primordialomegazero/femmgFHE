// ============================================
// φ-UNIVERSAL OPS — 100 iterations
// Mixed arbitrary operations sa iisang exponent
// φ ang nagbibigay ng mode
// Walang EvalMult, walang bootstrapping
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
    parameters.SetBatchSize(1);
    parameters.SetSecurityLevel(HEStd_128_classic);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(parameters);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);
    auto keyPair = cc->KeyGen();

    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);

    auto encrypt_n = [&](double n) {
        vector<double> v(1, n);
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto decrypt_n = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext pt;
        cc->Decrypt(keyPair.secretKey, ct, &pt);
        pt->SetLength(1);
        return pt->GetCKKSPackedValue()[0].real();
    };

    cout << "========================================\n";
    cout << "  φ-UNIVERSAL OPS — 20 iterations\n";
    cout << "========================================\n\n";
    cout << "  Mixed: ×2, ÷3, +5, -2\n";
    cout << "  Sa iisang exponent space\n";
    cout << "  φ ang nagbibigay ng mode\n\n";

    double n_start = log(10.0) / LN_PHI;
    auto ct_state = encrypt_n(n_start);

    cout << "  Initial: n = " << n_start << ", value = 10\n\n";

    double delta_mul2 = log(2.0) / LN_PHI;
    double delta_div3 = log(3.0) / LN_PHI;
    double delta_add5 = log(1.0 + 5.0 / 10.0) / LN_PHI;
    double delta_sub2 = log(1.0 - 2.0 / 10.0) / LN_PHI;

    vector<double> deltas = {delta_mul2, delta_div3, delta_add5, delta_sub2};
    string op_names[] = {"×2", "÷3", "+5", "-2"};

    auto start = high_resolution_clock::now();

    for (int i = 0; i < 20; i++) {
        int op_idx = i % 4;
        vector<double> delta_v(1, 0.0);
        delta_v[0] = deltas[op_idx];
        
        Plaintext pt_delta = cc->MakeCKKSPackedPlaintext(delta_v);
        
        if (op_idx == 1) {
            // ÷3 — EvalSub
            ct_state = cc->EvalSub(ct_state, pt_delta);
        } else {
            // ×2, +5, -2 — EvalAdd
            ct_state = cc->EvalAdd(ct_state, pt_delta);
        }
        
        if (i < 12) {
            double n = decrypt_n(ct_state);
            double value = pow(PHI, n);
            cout << "  Step " << setw(2) << i << " [" << op_names[op_idx] << "]: "
                 << "n=" << setw(8) << n
                 << ", value=" << setw(10) << value << "\n";
        }
    }

    auto end = high_resolution_clock::now();
    auto time = duration_cast<milliseconds>(end - start).count();

    double n_final = decrypt_n(ct_state);
    double value_final = pow(PHI, n_final);

    cout << "\n  ✅ Complete!\n";
    cout << "  Time: " << time << " ms\n\n";
    cout << "  Final n: " << n_final << "\n";
    cout << "  Final value: " << value_final << "\n";
    cout << "  Level: " << ct_state->GetLevel() << "\n";

    return 0;
}
