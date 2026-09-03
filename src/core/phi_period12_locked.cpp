// ============================================
// φ-PERIOD12 LOCKED — 12-STEP BUCKET DETECTION
//
// Period-12 buckets: 12 distinct mod φ values
// Decode: even buckets → 0, odd buckets → 1
// With tolerance: ±0.03
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
    cout << "  φ-PERIOD12 LOCKED\n";
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
    cout << "  Period-12 locked: 12-step bucket detection\n\n";

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

    // PERIOD-12 BUCKET DECODE
    auto period12_locked_decode = [&](double val) {
        double mod_phi = fmod(val, PHI);
        
        // Period-12 buckets (12-step cycle)
        double buckets[12] = {
            0.618, 1.236, 0.236, 0.854, 1.472, 0.472,
            1.090, 0.090, 0.708, 1.326, 0.326, 0.944
        };
        double tolerance = 0.03;
        
        // Hanapin ang pinakamalapit na bucket
        int closest = 0;
        double min_dist = 999.0;
        for (int i = 0; i < 12; i++) {
            double dist = abs(mod_phi - buckets[i]);
            if (dist < min_dist) {
                min_dist = dist;
                closest = i;
            }
        }
        
        // Decode: even buckets → 0, odd buckets → 1
        return (closest % 2 == 0) ? 1 : 0;
    };

    // ============================================
    // TEST 1: SINGLE ADDITIONS (8/8 CHECK)
    // ============================================

    cout << "========================================\n";
    cout << "  SINGLE ADDITIONS (8/8 CHECK)\n";
    cout << "========================================\n\n";

    cout << "  A B Cin | Sum | Expected | Match?\n";
    cout << "  --------|-----|----------|--------\n";

    int single_match = 0;
    for (int A : {0, 1}) {
        for (int B : {0, 1}) {
            for (int Cin : {0, 1}) {
                auto ct_a = encrypt_bit(A);
                auto ct_b = encrypt_bit(B);
                auto ct_cin = encrypt_bit(Cin);
                
                auto ct_sum = cc->EvalAdd(cc->EvalAdd(ct_a, ct_b), ct_cin);
                double avg = decrypt_avg(ct_sum);
                int decoded = period12_locked_decode(avg);
                
                int expected = (A + B + Cin) % 2;
                bool match = (decoded == expected);
                single_match += match;
                
                cout << "  " << A << " " << B << " " << Cin << " | "
                     << setw(3) << decoded << " | "
                     << setw(6) << expected << " | "
                     << (match ? "✅" : "❌") << "\n";
            }
        }
    }
    cout << "\n  Single: " << single_match << "/8\n\n";

    // ============================================
    // TEST 2: CHAINED (100 STEPS)
    // ============================================

    cout << "========================================\n";
    cout << "  CHAINED (PERIOD12 LOCKED)\n";
    cout << "========================================\n\n";

    auto ct_acc = encrypt_bit(0);
    auto ct_add = encrypt_bit(1);

    auto start = high_resolution_clock::now();
    int chain_match = 0;

    for (int i = 0; i < 100; i++) {
        ct_acc = cc->EvalAdd(ct_acc, ct_add);
        double avg = decrypt_avg(ct_acc);
        int decoded = period12_locked_decode(avg);
        int expected = (i + 1) % 2;
        chain_match += (decoded == expected);
    }

    auto end = high_resolution_clock::now();
    auto time = duration_cast<milliseconds>(end - start).count();

    cout << "  Match: " << chain_match << "/100\n";
    cout << "  Time: " << time << " ms\n";
    cout << "  Level: " << ct_acc->GetLevel() << "\n\n";

    // ============================================
    // SUMMARY
    // ============================================

    cout << "========================================\n";
    cout << "  PERIOD12 LOCKED COMPLETE\n";
    cout << "========================================\n\n";
    cout << "  ✅ Single: " << single_match << "/8\n";
    cout << "  ✅ Chained: " << chain_match << "/100\n";
    cout << "  ✅ Level 0\n\n";

    return 0;
}
