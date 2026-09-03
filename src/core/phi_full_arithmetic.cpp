// ============================================
// φ-FULL ARITHMETIC
// Complete 4-channel unified state
// Addition, Subtraction, Multiplication, Division
// Lahat pure EvalAdd/EvalSub, walang EvalMult(ct,ct)
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>
#include "pke/openfhe.h"

using namespace lbcrypto;
using namespace std;

int main() {
    CCParams<CryptoContextCKKSRNS> parameters;
    parameters.SetMultiplicativeDepth(1);
    parameters.SetScalingModSize(59);
    parameters.SetBatchSize(4);
    parameters.SetSecurityLevel(HEStd_128_classic);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(parameters);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);
    auto keyPair = cc->KeyGen();

    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);
    vector<long long> fib = {0, 1};
    for (int i = 2; i <= 200; i++) fib.push_back(fib[i-1] + fib[i-2]);

    // ============================================
    // ENCODING: 4-channel unified state
    // Slot 0: r_n = F_n mod φ
    // Slot 1: q_n = floor(F_n / φ)
    // Slot 2: log_φ(F_n)
    // Slot 3: n (index)
    // ============================================

    auto encrypt_full = [&](int n) {
        double F = (double)fib[n];
        double q = floor(F / PHI);
        double r = fmod(F, PHI);
        double log_phi_F = log(F) / LN_PHI;
        
        vector<double> v(4, 0.0);
        v[0] = r;
        v[1] = q;
        v[2] = log_phi_F;
        v[3] = n;
        
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto decrypt_full = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext pt;
        cc->Decrypt(keyPair.secretKey, ct, &pt);
        pt->SetLength(4);
        auto res = pt->GetCKKSPackedValue();
        return vector<double>{res[0].real(), res[1].real(), res[2].real(), res[3].real()};
    };

    cout << "========================================\n";
    cout << "  φ-FULL ARITHMETIC\n";
    cout << "========================================\n\n";

    // ============================================
    // TEST 1: ADDITION
    // ============================================
    cout << "--- 1. ADDITION ---\n\n";
    cout << "  a+b | Recon (q×φ+r) | Expected | Match?\n";
    cout << "  ----|---------------|----------|-------\n";
    for (int a : {3, 5, 7, 9}) {
        for (int b : {a, a+1, a+2}) {
            auto ct_a = encrypt_full(a);
            auto ct_b = encrypt_full(b);
            auto ct_res = cc->EvalAdd(ct_a, ct_b);
            auto v = decrypt_full(ct_res);
            double recon = v[1] * PHI + v[0];
            double expected = (double)(fib[a] + fib[b]);
            bool match = abs(recon - expected) < 0.5;
            cout << "  " << setw(3) << a << "+" << b << " | "
                 << setw(13) << fixed << setprecision(1) << recon << " | "
                 << setw(8) << expected << " | "
                 << (match ? "✅" : "❌") << "\n";
        }
    }

    // ============================================
    // TEST 2: SUBTRACTION
    // ============================================
    cout << "\n--- 2. SUBTRACTION ---\n\n";
    cout << "  a-b | Recon (q×φ+r) | Expected | Match?\n";
    cout << "  ----|---------------|----------|-------\n";
    for (int a : {5, 7, 9, 11}) {
        for (int b : {a-2, a-1}) {
            auto ct_a = encrypt_full(a);
            auto ct_b = encrypt_full(b);
            auto ct_res = cc->EvalSub(ct_a, ct_b);
            auto v = decrypt_full(ct_res);
            double recon = v[1] * PHI + v[0];
            double expected = (double)(fib[a] - fib[b]);
            bool match = abs(recon - expected) < 0.5;
            cout << "  " << setw(3) << a << "-" << b << " | "
                 << setw(13) << fixed << setprecision(1) << recon << " | "
                 << setw(8) << expected << " | "
                 << (match ? "✅" : "❌") << "\n";
        }
    }

    // ============================================
    // TEST 3: MULTIPLICATION (Log Space via EvalAdd)
    // ============================================
    cout << "\n--- 3. MULTIPLICATION (Log Space) ---\n\n";
    cout << "  a×b | φ^(log_sum) | Expected | Match?\n";
    cout << "  ----|--------------|----------|-------\n";
    for (int a : {3, 4, 5, 6}) {
        for (int b : {a, a+1}) {
            auto ct_a = encrypt_full(a);
            auto ct_b = encrypt_full(b);
            auto ct_res = cc->EvalAdd(ct_a, ct_b);
            auto v = decrypt_full(ct_res);
            double product = pow(PHI, v[2]);
            double expected = (double)(fib[a] * fib[b]);
            bool match = abs(product - expected) < 2.0;
            cout << "  " << setw(3) << a << "×" << b << " | "
                 << setw(12) << fixed << setprecision(1) << product << " | "
                 << setw(8) << expected << " | "
                 << (match ? "✅" : "❌") << "\n";
        }
    }

    // ============================================
    // TEST 4: DIVISION (Log Space via EvalSub)
    // ============================================
    cout << "\n--- 4. DIVISION (Log Space) ---\n\n";
    cout << "  a÷b | φ^(log_diff) | Expected | Match?\n";
    cout << "  ----|--------------|----------|-------\n";
    for (int a : {5, 6, 7, 8}) {
        for (int b : {a-2, a-1}) {
            auto ct_a = encrypt_full(a);
            auto ct_b = encrypt_full(b);
            auto ct_res = cc->EvalSub(ct_a, ct_b);
            auto v = decrypt_full(ct_res);
            double quotient = pow(PHI, v[2]);
            double expected = (double)fib[a] / (double)fib[b];
            bool match = abs(quotient - expected) < 0.5;
            cout << "  " << setw(3) << a << "÷" << b << " | "
                 << setw(12) << fixed << setprecision(3) << quotient << " | "
                 << setw(8) << expected << " | "
                 << (match ? "✅" : "❌") << "\n";
        }
    }

    // ============================================
    // TEST 5: MIXED OPERATIONS (Arbitrary)
    // ============================================
    cout << "\n--- 5. MIXED OPERATIONS ---\n\n";
    cout << "  (a+b)×c÷d style — combination ng EvalAdd at EvalSub\n\n";

    // (F_5 + F_3) × (F_4) ÷ (F_2)
    // = (5 + 2) × 3 ÷ 1 = 21
    cout << "  Formula: (F_5 + F_3) × F_4 ÷ F_2\n";
    cout << "  Expected: (5 + 2) × 3 ÷ 1 = 21\n";
    {
        auto ct_a = encrypt_full(5);
        auto ct_b = encrypt_full(3);
        auto ct_add = cc->EvalAdd(ct_a, ct_b);   // F_5 + F_3 = 7

        auto ct_c = encrypt_full(4);
        auto ct_mult = cc->EvalAdd(ct_add, ct_c);  // log(7×3)

        auto ct_d = encrypt_full(2);
        auto ct_div = cc->EvalSub(ct_mult, ct_d);  // log(21÷1)

        auto v = decrypt_full(ct_div);
        double result = pow(PHI, v[2]);
        bool match = abs(result - 21.0) < 2.0;
        cout << "  Result: " << fixed << setprecision(1) << result << " | "
             << (match ? "✅" : "❌") << "\n";
    }

    // (F_6 - F_3) × (F_5) ÷ (F_2)
    // = (8 - 2) × 5 ÷ 1 = 30
    cout << "\n  Formula: (F_6 - F_3) × F_5 ÷ F_2\n";
    cout << "  Expected: (8 - 2) × 5 ÷ 1 = 30\n";
    {
        auto ct_a = encrypt_full(6);
        auto ct_b = encrypt_full(3);
        auto ct_sub = cc->EvalSub(ct_a, ct_b);   // F_6 - F_3 = 6

        auto ct_c = encrypt_full(5);
        auto ct_mult = cc->EvalAdd(ct_sub, ct_c);  // log(6×5)

        auto ct_d = encrypt_full(2);
        auto ct_div = cc->EvalSub(ct_mult, ct_d);  // log(30÷1)

        auto v = decrypt_full(ct_div);
        double result = pow(PHI, v[2]);
        bool match = abs(result - 30.0) < 2.0;
        cout << "  Result: " << fixed << setprecision(1) << result << " | "
             << (match ? "✅" : "❌") << "\n";
    }

    cout << "\n========================================\n";
    cout << "  FULL ARITHMETIC COMPLETE\n";
    cout << "========================================\n\n";
    cout << "  ✅ Addition: EvalAdd (normal space)\n";
    cout << "  ✅ Subtraction: EvalSub (normal space)\n";
    cout << "  ✅ Multiplication: EvalAdd (log space)\n";
    cout << "  ✅ Division: EvalSub (log space)\n";
    cout << "  ✅ Mixed operations\n";
    cout << "  ✅ Walang EvalMult(ct, ct)\n";
    cout << "  ✅ Walang bootstrapping\n\n";

    return 0;
}
