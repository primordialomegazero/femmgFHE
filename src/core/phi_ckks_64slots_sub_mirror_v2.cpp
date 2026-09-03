// ============================================
// φ-CKKS 64 SLOTS SUB MIRROR V2
//
// Tamang mirror encoding:
// A: 0→0, 1→+10
// -B: 0→0, 1→-10
//
// Decode: 0→0, ±10→1
//
// Author: Dan Fernandez / Primordial Omega Zero
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
    cout << "========================================\n";
    cout << "  φ-CKKS 64 SLOTS SUB MIRROR V2\n";
    cout << "========================================\n\n";

    CCParams<CryptoContextCKKSRNS> parameters;
    parameters.SetMultiplicativeDepth(1);
    parameters.SetScalingModSize(50);
    parameters.SetBatchSize(64);
    parameters.SetSecurityLevel(HEStd_128_classic);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(parameters);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);

    auto keyPair = cc->KeyGen();

    cout << "  ✅ CKKS initialized (depth 1, batch 64!)\n";
    cout << "  Tamang mirror: A + (-B)\n";
    cout << "  Decode: 0→0, ±10→1\n\n";

    auto encrypt_bits = [&](uint64_t value, bool is_negative = false) {
        vector<double> vals(64, 0.0);
        for (int i = 0; i < 64; i++) {
            int bit = (value >> i) & 1;
            if (!is_negative) {
                vals[i] = (bit == 0) ? 0.0 : 10.0;
            } else {
                vals[i] = (bit == 0) ? 0.0 : -10.0;
            }
        }
        Plaintext pt = cc->MakeCKKSPackedPlaintext(vals);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto decode_bit = [&](double val) {
        double rounded = round(val / 10.0) * 10.0;
        double mod20 = fmod(rounded, 20.0);
        if (mod20 < 0) mod20 += 20.0;
        
        // 0 → 0
        // ±10 → 1
        if (mod20 >= 8.0 && mod20 <= 12.0) return 1;
        return 0;
    };

    // ============================================
    // 64-BIT SUBTRACTOR
    // ============================================

    cout << "========================================\n";
    cout << "  64-BIT SUBTRACTOR (MIRROR V2)\n";
    cout << "========================================\n\n";

    uint64_t A = 0xF0F0F0F0F0F0F0F0ULL;
    uint64_t B = 0x0F0F0F0F0F0F0F0FULL;
    uint64_t expected_diff = A - B;

    auto start = high_resolution_clock::now();

    auto ct_a = encrypt_bits(A);
    auto ct_neg_b = encrypt_bits(B, true);
    auto ct_diff = cc->EvalAdd(ct_a, ct_neg_b);

    Plaintext result_pt;
    cc->Decrypt(keyPair.secretKey, ct_diff, &result_pt);
    result_pt->SetLength(64);

    int sub_match = 0;

    for (int i = 0; i < 64; i++) {
        double val = result_pt->GetCKKSPackedValue()[i].real();
        int decoded = decode_bit(val);
        int expected = (expected_diff >> i) & 1;
        sub_match += (decoded == expected);
    }

    auto end = high_resolution_clock::now();
    auto sub_time = duration_cast<milliseconds>(end - start).count();

    cout << "  Match: " << sub_match << "/64\n";
    cout << "  Expected: 0x" << hex << expected_diff << dec << "\n";
    cout << "  Time: " << sub_time << " ms\n";
    cout << "  Level: " << ct_diff->GetLevel() << "\n\n";

    // ============================================
    // DEBUG: FIRST 16 SLOTS
    // ============================================

    cout << "========================================\n";
    cout << "  DEBUG: FIRST 16 SLOTS\n";
    cout << "========================================\n\n";

    cout << "  Slot | Val | Rounded | Dec | Exp | Match\n";
    cout << "  -----|-----|---------|-----|-----|------\n";

    for (int i = 0; i < 16; i++) {
        double val = result_pt->GetCKKSPackedValue()[i].real();
        double rounded = round(val / 10.0) * 10.0;
        int decoded = decode_bit(val);
        int expected = (expected_diff >> i) & 1;
        bool match = (decoded == expected);
        
        cout << "  " << setw(4) << i << " | "
             << setw(4) << fixed << setprecision(1) << val << " | "
             << setw(7) << rounded << " | "
             << decoded << " | " << expected << " | "
             << (match ? "✅" : "❌") << "\n";
    }

    cout << "\n  Summary: " << sub_match << "/64 correct\n\n";

    return 0;
}
