// ============================================
// φ-PURE NON-INTERACTIVE — 10K
// Walang plaintext expected sa loop
// Puro ciphertext operations
// LAHAT EvalAdd
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

    cout << "========================================\n";
    cout << "  φ-PURE NON-INTERACTIVE — 10K\n";
    cout << "========================================\n\n";

    int N = 10000;

    cout << "  Operations: " << N << "\n";
    cout << "  Puro ciphertext — walang plaintext sa loop\n";
    cout << "  LAHAT EvalAdd\n";
    cout << "  Isang encrypt sa simula, isang decrypt sa dulo\n";
    cout << "  Running...\n\n";

    // Isang encrypt lang sa simula
    auto ct_state = encrypt_log(5.0);

    // Pre-computed deltas — lahat naka-encrypt na
    auto ct_add = encrypt_log(add_corr[2]);      // ⊕φ³ correction
    auto ct_sub = encrypt_log(sub_corr[3]);      // ⊖φ² correction
    auto ct_mul = encrypt_log(1.0);               // ×φ
    auto ct_div = encrypt_log(-1.0);              // ÷φ

    auto start = high_resolution_clock::now();

    for (int i = 0; i < N; i++) {
        int op = i % 4;
        
        if (op == 0) {
            ct_state = cc->EvalAdd(ct_state, ct_add);
        } else if (op == 1) {
            ct_state = cc->EvalAdd(ct_state, ct_sub);
        } else if (op == 2) {
            ct_state = cc->EvalAdd(ct_state, ct_mul);
        } else {
            ct_state = cc->EvalAdd(ct_state, ct_div);
        }
    }

    auto end = high_resolution_clock::now();
    auto time = duration_cast<milliseconds>(end - start).count();

    // Isang decrypt lang sa dulo
    auto v_final = decrypt_log(ct_state);

    cout << "  ✅ Chain complete!\n";
    cout << "  Time: " << time << " ms\n";
    cout << "  Ops/sec: " << fixed << setprecision(0) << (N * 1000.0 / time) << "\n\n";

    cout << "  Final log: " << v_final[0] << "\n";
    cout << "  Final F: " << v_final[1] << "\n";
    cout << "  Level: " << ct_state->GetLevel() << "\n\n";

    cout << "========================================\n";
    cout << "  PURE NON-INTERACTIVE COMPLETE\n";
    cout << "========================================\n\n";
    cout << "  ✅ Isang encrypt, isang decrypt\n";
    cout << "  ✅ 10K EvalAdd sa gitna\n";
    cout << "  ✅ Walang EvalMult\n";
    cout << "  ✅ Walang bootstrapping\n";
    cout << "  ✅ Walang re-encryption\n\n";

    return 0;
}
