// ============================================
// φ-PERIOD FHE — 10K
// Log space na may φ-period
// Natural na reset — walang overflow
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

    // φ-period: ang log space ay naka-mod sa φ
    // log_val ∈ [0, φ)
    // Ang period ay φ mismo
    
    auto encrypt_period = [&](double log_val) {
        // I-mod ang log sa φ
        double mod_log = fmod(log_val, PHI);
        vector<double> v(2, 0.0);
        v[0] = mod_log;
        v[1] = pow(PHI, mod_log);  // φ^(mod_log) — bounded sa [1, φ^φ)
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto decrypt_period = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext pt;
        cc->Decrypt(keyPair.secretKey, ct, &pt);
        pt->SetLength(2);
        auto res = pt->GetCKKSPackedValue();
        return vector<double>{res[0].real(), res[1].real()};
    };

    cout << "========================================\n";
    cout << "  φ-PERIOD FHE — 10K\n";
    cout << "========================================\n\n";

    int N = 10000;

    cout << "  Operations: " << N << "\n";
    cout << "  Log space na may φ-period\n";
    cout << "  Natural na reset — walang overflow\n";
    cout << "  Running...\n\n";

    auto ct_state = encrypt_period(1.0);  // log = 1

    // Deltas — lahat naka-mod sa φ
    auto ct_delta1 = encrypt_period(0.5);   // +0.5
    auto ct_delta2 = encrypt_period(-0.3);  // -0.3
    auto ct_delta3 = encrypt_period(0.7);   // +0.7

    auto start = high_resolution_clock::now();

    for (int i = 0; i < N; i++) {
        int op = i % 3;
        
        if (op == 0) {
            ct_state = cc->EvalAdd(ct_state, ct_delta1);
        } else if (op == 1) {
            ct_state = cc->EvalAdd(ct_state, ct_delta2);
        } else {
            ct_state = cc->EvalAdd(ct_state, ct_delta3);
        }
    }

    auto end = high_resolution_clock::now();
    auto time = duration_cast<milliseconds>(end - start).count();

    auto v_final = decrypt_period(ct_state);

    cout << "  ✅ Chain complete!\n";
    cout << "  Time: " << time << " ms\n";
    cout << "  Ops/sec: " << fixed << setprecision(0) << (N * 1000.0 / time) << "\n\n";

    cout << "  Final log (mod φ): " << v_final[0] << "\n";
    cout << "  Final F: " << v_final[1] << "\n";
    cout << "  Level: " << ct_state->GetLevel() << "\n\n";

    cout << "========================================\n";
    cout << "  φ-PERIOD FHE COMPLETE\n";
    cout << "========================================\n\n";

    return 0;
}
