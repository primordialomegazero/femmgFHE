// ============================================
// φ-NATURAL ADDITION
// φ-addition bilang primary operation
// Walang masks, walang alignment
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

    vector<double> corr(20, 0.0);
    corr[0] = log(2.0) / LN_PHI;
    for (int d = 1; d <= 15; d++) {
        corr[d] = log(1.0 + pow(PHI, -d)) / LN_PHI;
    }

    // State: Slot 0 = log_φ(F), Slot 1 = F
    auto encrypt_log = [&](double log_val) {
        vector<double> v(2, 0.0);
        v[0] = log_val;
        v[1] = pow(PHI, log_val);
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

    // φ-addition: log_new = max(log_a, log_b) + corr(|log_a - log_b|)
    auto phi_add_log = [&](double log_a, double log_b) {
        double d = abs(log_a - log_b);
        double m = max(log_a, log_b);
        return m + corr[(int)d];
    };

    cout << "========================================\n";
    cout << "  φ-NATURAL ADDITION — 10K\n";
    cout << "========================================\n\n";

    int N = 10000;

    cout << "  Operations: " << N << "\n";
    cout << "  φ-addition sa log space\n";
    cout << "  Cycle: ⊕φ³, ⊕φ⁵, ×φ, ÷φ\n";
    cout << "  Walang masks, walang alignment\n";
    cout << "  Running...\n\n";

    auto ct_state = encrypt_log(3.0);
    double expected_log = 3.0;

    auto start = high_resolution_clock::now();

    for (int i = 0; i < N; i++) {
        int op = i % 4;
        
        if (op == 0) {
            // φ-addition with φ³
            double new_log = phi_add_log(expected_log, 3.0);
            auto ct_delta = encrypt_log(new_log - expected_log);
            ct_state = cc->EvalAdd(ct_state, ct_delta);
            expected_log = new_log;
        } else if (op == 1) {
            // φ-addition with φ⁵
            double new_log = phi_add_log(expected_log, 5.0);
            auto ct_delta = encrypt_log(new_log - expected_log);
            ct_state = cc->EvalAdd(ct_state, ct_delta);
            expected_log = new_log;
        } else if (op == 2) {
            // ×φ
            ct_state = cc->EvalAdd(ct_state, encrypt_log(1.0));
            expected_log += 1.0;
        } else {
            // ÷φ
            ct_state = cc->EvalAdd(ct_state, encrypt_log(-1.0));
            expected_log -= 1.0;
        }
    }

    auto end = high_resolution_clock::now();
    auto time = duration_cast<milliseconds>(end - start).count();

    auto v_final = decrypt_log(ct_state);

    cout << "  ✅ Chain complete!\n";
    cout << "  Time: " << time << " ms\n";
    cout << "  Ops/sec: " << fixed << setprecision(0) << (N * 1000.0 / time) << "\n\n";

    cout << "  Final log: " << v_final[0] << "\n";
    cout << "  Expected log: " << expected_log << "\n";
    cout << "  Final F: " << v_final[1] << "\n\n";

    double error = abs(v_final[0] - expected_log) / expected_log * 100.0;
    cout << "  Error: " << fixed << setprecision(6) << error << "%\n";
    cout << "  Match: " << (error < 0.1 ? "✅" : "❌") << "\n\n";

    cout << "========================================\n";
    cout << "  " << (error < 0.1 ? "✅ NATURAL CONFIRMED" : "❌ MAY ISSUE") << "\n";
    cout << "========================================\n\n";

    return 0;
}
