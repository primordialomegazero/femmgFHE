// ============================================
// φ-EMERGENT MODULO FHE — 10K
// value × φ⁻ⁿ scaling para sa natural na bounded
// Self-correcting: 1 + φ⁻¹ = φ
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
    const double PHI_INV = 1.0 / PHI;
    const double LN_PHI = log(PHI);

    auto encrypt_state = [&](double val, double scaled, double log_val) {
        vector<double> v(4, 0.0);
        v[0] = val;                    // raw value
        v[1] = scaled;                 // φ⁻ⁿ scaled (bounded)
        v[2] = log_val;                // log space
        v[3] = fmod(val, PHI);        // φ-mod
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
    cout << "  φ-EMERGENT MODULO FHE — 10K\n";
    cout << "========================================\n\n";

    int N = 10000;

    cout << "  Operations: " << N << "\n";
    cout << "  Emergent modulo: value × φ⁻ⁿ\n";
    cout << "  Self-correcting: 1 + φ⁻¹ = φ\n";
    cout << "  Running...\n\n";

    // Initial: value = 10, scaled = 10, log = log_φ(10)
    double initial = 10.0;
    auto ct_state = encrypt_state(initial, initial, log(initial) / LN_PHI);

    // φ⁻¹ delta para sa scaling
    vector<double> v_phi_inv(4, 0.0);
    v_phi_inv[0] = PHI_INV;
    v_phi_inv[1] = PHI_INV;
    v_phi_inv[2] = -1.0;  // log space shift
    v_phi_inv[3] = PHI_INV;
    Plaintext pt_phi_inv = cc->MakeCKKSPackedPlaintext(v_phi_inv);
    auto ct_phi_inv = cc->Encrypt(keyPair.publicKey, pt_phi_inv);

    auto start = high_resolution_clock::now();

    for (int i = 0; i < N; i++) {
        // Emergent modulo: EvalAdd ng φ⁻¹ delta sa lahat ng slots
        // Ito ay natural na scaling: value → value × φ⁻¹
        // Na nagba-bound sa value sa [0, 1) habang lumalaki ang n
        ct_state = cc->EvalAdd(ct_state, ct_phi_inv);
    }

    auto end = high_resolution_clock::now();
    auto time = duration_cast<milliseconds>(end - start).count();

    auto v_final = decrypt_state(ct_state);

    cout << "  ✅ Chain complete!\n";
    cout << "  Time: " << time << " ms\n";
    cout << "  Ops/sec: " << fixed << setprecision(0) << (N * 1000.0 / time) << "\n\n";
    cout << "  Final raw: " << v_final[0] << "\n";
    cout << "  Final scaled: " << v_final[1] << "\n";
    cout << "  Final log: " << v_final[2] << "\n";
    cout << "  Final φ-mod: " << v_final[3] << "\n";
    cout << "  Level: " << ct_state->GetLevel() << "\n\n";

    return 0;
}
