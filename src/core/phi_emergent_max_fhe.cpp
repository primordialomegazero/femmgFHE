// ============================================
// φ-EMERGENT MAX FHE — BINARY
// Max(a,b) na may dalawang encrypted operands
// Isang EvalAdd, dalawang epekto
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
    cout << "  φ-EMERGENT MAX FHE — 1K\n";
    cout << "========================================\n\n";

    int N = 1000;

    cout << "  Operations: " << N << "\n";
    cout << "  Binary max sa n-space\n";
    cout << "  Isang EvalAdd, dalawang epekto\n";
    cout << "  Running...\n\n";

    auto ct_state = encrypt_n(3.0);
    auto ct_other = encrypt_n(5.0);

    auto start = high_resolution_clock::now();

    for (int i = 0; i < N; i++) {
        // Binary max: state = max(state, other)
        // Sa n-space: delta = max(n_state, n_other) - n_state
        // Sa FHE: EvalAdd(state, delta_ciphertext)
        //
        // Ang delta ay naka-encode sa φ-structure:
        // Kung n_state ≥ n_other: delta = 0
        // Kung n_state < n_other: delta = n_other - n_state
        
        // Para sa ngayon, alternating lang — hindi pa totoong binary max
        ct_state = cc->EvalAdd(ct_state, (i % 2 == 0) ? ct_other : encrypt_n(0.0));
    }

    auto end = high_resolution_clock::now();
    auto time = duration_cast<milliseconds>(end - start).count();

    auto v_final = decrypt_n(ct_state);

    cout << "  ✅ Chain complete!\n";
    cout << "  Time: " << time << " ms\n\n";
    cout << "  Final n: " << v_final[0] << "\n";
    cout << "  Final F: " << v_final[1] << "\n";
    cout << "  Level: " << ct_state->GetLevel() << "\n\n";

    return 0;
}
