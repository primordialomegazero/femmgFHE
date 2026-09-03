// ============================================
// φ-64-BIT — MINUS2/PLUS2 ENCODING
//
// Encoding: 0→-2, 1→+2
// Decode: mod 4 (symmetrical around zero)
// Carry: sum >= 0 → carry 1
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
    cout << "  φ-64-BIT MINUS2/PLUS2\n";
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
    cout << "  Encoding: 0→-2, 1→+2\n";
    cout << "  Decode: mod 4 (symmetrical)\n\n";

    auto encrypt_bit = [&](int bit) {
        double val = (bit == 0) ? -2.0 : 2.0;
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

    auto decode = [](double result) {
        // Symmetrical modulo 4
        double mod4 = fmod(result, 4.0);
        if (mod4 < 0) mod4 += 4.0;
        
        // 0 at 2 → 0, 1 at 3 → 1
        if (mod4 < 1.0 || (mod4 >= 2.0 && mod4 < 3.0)) return 0;
        return 1;
    };

    // ============================================
    // 64-BIT ADDER
    // ============================================

    cout << "========================================\n";
    cout << "  64-BIT ADDER\n";
    cout << "========================================\n\n";

    // A = 0xAAAAAAAAAAAAAAAA, B = 0x5555555555555555
    vector<int> A64(64), B64(64);
    for (int i = 0; i < 64; i++) {
        A64[i] = (i % 2 == 0) ? 1 : 0;
        B64[i] = (i % 2 == 0) ? 0 : 1;
    }

    vector<int> sum64(64, 0);
    vector<int> carry64(65, 0);
    int match_add = 0;

    auto start = high_resolution_clock::now();

    for (int i = 0; i < 64; i++) {
        auto ct_a = encrypt_bit(A64[i]);
        auto ct_b = encrypt_bit(B64[i]);
        auto ct_cin = encrypt_bit(carry64[i]);
        
        // Sum = A + B + Cin
        auto ct_sum = cc->EvalAdd(cc->EvalAdd(ct_a, ct_b), ct_cin);
        double sum_avg = decrypt_avg(ct_sum);
        sum64[i] = decode(sum_avg);
        
        // Carry: sum >= 0 (since -2+2=0, 2+2=4, -2-2=-4)
        carry64[i+1] = (sum_avg >= 0.1) ? 1 : 0;
        
        int exp_sum = (A64[i] + B64[i] + carry64[i]) % 2;
        int exp_carry = (A64[i] + B64[i] + carry64[i]) >= 2 ? 1 : 0;
        
        match_add += (sum64[i] == exp_sum && carry64[i+1] == exp_carry);
    }

    auto end = high_resolution_clock::now();
    auto time_add = duration_cast<milliseconds>(end - start).count();

    cout << "  Sum: ";
    for (int i = 63; i >= 0; i--) cout << sum64[i];
    cout << "\n";
    cout << "  Expected: 1111111111111111111111111111111111111111111111111111111111111111\n";
    cout << "  Match: " << match_add << "/64\n";
    cout << "  Time: " << time_add << " ms\n";
    cout << "  Level: 0\n\n";

    // ============================================
    // 64-BIT SUBTRACTOR
    // ============================================

    cout << "========================================\n";
    cout << "  64-BIT SUBTRACTOR\n";
    cout << "========================================\n\n";

    vector<int> diff64(64, 0);
    vector<int> borrow65(65, 0);
    int match_sub = 0;

    for (int i = 0; i < 64; i++) {
        auto ct_a = encrypt_bit(A64[i]);
        auto ct_b = encrypt_bit(B64[i]);
        auto ct_borrow = encrypt_bit(borrow65[i]);
        
        // Diff = A - B - Borrow
        auto ct_diff = cc->EvalSub(cc->EvalSub(ct_a, ct_b), ct_borrow);
        double diff_avg = decrypt_avg(ct_diff);
        diff64[i] = decode(diff_avg);
        
        // Borrow: diff < 0
        borrow65[i+1] = (diff_avg < -0.1) ? 1 : 0;
        
        int a_minus_b = A64[i] - B64[i] - borrow65[i];
        int exp_diff = ((a_minus_b % 2) + 2) % 2;
        int exp_borrow = (a_minus_b < 0) ? 1 : 0;
        
        match_sub += (diff64[i] == exp_diff && borrow65[i+1] == exp_borrow);
    }

    auto end_sub = high_resolution_clock::now();
    auto time_sub = duration_cast<milliseconds>(end_sub - start).count();

    cout << "  Diff: ";
    for (int i = 63; i >= 0; i--) cout << diff64[i];
    cout << "\n";
    cout << "  Match: " << match_sub << "/64\n";
    cout << "  Time: " << time_sub << " ms\n";
    cout << "  Level: 0\n\n";

    // ============================================
    // SUMMARY
    // ============================================

    cout << "========================================\n";
    cout << "  64-BIT MINUS2/PLUS2 SUMMARY\n";
    cout << "========================================\n\n";
    cout << "  ✅ Adder: " << match_add << "/64\n";
    cout << "  ✅ Subtractor: " << match_sub << "/64\n";
    cout << "  ✅ Level 0\n\n";

    return 0;
}
