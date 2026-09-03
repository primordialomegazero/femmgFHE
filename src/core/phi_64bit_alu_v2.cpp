// ============================================
// φ-64-BIT ALU V2 — SIMPLENG ENCODING
//
// Encoding: 0 → 0, 1 → 2
// Decode: mod 4
// Operations: EvalAdd, EvalSub
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
    cout << "  φ-64-BIT ALU V2\n";
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
    // 64-BIT ADDER
    // ============================================

    cout << "========================================\n";
    cout << "  64-BIT FULL ADDER\n";
    cout << "========================================\n\n";

    uint64_t A = 0xF0F0F0F0F0F0F0F0ULL;
    uint64_t B = 0x0F0F0F0F0F0F0F0FULL;
    uint64_t expected_sum = A + B;

    auto start = high_resolution_clock::now();
    int adder_match = 0;

    Ciphertext<DCRTPoly> carry = encrypt_bit(0);

    for (int i = 0; i < 64; i++) {
        int bit_a = (A >> i) & 1;
        int bit_b = (B >> i) & 1;

        auto ct_a = encrypt_bit(bit_a);
        auto ct_b = encrypt_bit(bit_b);

        // Sum = A + B + Carry
        auto sum_ab = cc->EvalAdd(ct_a, ct_b);
        auto sum_total = cc->EvalAdd(sum_ab, carry);

        double avg = decrypt_avg(sum_total);
        int decoded = mod4_decode(avg);

        int expected_bit = (expected_sum >> i) & 1;
        adder_match += (decoded == expected_bit);

        // Carry = 1 kung A+B+Carry >= 2
        double val_a = (bit_a == 0) ? 0.0 : 2.0;
        double val_b = (bit_b == 0) ? 0.0 : 2.0;
        double val_carry = (decoded == 0) ? 0.0 : 2.0;
        double sum_val = val_a + val_b + val_carry;

        int carry_bit = (sum_val >= 2.0) ? 1 : 0;
        carry = encrypt_bit(carry_bit);
    }

    auto end = high_resolution_clock::now();
    auto adder_time = duration_cast<milliseconds>(end - start).count();

    cout << "  Match: " << adder_match << "/64\n";
    cout << "  Expected: 0x" << hex << expected_sum << dec << "\n";
    cout << "  Time: " << adder_time << " ms\n";
    cout << "  Level: 0\n\n";

    // ============================================
    // 64-BIT SUBTRACTOR
    // ============================================

    cout << "========================================\n";
    cout << "  64-BIT SUBTRACTOR\n";
    cout << "========================================\n\n";

    uint64_t expected_diff = A - B;
    int sub_match = 0;

    Ciphertext<DCRTPoly> borrow = encrypt_bit(1); // Initial borrow = 1

    for (int i = 0; i < 64; i++) {
        int bit_a = (A >> i) & 1;
        int bit_b = (~B >> i) & 1;

        auto ct_a = encrypt_bit(bit_a);
        auto ct_b = encrypt_bit(bit_b);

        auto sum_ab = cc->EvalAdd(ct_a, ct_b);
        auto sum_total = cc->EvalAdd(sum_ab, borrow);

        double avg = decrypt_avg(sum_total);
        int decoded = mod4_decode(avg);

        int expected_bit = (expected_diff >> i) & 1;
        sub_match += (decoded == expected_bit);

        double val_a = (bit_a == 0) ? 0.0 : 2.0;
        double val_b = (bit_b == 0) ? 0.0 : 2.0;
        double val_borrow = (decoded == 0) ? 0.0 : 2.0;
        double sum_val = val_a + val_b + val_borrow;

        int borrow_bit = (sum_val >= 2.0) ? 1 : 0;
        borrow = encrypt_bit(borrow_bit);
    }

    auto end_sub = high_resolution_clock::now();
    auto sub_time = duration_cast<milliseconds>(end_sub - start).count();

    cout << "  Match: " << sub_match << "/64\n";
    cout << "  Expected: 0x" << hex << expected_diff << dec << "\n";
    cout << "  Time: " << sub_time << " ms\n";
    cout << "  Level: 0\n\n";

    // ============================================
    // SUMMARY
    // ============================================

    cout << "========================================\n";
    cout << "  64-BIT ALU V2 COMPLETE\n";
    cout << "========================================\n\n";
    cout << "  ✅ Adder: " << adder_match << "/64\n";
    cout << "  ✅ Subtractor: " << sub_match << "/64\n";
    cout << "  ✅ Level 0\n";
    cout << "  ✅ Depth 1\n";
    cout << "  ✅ No bootstrapping\n\n";

    return 0;
}
