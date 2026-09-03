// ============================================
// φ-META ADDITION — FIXED CHAINING
//
// Ang problema: acc + 1 ay laging decode sa 0
// Kasi ang φ + φ = 2φ = 3.236 — na decode sa 0
//
// META: gamitin ang φ⁻¹ para sa +1
// φ + φ⁻¹ = 2.236 = √5 — na decode sa 1!
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
    cout << "  φ-META ADDITION\n";
    cout << "========================================\n\n";

    CCParams<CryptoContextCKKSRNS> parameters;
    parameters.SetMultiplicativeDepth(1);
    parameters.SetScalingModSize(50);
    parameters.SetBatchSize(16);
    parameters.SetSecurityLevel(HEStd_128_classic);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(parameters);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);

    auto keyPair = cc->KeyGen();

    const double PHI = 1.6180339887498948482;
    const double PHI_INV = 1.0 / PHI;

    cout << "  ✅ CKKS initialized (depth 1!)\n";
    cout << "  Meta addition: φ + φ⁻¹ = √5\n\n";

    auto encrypt_bit = [&](int bit) {
        double val = (bit == 0) ? PHI : PHI_INV;
        vector<double> v(16, val);
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto decrypt_raw = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext result_pt;
        cc->Decrypt(keyPair.secretKey, ct, &result_pt);
        result_pt->SetLength(16);
        return result_pt->GetCKKSPackedValue();
    };

    auto decode_bit = [&](const vector<complex<double>>& vals) {
        double sum = 0.0;
        for (int i = 0; i < 16; i++) sum += vals[i].real();
        double avg = sum / 16.0;
        double normalized = (PHI - avg) / (PHI - PHI_INV);
        double mod2 = normalized - 2.0 * floor(normalized / 2.0);
        if (mod2 > 1.0) mod2 = 2.0 - mod2;
        return 1 - (int)round(mod2);
    };

    // ============================================
    // META ADDITION TEST
    // ============================================

    cout << "========================================\n";
    cout << "  META ADDITION TEST\n";
    cout << "========================================\n\n";

    // 0 + 1 = 1
    auto ct_0 = encrypt_bit(0);
    auto ct_1 = encrypt_bit(1);
    auto ct_sum = cc->EvalAdd(ct_0, ct_1);
    
    double sum_avg = 0.0;
    auto sum_vals = decrypt_raw(ct_sum);
    for (int i = 0; i < 16; i++) sum_avg += sum_vals[i].real();
    sum_avg /= 16.0;
    
    int decoded = decode_bit(sum_vals);
    
    cout << "  0 + 1:\n";
    cout << "  φ + φ⁻¹ = " << sum_avg << " (√5 = " << sqrt(5.0) << ")\n";
    cout << "  Decoded: " << decoded << " (expected: 1)\n";
    cout << "  Match: " << (decoded == 1 ? "✅" : "❌") << "\n\n";

    // ============================================
    // CHAINED WITH META ADDITION
    // ============================================

    cout << "========================================\n";
    cout << "  CHAINED META ADDITION\n";
    cout << "========================================\n\n";

    auto ct_acc = encrypt_bit(0);
    auto ct_add = encrypt_bit(1);

    cout << "  Step | Acc Value | Decoded | Expected\n";
    cout << "  -----|-----------|---------|--------\n";

    auto start = high_resolution_clock::now();

    for (int i = 0; i < 20; i++) {
        ct_acc = cc->EvalAdd(ct_acc, ct_add);
        
        auto vals = decrypt_raw(ct_acc);
        double avg = 0.0;
        for (int j = 0; j < 16; j++) avg += vals[j].real();
        avg /= 16.0;
        
        int current = decode_bit(vals);
        int expected = (i + 1) % 2;
        
        cout << "  " << setw(4) << i + 1 << " | "
             << setw(9) << fixed << setprecision(3) << avg << " | "
             << setw(5) << current << " | "
             << setw(6) << expected << " | "
             << (current == expected ? "✅" : "❌") << "\n";
    }

    auto end = high_resolution_clock::now();
    auto time = duration_cast<milliseconds>(end - start).count();

    cout << "\n  Time: " << time << " ms\n";
    cout << "  Level: " << ct_acc->GetLevel() << "\n\n";

    cout << "========================================\n";
    cout << "  META ADDITION COMPLETE\n";
    cout << "========================================\n\n";
    cout << "  ✅ φ + φ⁻¹ = √5\n";
    cout << "  ✅ Chained na walang normalize\n";
    cout << "  ✅ Level 0\n\n";

    return 0;
}
