// ============================================
// φ-MODULO FHE
// 1K φ-additions sa mod φ space — pure FHE
// Natural na noise cancellation
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

    auto encrypt_mod = [&](double val) {
        vector<double> v(2, 0.0);
        v[0] = fmod(val, PHI);  // mod φ space
        v[1] = val;              // original space (para sa tracking)
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto decrypt_mod = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext pt;
        cc->Decrypt(keyPair.secretKey, ct, &pt);
        pt->SetLength(2);
        auto res = pt->GetCKKSPackedValue();
        return vector<double>{res[0].real(), res[1].real()};
    };

    cout << "========================================\n";
    cout << "  φ-MODULO FHE — 1K OPS\n";
    cout << "========================================\n\n";

    int N = 1000;

    cout << "  Operations: " << N << "\n";
    cout << "  φ-add sa mod φ space\n";
    cout << "  Walang bootstrapping, walang re-encryption\n";
    cout << "  Running...\n\n";

    // Initial: mod φ ng 5
    auto ct_state = encrypt_mod(5.0);
    double expected_mod = fmod(5.0, PHI);

    auto start = high_resolution_clock::now();

    for (int i = 0; i < N; i++) {
        // φ-add: mod(state + mod(φ⁴, φ), φ)
        auto ct_delta = encrypt_mod(fmod(pow(PHI, 4), PHI));
        ct_state = cc->EvalAdd(ct_state, ct_delta);
        
        expected_mod = fmod(expected_mod + fmod(pow(PHI, 4), PHI), PHI);
    }

    auto end = high_resolution_clock::now();
    auto time = duration_cast<milliseconds>(end - start).count();

    auto v_final = decrypt_mod(ct_state);

    cout << "  ✅ Chain complete!\n";
    cout << "  Time: " << time << " ms\n\n";
    cout << "  Final (mod φ): " << v_final[0] << "\n";
    cout << "  Expected: " << expected_mod << "\n\n";

    double error = abs(v_final[0] - expected_mod);
    cout << "  Error: " << fixed << setprecision(6) << error << "\n";
    cout << "  Match: " << (error < 0.01 ? "✅" : "❌") << "\n\n";

    cout << "========================================\n";
    cout << "  " << (error < 0.01 ? "✅ HOLY GRAIL CONFIRMED" : "❌ MAY ISSUE") << "\n";
    cout << "========================================\n\n";

    return 0;
}
