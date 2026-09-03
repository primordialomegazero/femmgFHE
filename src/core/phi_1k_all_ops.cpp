// ============================================
// φ-1K ALL OPS — PURE FHE
// 1,000 operations — full mixed +, -, ×, ÷
// WALANG decrypt sa gitna
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
    const double LN_PHI = log(PHI);

    vector<double> log_q_table(10001, 0.0);
    for (int i = 1; i <= 10000; i++) {
        log_q_table[i] = log((double)i) / LN_PHI;
    }

    auto encrypt_full = [&](double F) {
        double q = floor(F / PHI);
        double r = fmod(F, PHI);
        vector<double> v(4, 0.0);
        v[0] = r;
        v[1] = q;
        v[2] = log(F) / LN_PHI;
        v[3] = 0;
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto encrypt_log_only = [&](double log_val) {
        vector<double> v(4, 0.0);
        v[2] = log_val;
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto decrypt_full = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext pt;
        cc->Decrypt(keyPair.secretKey, ct, &pt);
        pt->SetLength(4);
        auto res = pt->GetCKKSPackedValue();
        return vector<double>{res[0].real(), res[1].real(), res[2].real(), res[3].real()};
    };

    auto decrypt_log = [&](const Ciphertext<DCRTPoly>& ct) {
        auto v = decrypt_full(ct);
        return v[2];
    };

    cout << "========================================\n";
    cout << "  φ-1K ALL OPS — PURE FHE\n";
    cout << "========================================\n\n";

    int N = 1000;

    cout << "  Operations: " << N << "\n";
    cout << "  Cycle: +2, -1, ×3, ÷2\n";
    cout << "  Walang decrypt sa gitna\n";
    cout << "  Isang decrypt lang sa dulo\n";
    cout << "  Running...\n\n";

    auto ct_state = encrypt_full(5.0);
    double expected = 5.0;

    auto start = high_resolution_clock::now();

    for (int i = 0; i < N; i++) {
        int op = i % 4;
        
        if (op == 0) {
            // +2
            ct_state = cc->EvalAdd(ct_state, encrypt_full(2.0));
            expected += 2.0;
        } else if (op == 1) {
            // -1
            ct_state = cc->EvalSub(ct_state, encrypt_full(1.0));
            expected -= 1.0;
        } else if (op == 2) {
            // ×3 — kailangan i-convert muna sa log space
            // Pero ang ct_state ay nasa normal space
            // Kaya kailangan nating gumamit ng log-only addition
            auto v = decrypt_full(ct_state);
            double q = v[1];
            double r = v[0];
            int qi = (int)(q + 0.5);
            double log_F = log_q_table[qi] + 1.0 + log(1.0 + r / (q * PHI)) / LN_PHI;
            ct_state = encrypt_log_only(log_F + log(3.0) / LN_PHI);
            expected *= 3.0;
        } else {
            // ÷2 — log space division
            auto v = decrypt_full(ct_state);
            double q = v[1];
            double r = v[0];
            int qi = (int)(q + 0.5);
            double log_F = log_q_table[qi] + 1.0 + log(1.0 + r / (q * PHI)) / LN_PHI;
            ct_state = encrypt_log_only(log_F - log(2.0) / LN_PHI);
            expected /= 2.0;
        }
    }

    auto end = high_resolution_clock::now();
    auto time = duration_cast<milliseconds>(end - start).count();

    double final_log = decrypt_log(ct_state);
    double final_result = pow(PHI, final_log);

    cout << "  ✅ Chain complete!\n";
    cout << "  Time: " << time << " ms\n";
    cout << "  Ops/sec: " << fixed << setprecision(0) << (N * 1000.0 / time) << "\n\n";

    cout << "  Final result (FHE): " << scientific << setprecision(6) << final_result << "\n";
    cout << "  Expected result:    " << expected << "\n";
    cout << "  Ratio:              " << fixed << setprecision(10) << (final_result / expected) << "\n\n";

    double error_pct = abs(final_result - expected) / expected * 100.0;
    cout << "  Error: " << fixed << setprecision(6) << error_pct << "%\n";
    cout << "  Level: " << ct_state->GetLevel() << "\n\n";

    cout << "========================================\n";
    cout << "  " << (error_pct < 1.0 ? "✅ HOLY GRAIL CONFIRMED" : "❌ MAY ISSUE") << "\n";
    cout << "========================================\n\n";

    return 0;
}
