// ============================================
// φ-EVALSUB PERIODIC — 10K
// EvalSub lang bilang primary operation
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

    auto encrypt_state = [&](double val) {
        vector<double> v(2, 0.0);
        v[0] = val;
        v[1] = fmod(val, PHI);
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto decrypt_state = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext pt;
        cc->Decrypt(keyPair.secretKey, ct, &pt);
        pt->SetLength(2);
        auto res = pt->GetCKKSPackedValue();
        return vector<double>{res[0].real(), res[1].real()};
    };

    cout << "========================================\n";
    cout << "  φ-EVALSUB PERIODIC — 10K\n";
    cout << "========================================\n\n";

    int N = 10000;

    cout << "  Operations: " << N << "\n";
    cout << "  EvalSub ng φ⁻¹ sa bawat iteration\n";
    cout << "  Natural na φ-periodic na pagbaba\n";
    cout << "  Running...\n\n";

    auto ct_state = encrypt_state(10.0);
    auto ct_phi_inv = encrypt_state(PHI - 1.0);  // φ⁻¹ = 0.618

    auto start = high_resolution_clock::now();

    for (int i = 0; i < N; i++) {
        ct_state = cc->EvalSub(ct_state, ct_phi_inv);
    }

    auto end = high_resolution_clock::now();
    auto time = duration_cast<milliseconds>(end - start).count();

    auto v_final = decrypt_state(ct_state);

    cout << "  ✅ Chain complete!\n";
    cout << "  Time: " << time << " ms\n";
    cout << "  Ops/sec: " << fixed << setprecision(0) << (N * 1000.0 / time) << "\n\n";
    cout << "  Final value: " << v_final[0] << "\n";
    cout << "  Final φ-mod: " << v_final[1] << "\n";
    cout << "  Level: " << ct_state->GetLevel() << "\n\n";

    return 0;
}
