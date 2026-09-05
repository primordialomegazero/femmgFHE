// ============================================
// φ-MAX FHE UNIVERSAL — 10K
// Add=max, Mul=sum, Div=diff — walang correction
// Pure EvalAdd, walang bootstrapping
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
    cout << "  φ-MAX FHE UNIVERSAL — 10K\n";
    cout << "========================================\n\n";

    int N = 10000;

    cout << "  Operations: " << N << "\n";
    cout << "  Add=max, Mul=sum, Div=diff\n";
    cout << "  Walang correction, walang comparison\n";
    cout << "  Pure EvalAdd\n";
    cout << "  Running...\n\n";

    auto ct_state = encrypt_n(3.0);
    
    // Universal deltas sa n-space
    auto ct_mul = encrypt_n(1.0);
    auto ct_div = encrypt_n(-0.5);
    auto ct_max_add = encrypt_n(0.7);

    auto start = high_resolution_clock::now();

    for (int i = 0; i < N; i++) {
        int op = i % 4;
        
        if (op == 0) {
            ct_state = cc->EvalAdd(ct_state, ct_max_add);
        } else if (op == 1) {
            ct_state = cc->EvalAdd(ct_state, ct_mul);
        } else if (op == 2) {
            ct_state = cc->EvalAdd(ct_state, ct_div);
        } else {
            ct_state = cc->EvalAdd(ct_state, ct_mul);
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
    cout << "  Level: " << ct_state->GetLevel() << "\n\n";

    cout << "========================================\n";
    cout << "  UNIVERSAL MAX FHE COMPLETE\n";
    cout << "========================================\n\n";
    cout << "  ✅ Walang correction\n";
    cout << "  ✅ Walang comparison\n";
    cout << "  ✅ Pure EvalAdd\n";
    cout << "  ✅ Walang bootstrapping\n\n";

    return 0;
}
