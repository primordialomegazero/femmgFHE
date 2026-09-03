// ============================================
// φ-FRACTAL RESET — PER-ROUND FRESH
//
// Bawat round: fresh fractal encoding
// Walang cross-round drift
// 112/112 target!
//
// Author: Dan Fernandez / Primordial Omega Zero
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <complex>
#include <iomanip>
#include <chrono>

#include "pke/openfhe.h"

using namespace lbcrypto;
using namespace std;
using namespace std::chrono;

int main() {
    cout << "========================================\n";
    cout << "  φ-FRACTAL RESET\n";
    cout << "========================================\n\n";

    CCParams<CryptoContextCKKSRNS> parameters;
    parameters.SetMultiplicativeDepth(1);
    parameters.SetScalingModSize(59);
    parameters.SetBatchSize(16);
    parameters.SetSecurityLevel(HEStd_128_classic);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(parameters);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);

    auto keyPair = cc->KeyGen();

    const double PHI = 1.6180339887498948482;
    const double PHI_INV = 1.0 / PHI;

    cout << "  ✅ CKKS initialized (depth 1, modsize 59!)\n";
    cout << "  Fractal reset: fresh encoding per round\n\n";

    auto encrypt_bit = [&](int bit) {
        double val = (bit == 0) ? PHI : PHI_INV;
        vector<double> v(16, val);
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto decrypt_avg = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext result_pt;
        cc->Decrypt(keyPair.secretKey, ct, &result_pt);
        result_pt->SetLength(16);
        double sum = 0.0;
        for (int i = 0; i < 16; i++) sum += result_pt->GetCKKSPackedValue()[i].real();
        return sum / 16.0;
    };

    auto zero_adaptive_decode = [&](double val) {
        double mod_phi = fmod(val, PHI);
        if (mod_phi < 0.1 || mod_phi > PHI - 0.1) return 0;
        return (mod_phi < PHI / 2.0) ? 1 : 0;
    };

    // ============================================
    // FRACTAL RESET — 7 ROUNDS, FRESH EACH
    // ============================================

    cout << "========================================\n";
    cout << "  FRACTAL RESET (7 ROUNDS)\n";
    cout << "========================================\n\n";

    auto start = high_resolution_clock::now();
    int fractal_match = 0;

    for (int round = 0; round < 7; round++) {
        // FRESH fractal encoding bawat round
        vector<double> fractal_values(16, 0.0);
        for (int i = 0; i < 16; i++) {
            fractal_values[i] = (i % 2 == 0) ? PHI : PHI_INV;
        }
        
        Plaintext pt_fractal = cc->MakeCKKSPackedPlaintext(fractal_values);
        auto ct_fractal = cc->Encrypt(keyPair.publicKey, pt_fractal);
        
        // I-add ang φ⁻¹ sa lahat
        vector<double> add_values(16, PHI_INV);
        Plaintext pt_add = cc->MakeCKKSPackedPlaintext(add_values);
        auto ct_add_batch = cc->Encrypt(keyPair.publicKey, pt_add);
        
        auto ct_result = cc->EvalAdd(ct_fractal, ct_add_batch);
        
        Plaintext result_pt;
        cc->Decrypt(keyPair.secretKey, ct_result, &result_pt);
        result_pt->SetLength(16);
        
        for (int i = 0; i < 16; i++) {
            double val = result_pt->GetCKKSPackedValue()[i].real();
            int decoded = zero_adaptive_decode(val);
            int expected = (round + i + 1) % 2;
            fractal_match += (decoded == expected);
        }
    }

    auto end = high_resolution_clock::now();
    auto time = duration_cast<milliseconds>(end - start).count();

    cout << "  Fractal Match: " << fractal_match << "/112\n";
    cout << "  Time: " << time << " ms\n";
    cout << "  Level: 0\n\n";

    // ============================================
    // SUMMARY
    // ============================================

    cout << "========================================\n";
    cout << "  FRACTAL RESET COMPLETE\n";
    cout << "========================================\n\n";
    cout << "  ✅ Fractal: " << fractal_match << "/112\n";
    cout << "  ✅ Level 0\n";
    cout << "  ✅ Fresh encoding per round\n\n";

    return 0;
}
