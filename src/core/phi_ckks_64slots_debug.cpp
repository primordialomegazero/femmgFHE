// ============================================
// φ-CKKS 64 SLOTS DEBUG — FULL VERBOSE
//
// Ipakita ang bawat slot: value, decoded, expected
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
    cout << "  φ-CKKS 64 SLOTS DEBUG\n";
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
    cout << "  Encoding: 0→0, 1→10\n";
    cout << "  Decode: mod 20\n\n";

    auto encrypt_bits = [&](uint64_t value) {
        vector<double> vals(64, 0.0);
        for (int i = 0; i < 64; i++) {
            int bit = (value >> i) & 1;
            vals[i] = (bit == 0) ? 0.0 : 10.0;
        }
        Plaintext pt = cc->MakeCKKSPackedPlaintext(vals);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    // ============================================
    // 64-BIT ADDER (FULL PARALLEL) WITH DEBUG
    // ============================================

    cout << "========================================\n";
    cout << "  64-BIT ADDER DEBUG\n";
    cout << "========================================\n\n";

    uint64_t A = 0xF0F0F0F0F0F0F0F0ULL;
    uint64_t B = 0x0F0F0F0F0F0F0F0FULL;
    uint64_t expected_sum = A + B;

    auto start = high_resolution_clock::now();

    auto ct_a = encrypt_bits(A);
    auto ct_b = encrypt_bits(B);
    auto ct_sum = cc->EvalAdd(ct_a, ct_b);

    Plaintext result_pt;
    cc->Decrypt(keyPair.secretKey, ct_sum, &result_pt);
    result_pt->SetLength(64);

    int adder_match = 0;

    cout << "  Slot | A | B | Value | Mod20 | Dec | Exp | Match\n";
    cout << "  -----|---|---|-------|-------|-----|-----|------\n";

    for (int i = 0; i < 64; i++) {
        int bit_a = (A >> i) & 1;
        int bit_b = (B >> i) & 1;
        double val = result_pt->GetCKKSPackedValue()[i].real();
        
        double mod20 = fmod(val, 20.0);
        if (mod20 < 0) mod20 += 20.0;
        
        int decoded = (mod20 < 10.0) ? 0 : 1;
        int expected = (expected_sum >> i) & 1;
        bool match = (decoded == expected);
        adder_match += match;

        cout << "  " << setw(4) << i << " | "
             << bit_a << " | " << bit_b << " | "
             << setw(5) << fixed << setprecision(2) << val << " | "
             << setw(5) << mod20 << " | "
             << decoded << " | " << expected << " | "
             << (match ? "✅" : "❌") << "\n";
    }

    auto end = high_resolution_clock::now();
    auto adder_time = duration_cast<milliseconds>(end - start).count();

    cout << "\n  Match: " << adder_match << "/64\n";
    cout << "  Expected: 0x" << hex << expected_sum << dec << "\n";
    cout << "  Time: " << adder_time << " ms\n";
    cout << "  Level: " << ct_sum->GetLevel() << "\n\n";

    return 0;
}
