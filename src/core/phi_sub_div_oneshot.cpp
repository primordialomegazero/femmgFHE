// ============================================
// φ-SUB+DIV ONESHOT
// 10K ops: -φ² at ÷φ alternating
// Isang encrypt, isang decrypt
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

    auto encrypt_state = [&](double F, double log_val) {
        vector<double> v(4, 0.0);
        v[0] = fmod(F, PHI);
        v[1] = floor(F / PHI);
        v[2] = log_val;
        v[3] = 0;
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
    cout << "  φ-SUB+DIV ONESHOT\n";
    cout << "========================================\n\n";

    int N = 10000;

    cout << "  Operations: " << N << "\n";
    cout << "  Cycle: -φ², ÷φ\n";
    cout << "  Isang encrypt, isang decrypt\n";
    cout << "  Running...\n\n";

    // Initial: φ^10 = 122.99
    auto ct_state = encrypt_state(pow(PHI, 10), 10.0);

    // Subtraction delta: -φ² sa normal space (Slot 0 at 1)
    auto ct_sub = encrypt_state(-pow(PHI, 2), 0.0);
    
    // Division delta: -1 sa log space (Slot 2)
    auto ct_div = encrypt_state(0.0, -1.0);

    auto start = high_resolution_clock::now();

    for (int i = 0; i < N; i++) {
        if (i % 2 == 0) {
            ct_state = cc->EvalAdd(ct_state, ct_sub);
        } else {
            ct_state = cc->EvalAdd(ct_state, ct_div);
        }
    }

    auto end = high_resolution_clock::now();
    auto time = duration_cast<milliseconds>(end - start).count();

    auto v_final = decrypt_state(ct_state);

    // Expected tracking
    double expected_F = pow(PHI, 10);
    for (int i = 0; i < N; i++) {
        if (i % 2 == 0) expected_F -= pow(PHI, 2);
        else expected_F /= PHI;
    }

    double actual_F = v_final[1] * PHI + v_final[0];
    double actual_log = v_final[2];

    cout << "  ✅ Chain complete!\n";
    cout << "  Time: " << time << " ms\n";
    cout << "  Ops/sec: " << fixed << setprecision(0) << (N * 1000.0 / time) << "\n\n";

    cout << "  Final F (normal): " << scientific << setprecision(6) << actual_F << "\n";
    cout << "  Expected F:       " << expected_F << "\n";
    cout << "  Final log:        " << actual_log << "\n\n";

    double error_F = abs(actual_F - expected_F) / expected_F * 100.0;
    cout << "  Error (F): " << fixed << setprecision(6) << error_F << "%\n\n";

    cout << "========================================\n";
    cout << "  " << (error_F < 1.0 ? "✅ OK" : "❌ ISSUE") << "\n";
    cout << "========================================\n\n";

    return 0;
}
