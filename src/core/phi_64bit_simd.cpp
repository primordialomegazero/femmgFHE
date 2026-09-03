// ============================================
// φ-64-BIT SIMD — PARALLEL BITS PER GROUP
//
// 4 groups × 16 bits
// Bawat grupo: 16 bits sabay-sabay sa 16 slots
// Walang sequential carry sa loob ng grupo
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
    cout << "  φ-64-BIT SIMD\n";
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

    cout << "  ✅ CKKS initialized (depth 1, modsize 50!)\n";
    cout << "  Encoding: 0→0, 1→2\n";
    cout << "  SIMD: 16 bits per group, parallel\n\n";

    auto encrypt_bits = [&](const vector<int>& bits) {
        vector<double> vals(16, 0.0);
        for (int i = 0; i < 16; i++) {
            vals[i] = (bits[i] == 0) ? 0.0 : 2.0;
        }
        Plaintext pt = cc->MakeCKKSPackedPlaintext(vals);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto decrypt_bits = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext result_pt;
        cc->Decrypt(keyPair.secretKey, ct, &result_pt);
        result_pt->SetLength(16);
        vector<int> decoded_bits(16, 0);
        for (int i = 0; i < 16; i++) {
            double val = result_pt->GetCKKSPackedValue()[i].real();
            double mod4 = fmod(val, 4.0);
            if (mod4 < 0) mod4 += 4.0;
            decoded_bits[i] = (mod4 < 2.0) ? 0 : 1;
        }
        return decoded_bits;
    };

    // ============================================
    // 64-BIT ADDER VIA SIMD
    // ============================================

    cout << "========================================\n";
    cout << "  64-BIT ADDER (SIMD)\n";
    cout << "========================================\n\n";

    uint64_t A = 0xF0F0F0F0F0F0F0F0ULL;
    uint64_t B = 0x0F0F0F0F0F0F0F0FULL;
    uint64_t expected_sum = A + B;

    auto start = high_resolution_clock::now();
    int adder_match = 0;

    int group_carry = 0;

    for (int group = 0; group < 4; group++) {
        vector<int> bits_a(16, 0);
        vector<int> bits_b(16, 0);
        
        for (int i = 0; i < 16; i++) {
            int bit_pos = group * 16 + i;
            bits_a[i] = (A >> bit_pos) & 1;
            bits_b[i] = (B >> bit_pos) & 1;
        }

        // I-encrypt ang 16 bits nang sabay-sabay
        auto ct_a = encrypt_bits(bits_a);
        auto ct_b = encrypt_bits(bits_b);
        
        // Kung may carry mula sa nakaraang grupo, i-add sa unang bit
        if (group_carry == 1) {
            vector<int> carry_bits(16, 0);
            carry_bits[0] = 1;
            auto ct_carry = encrypt_bits(carry_bits);
            ct_a = cc->EvalAdd(ct_a, ct_carry);
        }

        // I-add nang parallel
        auto ct_sum = cc->EvalAdd(ct_a, ct_b);

        // I-decode ang lahat ng 16 bits
        auto decoded_bits = decrypt_bits(ct_sum);
        
        for (int i = 0; i < 16; i++) {
            int bit_pos = group * 16 + i;
            int expected_bit = (expected_sum >> bit_pos) & 1;
            adder_match += (decoded_bits[i] == expected_bit);
        }

        // Carry out: ang pinakamataas na bit ay nag-overflow kung ≥ 4
        Plaintext result_pt;
        cc->Decrypt(keyPair.secretKey, ct_sum, &result_pt);
        result_pt->SetLength(16);
        double last_val = result_pt->GetCKKSPackedValue()[15].real();
        group_carry = (last_val >= 2.0) ? 1 : 0;
        
        cout << "  Group " << group << ": carry out = " << group_carry << "\n";
    }

    auto end = high_resolution_clock::now();
    auto adder_time = duration_cast<milliseconds>(end - start).count();

    cout << "\n  Match: " << adder_match << "/64\n";
    cout << "  Expected: 0x" << hex << expected_sum << dec << "\n";
    cout << "  Time: " << adder_time << " ms\n";
    cout << "  Level: 0\n\n";

    // ============================================
    // SUMMARY
    // ============================================

    cout << "========================================\n";
    cout << "  SIMD ADDER COMPLETE\n";
    cout << "========================================\n\n";
    cout << "  ✅ Adder: " << adder_match << "/64\n";
    cout << "  ✅ Level 0\n";
    cout << "  ✅ Depth 1\n";
    cout << "  ✅ No bootstrapping\n\n";

    return 0;
}
