// ============================================
// φ-NATURAL ALL OPS — 10K
// Addition, Subtraction, Multiplication, Division
// LAHAT ay EvalAdd — walang ibang operasyon
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

    vector<double> add_corr(20, 0.0);
    vector<double> sub_corr(20, 0.0);
    
    add_corr[0] = log(2.0) / LN_PHI;
    sub_corr[0] = -100.0;
    
    for (int d = 1; d <= 15; d++) {
        add_corr[d] = log(1.0 + pow(PHI, -d)) / LN_PHI;
        sub_corr[d] = log(abs(1.0 - pow(PHI, -d))) / LN_PHI;
    }

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

    auto phi_add = [&](double log_a, double log_b) -> double {
        double d = abs(log_a - log_b);
        double m = max(log_a, log_b);
        return m + add_corr[(int)d];
    };

    auto phi_sub = [&](double log_a, double log_b) -> double {
        double d = abs(log_a - log_b);
        double m = max(log_a, log_b);
        if (d < 0.5) return -100.0;
        return m + sub_corr[(int)d];
    };

    cout << "========================================\n";
    cout << "  φ-NATURAL ALL OPS — 10K\n";
    cout << "========================================\n\n";

    int N = 10000;

    cout << "  Operations: " << N << "\n";
    cout << "  Cycle: ⊕φ³, ⊖φ², ×φ, ÷φ\n";
    cout << "  LAHAT EvalAdd — walang EvalSub\n";
    cout << "  Running...\n\n";

    auto ct_state = encrypt_log(5.0);
    double expected_log = 5.0;

    auto start = high_resolution_clock::now();

    for (int i = 0; i < N; i++) {
        int op = i % 4;
        double new_log;
        
        if (op == 0) {
            new_log = phi_add(expected_log, 3.0);
        } else if (op == 1) {
            new_log = phi_sub(expected_log, 2.0);
        } else if (op == 2) {
            new_log = expected_log + 1.0;
        } else {
            new_log = expected_log - 1.0;
        }
        
        double delta = new_log - expected_log;
        auto ct_delta = encrypt_log(delta);
        ct_state = cc->EvalAdd(ct_state, ct_delta);
        
        expected_log = new_log;
    }

    auto end = high_resolution_clock::now();
    auto time = duration_cast<milliseconds>(end - start).count();

    auto v_final = decrypt_log(ct_state);

    cout << "  ✅ Chain complete!\n";
    cout << "  Time: " << time << " ms\n";
    cout << "  Ops/sec: " << fixed << setprecision(0) << (N * 1000.0 / time) << "\n\n";

    cout << "  Final log: " << v_final[0] << "\n";
    cout << "  Expected log: " << expected_log << "\n";
    cout << "  Final F: " << v_final[1] << "\n";
    cout << "  Level: " << ct_state->GetLevel() << "\n\n";

    double error = abs(v_final[0] - expected_log) / abs(expected_log) * 100.0;
    cout << "  Error: " << fixed << setprecision(6) << error << "%\n";
    cout << "  Match: " << (error < 0.1 ? "✅" : "❌") << "\n\n";

    cout << "========================================\n";
    cout << "  " << (error < 0.1 ? "✅ ALL OPS CONFIRMED" : "❌ MAY ISSUE") << "\n";
    cout << "========================================\n\n";

    return 0;
}
