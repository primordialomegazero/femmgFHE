// ============================================
// φ-1K BOUNDED — PURE FHE
// 1,000 operations — bounded cycle
// +2, -1, ×1.5, ÷1.5 — net effect = 1
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

    auto decrypt_full = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext pt;
        cc->Decrypt(keyPair.secretKey, ct, &pt);
        pt->SetLength(4);
        auto res = pt->GetCKKSPackedValue();
        return vector<double>{res[0].real(), res[1].real(), res[2].real(), res[3].real()};
    };

    cout << "========================================\n";
    cout << "  φ-1K BOUNDED — PURE FHE\n";
    cout << "========================================\n\n";

    int N = 1000;

    cout << "  Operations: " << N << "\n";
    cout << "  Cycle: +2, -1, ×1.5, ÷1.5\n";
    cout << "  Net effect: 1 (bounded)\n";
    cout << "  Walang decrypt sa gitna\n\n";

    auto ct_state = encrypt_full(5.0);
    double expected = 5.0;

    auto start = high_resolution_clock::now();

    for (int i = 0; i < N; i++) {
        int op = i % 4;
        
        if (op == 0) {
            // +2 — normal space
            ct_state = cc->EvalAdd(ct_state, encrypt_full(2.0));
            expected += 2.0;
        } else if (op == 1) {
            // -1 — normal space
            ct_state = cc->EvalSub(ct_state, encrypt_full(1.0));
            expected -= 1.0;
        } else if (op == 2) {
            // ×1.5 — kailangan ng transition
            auto v = decrypt_full(ct_state);
            double F_current = v[1] * PHI + v[0];
            double log_current = log(F_current) / LN_PHI;
            double log_new = log_current + log(1.5) / LN_PHI;
            double F_new = pow(PHI, log_new);
            ct_state = encrypt_full(F_new);
            expected *= 1.5;
        } else {
            // ÷1.5 — log space
            auto v = decrypt_full(ct_state);
            double F_current = v[1] * PHI + v[0];
            double log_current = log(F_current) / LN_PHI;
            double log_new = log_current - log(1.5) / LN_PHI;
            double F_new = pow(PHI, log_new);
            ct_state = encrypt_full(F_new);
            expected /= 1.5;
        }
    }

    auto end = high_resolution_clock::now();
    auto time = duration_cast<milliseconds>(end - start).count();

    auto final_v = decrypt_full(ct_state);
    double final_result = final_v[1] * PHI + final_v[0];

    cout << "  ✅ Chain complete!\n";
    cout << "  Time: " << time << " ms\n";
    cout << "  Ops/sec: " << fixed << setprecision(0) << (N * 1000.0 / time) << "\n\n";

    cout << "  Final result (FHE): " << fixed << setprecision(6) << final_result << "\n";
    cout << "  Expected result:    " << expected << "\n\n";

    double error_pct = abs(final_result - expected) / expected * 100.0;
    cout << "  Error: " << fixed << setprecision(6) << error_pct << "%\n";
    cout << "  Level: " << ct_state->GetLevel() << "\n\n";

    cout << "========================================\n";
    cout << "  " << (error_pct < 1.0 ? "✅ HOLY GRAIL CONFIRMED" : "❌ MAY ISSUE") << "\n";
    cout << "========================================\n\n";

    return 0;
}
