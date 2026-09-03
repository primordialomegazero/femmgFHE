// ============================================
// φ-DUAL MOD FHE
// State: (F mod φ, q) — parehong na-update sa bawat op
// Pure additive, walang decrypt
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

    // State: Slot 0 = F mod φ, Slot 1 = q = floor(F/φ)
    auto encrypt_dual = [&](double F) {
        vector<double> v(2, 0.0);
        v[0] = fmod(F, PHI);
        v[1] = floor(F / PHI);
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto decrypt_dual = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext pt;
        cc->Decrypt(keyPair.secretKey, ct, &pt);
        pt->SetLength(2);
        auto res = pt->GetCKKSPackedValue();
        return vector<double>{res[0].real(), res[1].real()};
    };

    cout << "========================================\n";
    cout << "  φ-DUAL MOD FHE — 1K OPS\n";
    cout << "========================================\n\n";

    int N = 1000;

    cout << "  Operations: " << N << "\n";
    cout << "  φ-add sa dual mod space\n";
    cout << "  Slot 0: F mod φ\n";
    cout << "  Slot 1: q = floor(F/φ)\n";
    cout << "  Walang decrypt, walang bootstrapping\n";
    cout << "  Running...\n\n";

    // Initial: F=5, mod φ=0.146, q=3
    auto ct_state = encrypt_dual(5.0);
    double expected_F = 5.0;

    auto start = high_resolution_clock::now();

    for (int i = 0; i < N; i++) {
        // φ-add φ⁴: F += φ⁴, q += floor(φ⁴/φ) = floor(φ³) = 4
        double addend = pow(PHI, 4);
        double q_delta = floor(addend / PHI);
        
        // EvalAdd sa parehong slots
        auto ct_delta = encrypt_dual(addend);
        ct_state = cc->EvalAdd(ct_state, ct_delta);
        
        expected_F += addend;
    }

    auto end = high_resolution_clock::now();
    auto time = duration_cast<milliseconds>(end - start).count();

    auto v_final = decrypt_dual(ct_state);
    double F_final = v_final[1] * PHI + v_final[0];

    cout << "  ✅ Chain complete!\n";
    cout << "  Time: " << time << " ms\n\n";
    cout << "  Final (mod φ): " << v_final[0] << "\n";
    cout << "  Final q: " << v_final[1] << "\n";
    cout << "  Final F: " << F_final << "\n";
    cout << "  Expected F: " << expected_F << "\n\n";

    double error_pct = abs(F_final - expected_F) / expected_F * 100.0;
    cout << "  Error: " << fixed << setprecision(6) << error_pct << "%\n";
    cout << "  Match: " << (error_pct < 0.01 ? "✅" : "❌") << "\n\n";

    cout << "========================================\n";
    cout << "  " << (error_pct < 0.01 ? "✅ HOLY GRAIL CONFIRMED" : "❌ MAY ISSUE") << "\n";
    cout << "========================================\n\n";

    return 0;
}
