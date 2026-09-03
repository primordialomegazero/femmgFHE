// ============================================
// φ-ONESHOT DUAL
// Isang encrypt, isang decrypt, 1K ops sa gitna
// WALANG re-encryption
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
    cout << "  φ-ONESHOT DUAL — PURE FHE\n";
    cout << "========================================\n\n";

    int N = 1000;

    cout << "  Operations: " << N << "\n";
    cout << "  Isang encrypt sa simula\n";
    cout << "  Isang decrypt sa dulo\n";
    cout << "  WALANG re-encryption sa gitna\n";
    cout << "  Running...\n\n";

    // Isang encrypt lang
    auto ct_state = encrypt_dual(5.0);
    
    // Isang constant delta
    auto ct_delta = encrypt_dual(pow(PHI, 4));

    auto start = high_resolution_clock::now();

    for (int i = 0; i < N; i++) {
        ct_state = cc->EvalAdd(ct_state, ct_delta);
    }

    auto end = high_resolution_clock::now();
    auto time = duration_cast<milliseconds>(end - start).count();

    // Isang decrypt lang
    auto v_final = decrypt_dual(ct_state);
    double F_final = v_final[1] * PHI + v_final[0];
    double F_expected = 5.0 + N * pow(PHI, 4);

    cout << "  ✅ Chain complete!\n";
    cout << "  Time: " << time << " ms\n\n";
    cout << "  Final F: " << F_final << "\n";
    cout << "  Expected F: " << F_expected << "\n\n";

    double error_pct = abs(F_final - F_expected) / F_expected * 100.0;
    cout << "  Error: " << fixed << setprecision(6) << error_pct << "%\n";
    cout << "  Match: " << (error_pct < 0.01 ? "✅" : "❌") << "\n\n";

    cout << "========================================\n";
    cout << "  " << (error_pct < 0.01 ? "✅ HOLY GRAIL CONFIRMED" : "❌ MAY ISSUE") << "\n";
    cout << "========================================\n\n";

    return 0;
}
