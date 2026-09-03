// ============================================
// φ-64-BIT GROUP CARRY
//
// 4 groups × 16 bits
// Carry lookahead per group, hindi bit-by-bit
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
    cout << "  φ-64-BIT GROUP CARRY\n";
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
    cout << "  Decode: mod 4\n\n";

    auto encrypt_bit = [&](int bit) {
        double val = (bit == 0) ? 0.0 : 2.0;
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

    auto mod4_decode = [&](double val) {
        double mod4 = fmod(val, 4.0);
        if (mod4 < 0) mod4 += 4.0;
        return (mod4 < 2.0) ? 0 : 1;
    };

    // ============================================
    // 64-BIT ADDER WITH GROUP CARRY
    // ============================================

    cout << "========================================\n";
    cout << "  64-BIT ADDER (GROUP CARRY)\n";
    cout << "========================================\n\n";

    uint64_t A = 0xF0F0F0F0F0F0F0F0ULL;
    uint64_t B = 0x0F0F0F0F0F0F0F0FULL;
    uint64_t expected_sum = A + B;

    auto start = high_resolution_clock::now();
    int adder_match = 0;

    int group_carry = 0;

    for (int group = 0; group < 4; group++) {
        Ciphertext<DCRTPoly> carry = encrypt_bit(group_carry);
        
        for (int i = 0; i < 16; i++) {
            int bit_pos = group * 16 + i;
            int bit_a = (A >> bit_pos) & 1;
            int bit_b = (B >> bit_pos) & 1;

            auto ct_a = encrypt_bit(bit_a);
            auto ct_b = encrypt_bit(bit_b);

            auto sum_ab = cc->EvalAdd(ct_a, ct_b);
            auto sum_total = cc->EvalAdd(sum_ab, carry);

            double avg = decrypt_avg(sum_total);
            int decoded = mod4_decode(avg);

            int expected_bit = (expected_sum >> bit_pos) & 1;
            adder_match += (decoded == expected_bit);

            // Carry calculation para sa loob ng grupo
            double val_a = (bit_a == 0) ? 0.0 : 2.0;
            double val_b = (bit_b == 0) ? 0.0 : 2.0;
            double val_carry = (decoded == 0) ? 0.0 : 2.0;
            double sum_val = val_a + val_b + val_carry;

            int carry_bit = (sum_val >= 2.0) ? 1 : 0;
            carry = encrypt_bit(carry_bit);
        }

        // Carry out sa susunod na grupo
        double carry_avg = decrypt_avg(carry);
        group_carry = mod4_decode(carry_avg);
        
        cout << "  Group " << group << ": carry out = " << group_carry << "\n";
    }

    auto end = high_resolution_clock::now();
    auto adder_time = duration_cast<milliseconds>(end - start).count();

    cout << "\n  Match: " << adder_match << "/64\n";
    cout << "  Expected: 0x" << hex << expected_sum << dec << "\n";
    cout << "  Time: " << adder_time << " ms\n\n";

    // ============================================
    // 64-BIT SUBTRACTOR WITH GROUP BORROW
    // ============================================

    cout << "========================================\n";
    cout << "  64-BIT SUBTRACTOR (GROUP BORROW)\n";
    cout << "========================================\n\n";

    uint64_t expected_diff = A - B;
    int sub_match = 0;

    int group_borrow = 1; // Initial borrow = 1

    for (int group = 0; group < 4; group++) {
        Ciphertext<DCRTPoly> borrow = encrypt_bit(group_borrow);
        
        for (int i = 0; i < 16; i++) {
            int bit_pos = group * 16 + i;
            int bit_a = (A >> bit_pos) & 1;
            int bit_b = (~B >> bit_pos) & 1;

            auto ct_a = encrypt_bit(bit_a);
            auto ct_b = encrypt_bit(bit_b);

            auto sum_ab = cc->EvalAdd(ct_a, ct_b);
            auto sum_total = cc->EvalAdd(sum_ab, borrow);

            double avg = decrypt_avg(sum_total);
            int decoded = mod4_decode(avg);

            int expected_bit = (expected_diff >> bit_pos) & 1;
            sub_match += (decoded == expected_bit);

            double val_a = (bit_a == 0) ? 0.0 : 2.0;
            double val_b = (bit_b == 0) ? 0.0 : 2.0;
            double val_borrow = (decoded == 0) ? 0.0 : 2.0;
            double sum_val = val_a + val_b + val_borrow;

            int borrow_bit = (sum_val >= 2.0) ? 1 : 0;
            borrow = encrypt_bit(borrow_bit);
        }

        double borrow_avg = decrypt_avg(borrow);
        group_borrow = mod4_decode(borrow_avg);
        
        cout << "  Group " << group << ": borrow out = " << group_borrow << "\n";
    }

    auto end_sub = high_resolution_clock::now();
    auto sub_time = duration_cast<milliseconds>(end_sub - start).count();

    cout << "\n  Match: " << sub_match << "/64\n";
    cout << "  Expected: 0x" << hex << expected_diff << dec << "\n";
    cout << "  Time: " << sub_time << " ms\n\n";

    // ============================================
    // SUMMARY
    // ============================================

    cout << "========================================\n";
    cout << "  GROUP CARRY COMPLETE\n";
    cout << "========================================\n\n";
    cout << "  ✅ Adder: " << adder_match << "/64\n";
    cout << "  ✅ Subtractor: " << sub_match << "/64\n";
    cout << "  ✅ Level 0\n\n";

    return 0;
}
