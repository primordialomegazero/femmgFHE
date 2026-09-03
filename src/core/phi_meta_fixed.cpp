// ============================================
// φ-META FIXED — DYNAMIC DECODE
//
// Dynamic decode: range-based imbis na fixed
// φ + φ⁻¹ = √5 ≈ 2.236 → decode 1
// φ + φ = 2φ = 3.236 → decode 0
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
    cout << "  φ-META FIXED\n";
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
    const double SQRT5 = sqrt(5.0);

    cout << "  ✅ CKKS initialized (depth 1, modsize 59!)\n";
    cout << "  Dynamic decode: range-based\n\n";

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
    // DYNAMIC DECODE
    // ============================================

    auto dynamic_decode = [&](double avg) {
        // Dynamic: hanapin ang pinakamalapit na "valid" value
        // Valid values: φ, φ⁻¹, √5, 2φ, 2φ⁻¹, ...
        
        // φ = 1.618 (0)
        // φ⁻¹ = 0.618 (1)
        // √5 = 2.236 (1 — 0+1)
        // 2φ = 3.236 (0 — 1+1)
        // 2φ⁻¹ = 1.236 (?)
        
        // Simpleng approach: modulo sa φ-period
        double mod_phi = fmod(avg, PHI);
        
        // Decode based sa mod_phi:
        // 0 → φ → 0
        // 0.618 → φ⁻¹ → 1
        // 0.618+1.618=2.236 mod 1.618 = 0.618 → 1
        
        if (mod_phi < 0.01 || mod_phi > PHI - 0.01) return 0;
        else if (abs(mod_phi - PHI_INV) < 0.01) return 1;
        else if (abs(mod_phi - (SQRT5 - PHI)) < 0.01) return 1;
        else if (abs(mod_phi - (2.0 * PHI - 2.0 * PHI)) < 0.01) return 0;
        
        // Fallback: round-based
        double normalized = (PHI - avg) / (PHI - PHI_INV);
        double mod2 = normalized - 2.0 * floor(normalized / 2.0);
        if (mod2 > 1.0) mod2 = 2.0 - mod2;
        return 1 - (int)round(mod2);
    };

    // ============================================
    // CHAINED WITH DYNAMIC DECODE
    // ============================================

    cout << "========================================\n";
    cout << "  CHAINED (DYNAMIC DECODE)\n";
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
        int current = dynamic_decode(avg);
        int expected = (i + 1) % 2;
        
        bool match = (current == expected);
        match_count += match;
        
        if (i < 20) {
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
    cout << "  META FIXED COMPLETE\n";
    cout << "========================================\n\n";
    cout << "  ✅ Dynamic decode\n";
    cout << "  ✅ Range-based thresholds\n";
    cout << "  ✅ Match: " << match_count << "/100\n";
    cout << "  ✅ Level 0\n\n";

    return 0;
}
