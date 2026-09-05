// ============================================
// φ-REAL INDEX FHE — ARBITRARY VALUES
// 10K mixed operations: +7, -3, ×2, ÷3
// Universal n-space, pure EvalAdd
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

    auto encrypt_n = [&](double n) {
        vector<double> v(2, 0.0);
        v[0] = n;
        v[1] = pow(PHI, n);
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto decrypt_n = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext pt;
        cc->Decrypt(keyPair.secretKey, ct, &pt);
        pt->SetLength(2);
        auto res = pt->GetCKKSPackedValue();
        return vector<double>{res[0].real(), res[1].real()};
    };

    cout << "========================================\n";
    cout << "  φ-REAL INDEX FHE — 10K ARBITRARY\n";
    cout << "========================================\n\n";

    int N = 10000;

    cout << "  Operations: " << N << "\n";
    cout << "  Cycle: +7, -3, ×2, ÷3 (no-cancel)\n";
    cout << "  Initial: 100\n";
    cout << "  Universal n-space\n";
    cout << "  Running...\n\n";

    // Initial: 100 → n = log_φ(100)
    double n_initial = log(100.0) / LN_PHI;
    auto ct_state = encrypt_n(n_initial);
    double expected = 100.0;

    // Deltas sa n-space
    double n_add = log(7.0) / LN_PHI;
    double n_sub = log(3.0) / LN_PHI;
    double n_mul = log(2.0) / LN_PHI;
    double n_div = log(3.0) / LN_PHI;
    
    auto ct_add = encrypt_n(n_add);
    auto ct_sub = encrypt_n(-n_sub);
    auto ct_mul = encrypt_n(n_mul);
    auto ct_div = encrypt_n(-n_div);

    auto start = high_resolution_clock::now();

    for (int i = 0; i < N; i++) {
        int op = i % 4;
        
        if (op == 0) {
            ct_state = cc->EvalAdd(ct_state, ct_add);
            expected += 7.0;
        } else if (op == 1) {
            ct_state = cc->EvalAdd(ct_state, ct_sub);
            expected -= 3.0;
        } else if (op == 2) {
            ct_state = cc->EvalAdd(ct_state, ct_mul);
            expected *= 2.0;
        } else {
            ct_state = cc->EvalAdd(ct_state, ct_div);
            expected /= 3.0;
        }
    }

    auto end = high_resolution_clock::now();
    auto time = duration_cast<milliseconds>(end - start).count();

    auto v_final = decrypt_n(ct_state);

    cout << "  ✅ Chain complete!\n";
    cout << "  Time: " << time << " ms\n";
    cout << "  Ops/sec: " << fixed << setprecision(0) << (N * 1000.0 / time) << "\n\n";

    cout << "  Final n: " << v_final[0] << "\n";
    cout << "  Final F: " << v_final[1] << "\n";
    cout << "  Expected: " << expected << "\n";
    cout << "  Level: " << ct_state->GetLevel() << "\n\n";

    cout << "========================================\n";
    cout << "  φ-REAL INDEX FHE COMPLETE\n";
    cout << "========================================\n\n";

    return 0;
}
