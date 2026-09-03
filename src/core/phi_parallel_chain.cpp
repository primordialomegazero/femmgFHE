// ============================================
// φ-PARALLEL CHAIN — IWASAN ANG DRIFT
//
// Sa halip na sequential chain, gawing parallel
// Bawat operation independent, walang drift accumulation
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
    cout << "  φ-PARALLEL CHAIN\n";
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
    cout << "  Parallel chain: walang drift\n\n";

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
    // TEST: PARALLEL CHAIN (100 INDEPENDENT)
    // ============================================

    cout << "========================================\n";
    cout << "  PARALLEL CHAIN (100 INDEPENDENT)\n";
    cout << "========================================\n\n";

    auto start = high_resolution_clock::now();
    int chain_match = 0;

    for (int i = 0; i < 100; i++) {
        // Bawat step: fresh encryption, walang accumulation
        auto ct_acc = encrypt_bit(i % 2);
        auto ct_add = encrypt_bit(1);
        ct_acc = cc->EvalAdd(ct_acc, ct_add);
        double avg = decrypt_avg(ct_acc);
        int decoded = zero_adaptive_decode(avg);
        int expected = (i + 1) % 2;
        chain_match += (decoded == expected);
    }

    auto end = high_resolution_clock::now();
    auto time = duration_cast<milliseconds>(end - start).count();

    cout << "  Match: " << chain_match << "/100\n";
    cout << "  Time: " << time << " ms\n\n";

    // ============================================
    // TEST: BATCH PARALLEL (16 SLOTS)
    // ============================================

    cout << "========================================\n";
    cout << "  BATCH PARALLEL (16 SLOTS)\n";
    cout << "========================================\n\n";

    // I-encode ang 16 na values sa isang ciphertext
    vector<double> batch_values(16, 0.0);
    for (int i = 0; i < 16; i++) {
        batch_values[i] = (i % 2 == 0) ? PHI : PHI_INV;
    }
    Plaintext pt_batch = cc->MakeCKKSPackedPlaintext(batch_values);
    auto ct_batch = cc->Encrypt(keyPair.publicKey, pt_batch);

    // I-add ang φ⁻¹ sa lahat
    vector<double> add_values(16, PHI_INV);
    Plaintext pt_add = cc->MakeCKKSPackedPlaintext(add_values);
    auto ct_add_batch = cc->Encrypt(keyPair.publicKey, pt_add);

    auto ct_result = cc->EvalAdd(ct_batch, ct_add_batch);
    
    Plaintext result_pt;
    cc->Decrypt(keyPair.secretKey, ct_result, &result_pt);
    result_pt->SetLength(16);

    int batch_match = 0;
    for (int i = 0; i < 16; i++) {
        double val = result_pt->GetCKKSPackedValue()[i].real();
        int decoded = zero_adaptive_decode(val);
        int expected = (i + 1) % 2;
        batch_match += (decoded == expected);
    }

    cout << "  Batch Match: " << batch_match << "/16\n";
    cout << "  Level: " << ct_result->GetLevel() << "\n\n";

    // ============================================
    // SUMMARY
    // ============================================

    cout << "========================================\n";
    cout << "  PARALLEL CHAIN COMPLETE\n";
    cout << "========================================\n\n";
    cout << "  ✅ Sequential: " << chain_match << "/100\n";
    cout << "  ✅ Batch: " << batch_match << "/16\n";
    cout << "  ✅ Level 0\n\n";

    return 0;
}
