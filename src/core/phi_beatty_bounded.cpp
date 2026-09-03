// ============================================
// φ-BEATTY BOUNDED
// Beatty indexing na may bounded cycle
// +3, -1, ×1.5, ÷2 — net effect bounded
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

    auto encrypt_beatty = [&](int n) {
        double F = floor(n * PHI);
        vector<double> v(4, 0.0);
        v[0] = F;
        v[1] = n;
        v[2] = log((double)n) / LN_PHI + 1.0;
        v[3] = 0;
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto encrypt_mask_normal = [&](double F) {
        vector<double> v(4, 0.0);
        v[0] = F;
        v[1] = 0;
        v[2] = 0;
        v[3] = 0;
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto encrypt_mask_log = [&](double log_val) {
        vector<double> v(4, 0.0);
        v[0] = 0;
        v[1] = 0;
        v[2] = log_val;
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
    cout << "  φ-BEATTY BOUNDED\n";
    cout << "========================================\n\n";

    int N = 1000;

    cout << "  Operations: " << N << "\n";
    cout << "  Cycle: +3, -1, ×1.5, ÷2\n";
    cout << "  Net effect per cycle: +2, ×0.75\n";
    cout << "  Bounded — walang overflow\n\n";

    auto ct_state = encrypt_beatty(3);  // F = 4
    double expected = floor(3 * PHI);

    auto start = high_resolution_clock::now();

    for (int i = 0; i < N; i++) {
        int op = i % 4;
        
        if (op == 0) {
            // +3
            ct_state = cc->EvalAdd(ct_state, encrypt_mask_normal(3.0));
            expected += 3.0;
        } else if (op == 1) {
            // -1
            ct_state = cc->EvalSub(ct_state, encrypt_mask_normal(1.0));
            expected -= 1.0;
        } else if (op == 2) {
            // ×1.5
            ct_state = cc->EvalAdd(ct_state, encrypt_mask_log(log(1.5) / LN_PHI));
            expected *= 1.5;
        } else {
            // ÷2
            ct_state = cc->EvalSub(ct_state, encrypt_mask_log(log(2.0) / LN_PHI));
            expected /= 2.0;
        }
    }

    auto end = high_resolution_clock::now();
    auto time = duration_cast<milliseconds>(end - start).count();

    auto final_v = decrypt_full(ct_state);
    double final_result = final_v[0];

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
