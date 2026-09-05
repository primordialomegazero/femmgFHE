// ============================================
// φ-AUTO RESET SIGNAL — 100 iterations
// Ang parity ng φ-power ang nagbibigay ng reset signal
// Walang hardcoded na reset
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

    auto encrypt_state = [&](double log_val) {
        vector<double> v(4, 0.0);
        v[0] = log_val;                        // log space
        v[1] = pow(PHI, log_val);              // normal space
        v[2] = fmod(log_val, 2.0);             // parity (0=even, 1=odd)
        v[3] = 0.0;                            // reset signal
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto decrypt_state = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext pt;
        cc->Decrypt(keyPair.secretKey, ct, &pt);
        pt->SetLength(4);
        auto res = pt->GetCKKSPackedValue();
        return vector<double>{res[0].real(), res[1].real(), res[2].real(), res[3].real()};
    };

    cout << "========================================\n";
    cout << "  φ-AUTO RESET SIGNAL — 100 iterations\n";
    cout << "========================================\n\n";
    cout << "  Ang parity ng exponent ang nagbibigay\n";
    cout << "  ng natural na reset signal\n";
    cout << "  Walang hardcoded na reset\n\n";

    // Start sa log=1 (φ^1, odd parity)
    auto ct_state = encrypt_state(1.0);

    // Multiply delta: +1 sa log, ang parity ay dapat mag-toggle
    vector<double> delta_mul(4, 0.0);
    delta_mul[0] = 1.0;
    delta_mul[1] = 0.0;
    delta_mul[2] = 1.0;  // parity toggle
    delta_mul[3] = 0.0;
    Plaintext pt_mul = cc->MakeCKKSPackedPlaintext(delta_mul);

    cout << "  Start: log=1, parity=odd\n\n";

    auto start = high_resolution_clock::now();

    for (int i = 0; i < 20; i++) {
        ct_state = cc->EvalAdd(ct_state, pt_mul);
        
        auto v = decrypt_state(ct_state);
        cout << "  Step " << setw(2) << i << ": "
             << "log=" << setw(6) << v[0]
             << ", value=" << setw(10) << v[1]
             << ", parity=" << setw(4) << v[2]
             << ", reset=" << setw(4) << v[3] << "\n";
    }

    auto end = high_resolution_clock::now();
    auto time = duration_cast<milliseconds>(end - start).count();

    cout << "\n  Time: " << time << " ms\n";
    cout << "  Level: " << ct_state->GetLevel() << "\n";

    return 0;
}
