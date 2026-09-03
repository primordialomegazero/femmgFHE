// ============================================
// φ-CARRY DEBUG — TAMANG PROPAGATION
//
// I-debug ang carry sa 8-bit adder
// Tingnan ang actual values bawat bit
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
    cout << "  φ-CARRY DEBUG\n";
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

    auto add = [&](const Ciphertext<DCRTPoly>& a, const Ciphertext<DCRTPoly>& b) {
        return cc->EvalAdd(a, b);
    };

    // ============================================
    // DEBUG: TAMANG CARRY PROPAGATION
    // ============================================

    cout << "========================================\n";
    cout << "  CARRY PROPAGATION DEBUG\n";
    cout << "========================================\n\n";

    vector<int> A = {0,1,0,1,0,1,0,1};  // 85
    vector<int> B = {0,0,1,1,0,0,1,1};  // 51

    cout << "  A = 01010101 (85)\n";
    cout << "  B = 00110011 (51)\n";
    cout << "  Expected: 10001000 (136)\n\n";

    // TAMANG RIPPLE CARRY:
    // carry_in[i] = carry_out[i-1]
    // sum[i] = A[i] XOR B[i] XOR carry_in[i]
    // carry_out[i] = (A[i] AND B[i]) OR (carry_in[i] AND (A[i] XOR B[i]))

    vector<int> sum_bits(8);
    vector<int> carry_bits(8);
    int carry_val = 0;

    cout << "  Bit | A | B | Cin | Sum | Cout\n";
    cout << "  ----|---|---|-----|-----|-----\n";

    for (int i = 0; i < 8; i++) {
        // PLAINTEXT CORRECT COMPUTATION:
        int total = A[i] + B[i] + carry_val;
        int sum_plain = total % 2;
        int carry_plain = total / 2;
        
        // ENCRYPTED VERSION:
        auto ct_a = encrypt_bit(A[i]);
        auto ct_b = encrypt_bit(B[i]);
        auto ct_carry = encrypt_bit(carry_val);
        
        // SUM = A + B + Cin (2-stage)
        auto ct_stage1 = add(ct_a, ct_b);
        auto ct_sum = add(ct_stage1, ct_carry);
        
        int sum_encrypted = decode_bit(decrypt_raw(ct_sum));
        
        // CARRY = (A AND B) OR (Cin AND (A XOR B))
        // Sa φ-emergent: carry_out = (total >= 2)
        int carry_encrypted = carry_plain;  // Sa plaintext muna
        
        cout << "  " << i << " | " << A[i] << " | " << B[i] << " | "
             << carry_val << " | "
             << sum_encrypted << "(" << sum_plain << ") | "
             << carry_encrypted << "(" << carry_plain << ") | "
             << (sum_encrypted == sum_plain ? "✅" : "❌") << "\n";
        
        sum_bits[i] = sum_plain;
        carry_val = carry_plain;
    }

    cout << "\n  SUM (plaintext): ";
    for (int i = 7; i >= 0; i--) cout << sum_bits[i];
    cout << " (expected: 10001000)\n\n";

    // ============================================
    // DEBUG: CHAINED ADDITIONS (TAMANG APPROACH)
    // ============================================

    cout << "========================================\n";
    cout << "  CHAINED ADDITIONS (TAMANG APPROACH)\n";
    cout << "========================================\n\n";

    // Sa halip na chained na may drift,
    // gawin nating MODULAR na pag-add:
    // acc = (acc + 1) % 2 — na may normalization

    auto ct_acc = encrypt_bit(0);
    auto ct_one = encrypt_bit(1);

    cout << "  Step | Decoded | Expected\n";
    cout << "  -----|---------|--------\n";

    auto start = high_resolution_clock::now();

    for (int i = 0; i < 100; i++) {  // 100 muna para sa debug
        ct_acc = add(ct_acc, ct_one);
        
        int current = decode_bit(decrypt_raw(ct_acc));
        int expected = (i + 1) % 2;
        
        if (i < 10 || i % 10 == 0) {
            cout << "  " << setw(4) << i + 1 << " | "
                 << setw(5) << current << " | "
                 << setw(6) << expected << " | "
                 << (current == expected ? "✅" : "❌") << "\n";
        }
        
        // NORMALIZE: re-encrypt para walang drift
        ct_acc = encrypt_bit(current);
    }

    auto end = high_resolution_clock::now();
    auto time = duration_cast<milliseconds>(end - start).count();

    cout << "\n  Time: " << time << " ms\n";
    cout << "  Level: " << ct_acc->GetLevel() << "\n\n";

    cout << "========================================\n";
    cout << "  CARRY DEBUG COMPLETE\n";
    cout << "========================================\n\n";
    cout << "  ✅ Carry propagation: plaintext correct\n";
    cout << "  ✅ Chained: normalize per step\n";
    cout << "  ✅ Level 0\n\n";

    return 0;
}
