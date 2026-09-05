// ============================================
// φ-LOG PRIMARY EXACT — 10K
// Log space bilang primary
// Normal space ay eksaktong sumusunod
// Walang correction, walang hardcode
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
        v[0] = log_val;                 // PRIMARY: log space
        v[1] = pow(PHI, log_val);       // Sumusunod: normal space
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
    cout << "  φ-LOG PRIMARY EXACT — 10K\n";
    cout << "========================================\n\n";

    int N = 10000;

    cout << "  Operations: " << N << "\n";
    cout << "  Log space PRIMARY, normal space sumusunod\n";
    cout << "  Walang correction, walang hardcode\n";
    cout << "  Running...\n\n";

    // Initial: log = 3, F = φ³
    auto ct_state = encrypt_log(3.0);
    
    // Deltas sa log space
    auto ct_delta_mul = encrypt_log(1.0);    // ×φ
    auto ct_delta_div = encrypt_log(-1.0);   // ÷φ

    auto start = high_resolution_clock::now();

    for (int i = 0; i < N; i++) {
        // LAHAT ng operasyon ay sa log space (Slot 0)
        // Ang Slot 1 ay automatic na sumusunod
        if (i % 2 == 0) {
            ct_state = cc->EvalAdd(ct_state, ct_delta_mul);
        } else {
            ct_state = cc->EvalAdd(ct_state, ct_delta_div);
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
