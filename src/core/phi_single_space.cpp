// ============================================
// φ-SINGLE SPACE — 10K
// Isang value, dalawang interpretations
// Transmutation ay automatic sa φ
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
    parameters.SetMultiplicativeDepth(2);
    parameters.SetScalingModSize(59);
    parameters.SetBatchSize(2);
    parameters.SetSecurityLevel(HEStd_128_classic);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(parameters);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);
    auto keyPair = cc->KeyGen();
    cc->EvalMultKeyGen(keyPair.secretKey);

    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);
    const double PHI_INV = 1.0 / PHI;

    auto encrypt_phi = [&](double n) {
        vector<double> v(2, 0.0);
        v[0] = pow(PHI, n);
        v[1] = n;
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto decrypt_phi = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext pt;
        cc->Decrypt(keyPair.secretKey, ct, &pt);
        pt->SetLength(2);
        auto res = pt->GetCKKSPackedValue();
        return vector<double>{res[0].real(), res[1].real()};
    };

    cout << "========================================\n";
    cout << "  φ-SINGLE SPACE — 10K\n";
    cout << "========================================\n\n";
    cout << "  Isang value, dalawang interpretations\n";
    cout << "  φ^n + φ^(n-1) = φ^(n+1)\n\n";

    // Start sa φ^1 = φ
    auto ct_state = encrypt_phi(1.0);

    // Pre-compute φ⁻¹ bilang encrypted constant
    vector<double> phi_inv_v(2, 0.0);
    phi_inv_v[0] = PHI_INV;   // normal: φ⁻¹
    phi_inv_v[1] = PHI_INV;   // reference
    Plaintext pt_phi_inv = cc->MakeCKKSPackedPlaintext(phi_inv_v);

    cout << "  Running 10K iterations...\n\n";

    auto start = high_resolution_clock::now();

    for (int i = 0; i < 10000; i++) {
        // φ^(n-1) = φ^n × φ⁻¹
        auto ct_prev = cc->EvalMult(ct_state, pt_phi_inv);
        
        // φ^n + φ^(n-1) = φ^(n+1)
        ct_state = cc->EvalAdd(ct_state, ct_prev);
    }

    auto end = high_resolution_clock::now();
    auto time = duration_cast<milliseconds>(end - start).count();

    auto v_final = decrypt_phi(ct_state);

    cout << "  ✅ Chain complete!\n";
    cout << "  Time: " << time << " ms\n\n";
    cout << "  Final value: " << v_final[0] << "\n";
    cout << "  Final log (reference): " << v_final[1] << "\n";
    cout << "  Expected exponent: " << 1.0 + 2.0 * 10000.0 << "\n";
    cout << "  Actual exponent (from value): " << log(v_final[0]) / LN_PHI << "\n";
    cout << "  Level: " << ct_state->GetLevel() << "\n\n";

    return 0;
}
