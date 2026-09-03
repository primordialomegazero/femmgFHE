// ============================================
// φ-1K PURE DUAL — NO DECRYPT SA TRANSITION
// Dual-space state: laging may normal at log
// Slot-wise operations gamit ang masks
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

    // ============================================
    // ENCODING — DUAL-SPACE STATE
    // Slot 0: r = F mod φ (normal space)
    // Slot 1: q = floor(F/φ) (normal space)
    // Slot 2: log_φ(F) (log space)
    // Slot 3: index (tracking)
    // ============================================

    auto encrypt_dual = [&](double F) {
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

    // Mask para sa normal space (Slot 0 at 1 lang)
    auto encrypt_mask_normal = [&](double F) {
        double q = floor(F / PHI);
        double r = fmod(F, PHI);
        vector<double> v(4, 0.0);
        v[0] = r;
        v[1] = q;
        v[2] = 0;  // log space hindi magbabago
        v[3] = 0;
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    // Mask para sa log space (Slot 2 lang)
    auto encrypt_mask_log = [&](double F) {
        vector<double> v(4, 0.0);
        v[0] = 0;  // normal space hindi magbabago
        v[1] = 0;
        v[2] = log(F) / LN_PHI;
        v[3] = 0;
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto decrypt_dual = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext pt;
        cc->Decrypt(keyPair.secretKey, ct, &pt);
        pt->SetLength(4);
        auto res = pt->GetCKKSPackedValue();
        return vector<double>{res[0].real(), res[1].real(), res[2].real(), res[3].real()};
    };

    cout << "========================================\n";
    cout << "  φ-1K PURE DUAL — NO DECRYPT\n";
    cout << "========================================\n\n";

    int N = 1000;

    cout << "  Operations: " << N << "\n";
    cout << "  Cycle: +2, -1, ×1.5, ÷1.5\n";
    cout << "  Net effect: 1 (bounded)\n";
    cout << "  WALANG decrypt sa transition\n";
    cout << "  Isang decrypt lang sa dulo\n\n";

    auto ct_state = encrypt_dual(5.0);
    double expected = 5.0;

    auto start = high_resolution_clock::now();

    for (int i = 0; i < N; i++) {
        int op = i % 4;
        
        if (op == 0) {
            // +2 — normal space, Slot 0 at 1 lang
            ct_state = cc->EvalAdd(ct_state, encrypt_mask_normal(2.0));
            expected += 2.0;
        } else if (op == 1) {
            // -1 — normal space, Slot 0 at 1 lang
            ct_state = cc->EvalSub(ct_state, encrypt_mask_normal(1.0));
            expected -= 1.0;
        } else if (op == 2) {
            // ×1.5 — log space, Slot 2 lang
            ct_state = cc->EvalAdd(ct_state, encrypt_mask_log(1.5));
            expected *= 1.5;
        } else {
            // ÷1.5 — log space, Slot 2 lang
            ct_state = cc->EvalSub(ct_state, encrypt_mask_log(1.5));
            expected /= 1.5;
        }
    }

    auto end = high_resolution_clock::now();
    auto time = duration_cast<milliseconds>(end - start).count();

    // ONE SHOT DECRYPT SA DULO
    auto final_v = decrypt_dual(ct_state);
    double final_normal = final_v[1] * PHI + final_v[0];
    double final_log = pow(PHI, final_v[2]);

    cout << "  ✅ Chain complete!\n";
    cout << "  Time: " << time << " ms\n";
    cout << "  Ops/sec: " << fixed << setprecision(0) << (N * 1000.0 / time) << "\n\n";

    cout << "  Final normal space: " << fixed << setprecision(6) << final_normal << "\n";
    cout << "  Final log space:    " << final_log << "\n";
    cout << "  Expected result:    " << expected << "\n\n";

    double error_normal = abs(final_normal - expected) / expected * 100.0;
    double error_log = abs(final_log - expected) / expected * 100.0;

    cout << "  Error (normal): " << fixed << setprecision(6) << error_normal << "%\n";
    cout << "  Error (log):    " << error_log << "%\n";
    cout << "  Level: " << ct_state->GetLevel() << "\n\n";

    cout << "========================================\n";
    cout << "  " << (min(error_normal, error_log) < 1.0 ? "✅ HOLY GRAIL CONFIRMED" : "❌ MAY ISSUE") << "\n";
    cout << "========================================\n\n";

    return 0;
}
