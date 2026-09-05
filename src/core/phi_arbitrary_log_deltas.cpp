// ============================================
// φ-ARBITRARY LOG DELTAS — 10K
// Hindi lang ±1 — arbitrary φ-multiplications
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

    cout << "========================================\n";
    cout << "  φ-ARBITRARY LOG DELTAS — 10K\n";
    cout << "========================================\n\n";

    int N = 10000;

    cout << "  Operations: " << N << "\n";
    cout << "  Deltas: log_φ(2), log_φ(3), log_φ(1.5)\n";
    cout << "  Arbitrary φ-multiplications\n";
    cout << "  Running...\n\n";

    auto ct_state = encrypt_log(3.0);

    // Arbitrary deltas
    auto ct_delta2 = encrypt_log(log(2.0) / LN_PHI);      // ×2
    auto ct_delta3 = encrypt_log(log(3.0) / LN_PHI);      // ×3
    auto ct_delta15 = encrypt_log(log(1.5) / LN_PHI);     // ×1.5
    auto ct_delta_neg = encrypt_log(-log(2.5) / LN_PHI);  // ÷2.5

    auto start = high_resolution_clock::now();

    for (int i = 0; i < N; i++) {
        int op = i % 4;
        
        if (op == 0) {
            ct_state = cc->EvalAdd(ct_state, ct_delta2);
        } else if (op == 1) {
            ct_state = cc->EvalAdd(ct_state, ct_delta3);
        } else if (op == 2) {
            ct_state = cc->EvalAdd(ct_state, ct_delta15);
        } else {
            ct_state = cc->EvalAdd(ct_state, ct_delta_neg);
        }
    }

    auto end = high_resolution_clock::now();
    auto time = duration_cast<milliseconds>(end - start).count();

    auto v_final = decrypt_log(ct_state);

    cout << "  ✅ Chain complete!\n";
    cout << "  Time: " << time << " ms\n";
    cout << "  Ops/sec: " << fixed << setprecision(0) << (N * 1000.0 / time) << "\n\n";
    cout << "  Final log: " << v_final[0] << "\n";
    cout << "  Final F: " << v_final[1] << "\n";
    cout << "  Level: " << ct_state->GetLevel() << "\n\n";

    return 0;
}
