// ============================================
// φ-CKKS 64 SLOTS SUB MIRROR
//
// A - B = A + (-B)
// Encoding: 0→-10, 1→+10 para sa B
// Then add to A, round, decode
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
    cout << "  φ-CKKS 64 SLOTS SUB MIRROR\n";
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
    cout << "  Mirror: A + (-B)\n\n";

    auto encrypt_bits = [&](uint64_t value, bool mirror = false) {
        vector<double> vals(64, 0.0);
        for (int i = 0; i < 64; i++) {
            int bit = (value >> i) & 1;
            if (!mirror) {
                vals[i] = (bit == 0) ? 0.0 : 10.0;
            } else {
                vals[i] = (bit == 0) ? -10.0 : 0.0;
            }
        }
        Plaintext pt = cc->MakeCKKSPackedPlaintext(vals);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto decode_bit = [&](double val) {
        double rounded = round(val / 10.0) * 10.0;
        double mod20 = fmod(rounded, 20.0);
        if (mod20 < 0) mod20 += 20.0;
        return (mod20 < 10.0) ? 0 : 1;
    };

    // ============================================
    // 64-BIT SUBTRACTOR (MIRROR)
    // ============================================

    cout << "========================================\n";
    cout << "  64-BIT SUBTRACTOR (MIRROR)\n";
    cout << "========================================\n\n";

    uint64_t A = 0xF0F0F0F0F0F0F0F0ULL;
    uint64_t B = 0x0F0F0F0F0F0F0F0FULL;
    uint64_t expected_diff = A - B;

    auto start = high_resolution_clock::now();

    auto ct_a = encrypt_bits(A);
    auto ct_b_mirror = encrypt_bits(B, true);
    auto ct_diff = cc->EvalAdd(ct_a, ct_b_mirror);

    Plaintext result_pt;
    cc->Decrypt(keyPair.secretKey, ct_diff, &result_pt);
    result_pt->SetLength(64);

    int sub_match = 0;

    cout << "  Slot | A | -B | Value | Rounded | Dec | Exp | Match\n";
    cout << "  -----|---|---|-------|---------|-----|-----|------\n";

    for (int i = 0; i < 64; i++) {
        int bit_a = (A >> i) & 1;
        int bit_b = (B >> i) & 1;
        double val = result_pt->GetCKKSPackedValue()[i].real();
        
        double rounded = round(val / 10.0) * 10.0;
        double mod20 = fmod(rounded, 20.0);
        if (mod20 < 0) mod20 += 20.0;
        
        int decoded = (mod20 < 10.0) ? 0 : 1;
        int expected = (expected_diff >> i) & 1;
        bool match = (decoded == expected);
        sub_match += match;

        cout << "  " << setw(4) << i << " | "
             << bit_a << " | " << bit_b << " | "
             << setw(5) << fixed << setprecision(1) << val << " | "
             << setw(7) << rounded << " | "
             << decoded << " | " << expected << " | "
             << (match ? "✅" : "❌") << "\n";
    }

    auto end = high_resolution_clock::now();
    auto sub_time = duration_cast<milliseconds>(end - start).count();

    cout << "\n  Match: " << sub_match << "/64\n";
    cout << "  Expected: 0x" << hex << expected_diff << dec << "\n";
    cout << "  Time: " << sub_time << " ms\n";
    cout << "  Level: " << ct_diff->GetLevel() << "\n\n";

    return 0;
}
