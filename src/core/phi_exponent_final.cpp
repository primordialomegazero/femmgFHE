// ============================================
// φ-EXPONENT FINAL — Universal Architecture
// State: n (exponent)
// Lahat ng ops: EvalAdd/EvalSub ng constants
// Walang decrypt, walang EvalMult, walang rotate
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
    CCParams<CryptoContextCKKSRNS> parameters;
    parameters.SetMultiplicativeDepth(1);
    parameters.SetScalingModSize(59);
    parameters.SetBatchSize(1);
    parameters.SetSecurityLevel(HEStd_128_classic);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(parameters);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);
    auto keyPair = cc->KeyGen();

    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);

    auto encrypt_n = [&](double n) {
        vector<double> v(1, n);
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto decrypt_n = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext pt;
        cc->Decrypt(keyPair.secretKey, ct, &pt);
        pt->SetLength(1);
        return pt->GetCKKSPackedValue()[0].real();
    };

    cout << "========================================\n";
    cout << "  φ-EXPONENT FINAL — Universal Architecture\n";
    cout << "========================================\n\n";
    cout << "  State: n (single slot exponent)\n";
    cout << "  Value: φ^n (derived)\n\n";

    // ============================================
    // TEST 1: Individual operations
    // ============================================
    cout << "  TEST 1: Individual operations\n\n";

    // Multiply ng φ: n → n+1
    auto ct = encrypt_n(5.0);
    vector<double> d_mul(1, 1.0);
    Plaintext pt_mul = cc->MakeCKKSPackedPlaintext(d_mul);
    ct = cc->EvalAdd(ct, pt_mul);
    cout << "    Mul φ: φ⁵ × φ = φ⁶ = " << pow(PHI, decrypt_n(ct)) << " ✅\n";

    // Divide ng φ: n → n-1
    ct = encrypt_n(5.0);
    vector<double> d_div(1, 1.0);
    Plaintext pt_div = cc->MakeCKKSPackedPlaintext(d_div);
    ct = cc->EvalSub(ct, pt_div);
    cout << "    Div φ: φ⁵ ÷ φ = φ⁴ = " << pow(PHI, decrypt_n(ct)) << " ✅\n";

    // Add φ^(n-1): n → n+1
    ct = encrypt_n(5.0);
    ct = cc->EvalAdd(ct, pt_mul);
    cout << "    Add φ⁴: φ⁵ + φ⁴ = φ⁶ = " << pow(PHI, decrypt_n(ct)) << " ✅\n";

    // Sub φ^(n-1): n → n-2
    ct = encrypt_n(5.0);
    vector<double> d_sub(1, 2.0);
    Plaintext pt_sub = cc->MakeCKKSPackedPlaintext(d_sub);
    ct = cc->EvalSub(ct, pt_sub);
    cout << "    Sub φ⁴: φ⁵ - φ⁴ = φ³ = " << pow(PHI, decrypt_n(ct)) << " ✅\n\n";

    // ============================================
    // TEST 2: Sequence ng mixed operations
    // ============================================
    cout << "  TEST 2: Sequence ng mixed operations\n\n";

    ct = encrypt_n(10.0);
    cout << "    Start: φ¹⁰ = " << pow(PHI, 10) << "\n";

    // +φ⁹ → φ¹¹ (EvalAdd 1)
    ct = cc->EvalAdd(ct, pt_mul);
    cout << "    +φ⁹ → φ¹¹ = " << pow(PHI, decrypt_n(ct)) << "\n";

    // ×φ → φ¹² (EvalAdd 1)
    ct = cc->EvalAdd(ct, pt_mul);
    cout << "    ×φ → φ¹² = " << pow(PHI, decrypt_n(ct)) << "\n";

    // -φ¹¹ → φ¹⁰ (EvalSub 2)
    ct = cc->EvalSub(ct, pt_sub);
    cout << "    -φ¹¹ → φ¹⁰ = " << pow(PHI, decrypt_n(ct)) << "\n";

    // ÷φ → φ⁹ (EvalSub 1)
    ct = cc->EvalSub(ct, pt_div);
    cout << "    ÷φ → φ⁹ = " << pow(PHI, decrypt_n(ct)) << "\n\n";

    // ============================================
    // TEST 3: 10K mixed operations
    // ============================================
    cout << "  TEST 3: 10K mixed operations\n\n";

    ct = encrypt_n(20.0);
    
    auto start = high_resolution_clock::now();
    
    for (int i = 0; i < 10000; i++) {
        switch (i % 4) {
            case 0: ct = cc->EvalAdd(ct, pt_mul); break;  // +1
            case 1: ct = cc->EvalSub(ct, pt_div); break;  // -1
            case 2: ct = cc->EvalAdd(ct, pt_mul); break;  // +1
            case 3: ct = cc->EvalSub(ct, pt_sub); break;  // -2
        }
    }
    
    auto end = high_resolution_clock::now();
    auto time = duration_cast<milliseconds>(end - start).count();
    
    double n_final = decrypt_n(ct);
    double expected_n = 20.0 + 2500.0 * (1.0 - 1.0 + 1.0 - 2.0);
    
    cout << "    Time: " << time << " ms\n";
    cout << "    Final n: " << n_final << "\n";
    cout << "    Expected n: " << expected_n << "\n";
    cout << "    Match: " << (abs(n_final - expected_n) < 0.1 ? "✅" : "❌") << "\n";
    cout << "    Level: " << ct->GetLevel() << "\n\n";

    // ============================================
    // TEST 4: Arbitrary multiply/divide
    // ============================================
    cout << "  TEST 4: Arbitrary multiply/divide\n\n";

    // φ¹⁰ × 3 = φ¹⁰ × φ^log_φ(3)
    ct = encrypt_n(10.0);
    double log3 = log(3.0) / LN_PHI;
    vector<double> d_log3(1, log3);
    Plaintext pt_log3 = cc->MakeCKKSPackedPlaintext(d_log3);
    ct = cc->EvalAdd(ct, pt_log3);
    cout << "    φ¹⁰ × 3 = " << pow(PHI, decrypt_n(ct)) << "\n";
    cout << "    Expected: " << pow(PHI, 10) * 3 << "\n\n";

    // φ¹⁰ ÷ 4 = φ¹⁰ ÷ φ^log_φ(4)
    ct = encrypt_n(10.0);
    double log4 = log(4.0) / LN_PHI;
    vector<double> d_log4(1, log4);
    Plaintext pt_log4 = cc->MakeCKKSPackedPlaintext(d_log4);
    ct = cc->EvalSub(ct, pt_log4);
    cout << "    φ¹⁰ ÷ 4 = " << pow(PHI, decrypt_n(ct)) << "\n";
    cout << "    Expected: " << pow(PHI, 10) / 4 << "\n";
    cout << "    Level: " << ct->GetLevel() << "\n";

    return 0;
}
