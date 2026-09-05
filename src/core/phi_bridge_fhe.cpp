// ============================================
// φ-BRIDGE FHE — 10K
// Test: Iisang EvalAdd, dalawang epekto
// Slot 0 (normal): φ + 1 = φ²
// Slot 1 (log): 1 + φ⁻¹ = φ
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
    const double PHI_INV = 1.0 / PHI;

    // 4-slot state:
    // Slot 0: normal space value (φ)
    // Slot 1: log space value (1 = log_φ(φ))
    // Slot 2: normal + 1 (φ + 1 = φ²)
    // Slot 3: log + φ⁻¹ (1 + φ⁻¹ = φ)

    auto encrypt_bridge = [&](double normal, double log_val) {
        vector<double> v(4, 0.0);
        v[0] = normal;                    // φ
        v[1] = log_val;                   // 1
        v[2] = normal + 1.0;              // φ + 1 = φ²
        v[3] = log_val + PHI_INV;         // 1 + φ⁻¹ = φ
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto decrypt_bridge = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext pt;
        cc->Decrypt(keyPair.secretKey, ct, &pt);
        pt->SetLength(4);
        auto res = pt->GetCKKSPackedValue();
        return vector<double>{res[0].real(), res[1].real(), res[2].real(), res[3].real()};
    };

    cout << "========================================\n";
    cout << "  φ-BRIDGE FHE — 10K\n";
    cout << "========================================\n\n";
    cout << "  Test: Iisang EvalAdd, dalawang epekto\n";
    cout << "  Slot 0 (normal): φ + 1 = φ²\n";
    cout << "  Slot 1 (log): 1 + φ⁻¹ = φ\n\n";

    // Initial state: φ sa normal, 1 sa log
    auto ct_state = encrypt_bridge(PHI, 1.0);

    // Ang delta: +1 sa normal, +φ⁻¹ sa log
    // Ito ay iisang plaintext na may dalawang magkaibang values
    vector<double> delta(4, 0.0);
    delta[0] = 1.0;        // +1 sa normal
    delta[1] = PHI_INV;    // +φ⁻¹ sa log
    delta[2] = 0.0;
    delta[3] = 0.0;
    Plaintext pt_delta = cc->MakeCKKSPackedPlaintext(delta);
    auto ct_delta = cc->Encrypt(keyPair.publicKey, pt_delta);

    cout << "  Initial state:\n";
    auto v_init = decrypt_bridge(ct_state);
    cout << "    Slot 0 (normal): " << v_init[0] << "\n";
    cout << "    Slot 1 (log): " << v_init[1] << "\n\n";

    cout << "  Running 10K operations...\n\n";

    auto start = high_resolution_clock::now();

    for (int i = 0; i < 10000; i++) {
        ct_state = cc->EvalAdd(ct_state, ct_delta);
    }

    auto end = high_resolution_clock::now();
    auto time = duration_cast<milliseconds>(end - start).count();

    auto v_final = decrypt_bridge(ct_state);

    cout << "  ✅ Chain complete!\n";
    cout << "  Time: " << time << " ms\n\n";
    cout << "  Final state:\n";
    cout << "    Slot 0 (normal): " << v_final[0] << "\n";
    cout << "    Slot 1 (log): " << v_final[1] << "\n\n";

    // Check transmutation
    double expected_normal = PHI + 10000.0 * 1.0;
    double expected_log = 1.0 + 10000.0 * PHI_INV;

    cout << "  Expected normal: " << expected_normal << "\n";
    cout << "  Expected log: " << expected_log << "\n\n";

    cout << "  Transmutation check:\n";
    cout << "    Normal: φ + 10000×1 = " << v_final[0] << "\n";
    cout << "    Log: 1 + 10000×φ⁻¹ = " << v_final[1] << "\n";
    cout << "    φ^(log) = " << pow(PHI, v_final[1]) << "\n";
    cout << "    Normal ≈ φ^(log)? " << (abs(v_final[0] - pow(PHI, v_final[1])) < 0.01 ? "✅" : "❌") << "\n\n";
    cout << "  Level: " << ct_state->GetLevel() << "\n";

    return 0;
}
