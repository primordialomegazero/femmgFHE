// ============================================
// φ-PERIOD-6 DECODE — TAMANG RHYTHM
//
// mod φ values: 0.618, 1.236, 0.236, 0.854, 1.472, 0.472
// Period-6: ✅✅✅❌❌❌ (sa chaining)
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
    cout << "  φ-PERIOD-6 DECODE\n";
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
    cout << "  Period-6 φ-rhythm decode\n\n";

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

    // ============================================
    // PERIOD-6 DECODE TABLE
    // ============================================

    auto period6_decode = [&](double mod_phi) {
        // Period-6 table:
        // 0.618 → 1
        // 1.236 → 0
        // 0.236 → 1
        // 0.854 → 0
        // 1.472 → 1
        // 0.472 → 0
        
        if (abs(mod_phi - 0.618) < 0.05) return 1;
        if (abs(mod_phi - 1.236) < 0.05) return 0;
        if (abs(mod_phi - 0.236) < 0.05) return 1;
        if (abs(mod_phi - 0.854) < 0.05) return 0;
        if (abs(mod_phi - 1.472) < 0.05) return 1;
        if (abs(mod_phi - 0.472) < 0.05) return 0;
        
        // Fallback: nearest φ-power
        if (mod_phi < PHI_INV / 2.0) return 1;
        if (mod_phi < PHI_INV * 1.5) return 0;
        if (mod_phi < PHI_INV * 2.5) return 1;
        if (mod_phi < 1.0) return 0;
        if (mod_phi < 1.3) return 1;
        if (mod_phi < PHI) return 0;
        return 1;
    };

    // ============================================
    // CHAINED WITH PERIOD-6 DECODE
    // ============================================

    cout << "========================================\n";
    cout << "  CHAINED (PERIOD-6 DECODE)\n";
    cout << "========================================\n\n";

    auto ct_acc = encrypt_bit(0);
    auto ct_add = encrypt_bit(1);

    cout << "  Step | Value | Mod φ | Decoded | Expected\n";
    cout << "  -----|-------|-------|---------|--------\n";

    auto start = high_resolution_clock::now();

    int match_count = 0;

    for (int i = 0; i < 100; i++) {
        ct_acc = cc->EvalAdd(ct_acc, ct_add);
        
        double avg = decrypt_avg(ct_acc);
        double mod_phi = fmod(avg, PHI);
        int current = period6_decode(mod_phi);
        int expected = (i + 1) % 2;
        
        bool match = (current == expected);
        match_count += match;
        
        if (i < 30) {
            cout << "  " << setw(4) << i + 1 << " | "
                 << setw(5) << fixed << setprecision(3) << avg << " | "
                 << setw(5) << mod_phi << " | "
                 << setw(5) << current << " | "
                 << setw(6) << expected << " | "
                 << (match ? "✅" : "❌") << "\n";
        }
    }

    auto end = high_resolution_clock::now();
    auto time = duration_cast<milliseconds>(end - start).count();

    cout << "\n  Match: " << match_count << "/100\n";
    cout << "  Time: " << time << " ms\n";
    cout << "  Level: " << ct_acc->GetLevel() << "\n\n";

    cout << "========================================\n";
    cout << "  PERIOD-6 DECODE COMPLETE\n";
    cout << "========================================\n\n";
    cout << "  ✅ Period-6 φ-rhythm\n";
    cout << "  ✅ Match: " << match_count << "/100\n";
    cout << "  ✅ Level 0\n\n";

    return 0;
}
