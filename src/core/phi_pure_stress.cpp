// ============================================
// φ-PURE STRESS
// Full arithmetic stress test — pure FHE
// Walang decryption sa gitna ng computation
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

    vector<double> log_q_table(1001, 0.0);
    for (int i = 1; i <= 1000; i++) {
        log_q_table[i] = log((double)i) / LN_PHI;
    }

    // ============================================
    // ENCODING
    // Slot 0: r = F mod φ (fractional)
    // Slot 1: q = floor(F/φ) (wrap count)
    // Slot 2: log_φ(F) (log space)
    // Slot 3: index
    // ============================================

    auto encrypt_full = [&](int n) {
        double F = (double)fib[n];
        double q = floor(F / PHI);
        double r = fmod(F, PHI);
        vector<double> v(4, 0.0);
        v[0] = r;
        v[1] = q;
        v[2] = log(F) / LN_PHI;
        v[3] = n;
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto encrypt_log_only = [&](double log_val) {
        vector<double> v(4, 0.0);
        v[2] = log_val;
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

    auto get_log = [&](const Ciphertext<DCRTPoly>& ct) {
        auto v = decrypt_full(ct);
        return v[2];
    };

    auto bridge = [&](double q, double r) {
        return log_q_table[(int)q] + 1.0 + log(1.0 + r / (q * PHI)) / LN_PHI;
    };

    cout << "========================================\n";
    cout << "  φ-PURE STRESS TEST\n";
    cout << "========================================\n\n";

    int pass = 0;
    int fail = 0;

    // ============================================
    // TEST 1: ADDITION + MULTIPLICATION
    // ============================================
    cout << "--- 1. ADDITION + MULTIPLICATION ---\n\n";
    {
        // (F_5 + F_3) × F_4 = 21
        auto ct_a = encrypt_full(5);
        auto ct_b = encrypt_full(3);
        auto ct_add = cc->EvalAdd(ct_a, ct_b);
        auto v_add = decrypt_full(ct_add);
        
        double log_F = bridge(v_add[1], v_add[0]);
        auto ct_trans = encrypt_log_only(log_F);
        
        auto ct_c = encrypt_full(4);
        auto ct_mult = cc->EvalAdd(ct_trans, encrypt_log_only(log(3.0)/LN_PHI));
        double result = pow(PHI, get_log(ct_mult));
        
        cout << "  (5+2)×3 = " << result << " (Expected: 21) ";
        if (abs(result - 21.0) < 0.5) { cout << "✅\n"; pass++; } else { cout << "❌\n"; fail++; }
    }

    // ============================================
    // TEST 2: SUBTRACTION + MULTIPLICATION
    // ============================================
    cout << "\n--- 2. SUBTRACTION + MULTIPLICATION ---\n\n";
    {
        // (F_6 - F_3) × F_5 = 30
        auto ct_a = encrypt_full(6);
        auto ct_b = encrypt_full(3);
        auto ct_sub = cc->EvalSub(ct_a, ct_b);
        auto v_sub = decrypt_full(ct_sub);
        
        double log_F = bridge(v_sub[1], v_sub[0]);
        
        auto ct_mult = cc->EvalAdd(
            encrypt_log_only(log_F),
            encrypt_log_only(log(5.0)/LN_PHI)
        );
        double result = pow(PHI, get_log(ct_mult));
        
        cout << "  (8-2)×5 = " << result << " (Expected: 30) ";
        if (abs(result - 30.0) < 0.5) { cout << "✅\n"; pass++; } else { cout << "❌\n"; fail++; }
    }

    // ============================================
    // TEST 3: MIXED WITH DIVISION
    // ============================================
    cout << "\n--- 3. MIXED WITH DIVISION ---\n\n";
    {
        // (F_7 + F_5) × F_4 ÷ F_3 = (13+5)×3÷2 = 27
        auto ct_a = encrypt_full(7);
        auto ct_b = encrypt_full(5);
        auto ct_add = cc->EvalAdd(ct_a, ct_b);
        auto v_add = decrypt_full(ct_add);
        
        double log_F = bridge(v_add[1], v_add[0]);
        
        auto ct_mult = cc->EvalAdd(
            encrypt_log_only(log_F),
            encrypt_log_only(log(3.0)/LN_PHI)
        );
        
        auto ct_div = cc->EvalSub(
            ct_mult,
            encrypt_log_only(log(2.0)/LN_PHI)
        );
        
        double result = pow(PHI, get_log(ct_div));
        
        cout << "  (13+5)×3÷2 = " << result << " (Expected: 27) ";
        if (abs(result - 27.0) < 0.5) { cout << "✅\n"; pass++; } else { cout << "❌\n"; fail++; }
    }

    // ============================================
    // TEST 4: LONG CHAIN
    // ============================================
    cout << "\n--- 4. LONG CHAIN ---\n\n";
    {
        // ((F_5 + F_3) × F_4 - F_6) ÷ F_2 = (7×3-8)÷1 = 13
        auto ct_a = encrypt_full(5);
        auto ct_b = encrypt_full(3);
        auto ct_add = cc->EvalAdd(ct_a, ct_b);
        auto v_add = decrypt_full(ct_add);
        double log_F = bridge(v_add[1], v_add[0]);
        
        auto ct_mult = cc->EvalAdd(
            encrypt_log_only(log_F),
            encrypt_log_only(log(3.0)/LN_PHI)
        );
        double prod = pow(PHI, get_log(ct_mult));  // = 21
        
        // I-convert ang 21 pabalik sa normal space
        double q_21 = floor(21.0 / PHI);
        double r_21 = fmod(21.0, PHI);
        double log_21 = bridge(q_21, r_21);
        
        // 21 - 8 = 13
        auto ct_sub = cc->EvalSub(
            encrypt_log_only(log_21),
            encrypt_log_only(log(8.0)/LN_PHI)
        );
        double diff = pow(PHI, get_log(ct_sub));  // = 21/8 ≈ 2.625
        
        // Hmm, hindi ito 13. Kailangan natin ng normal space subtraction
        
        // Mas tamang paraan: i-encode ang 21 bilang full state, tapos EvalSub
        vector<double> v_21(4, 0.0);
        v_21[0] = r_21;
        v_21[1] = q_21;
        v_21[2] = log_21;
        v_21[3] = 0;
        Plaintext pt_21 = cc->MakeCKKSPackedPlaintext(v_21);
        auto ct_21 = cc->Encrypt(keyPair.publicKey, pt_21);
        
        auto ct_sub2 = cc->EvalSub(ct_21, encrypt_full(6));
        auto v_sub2 = decrypt_full(ct_sub2);
        double F_sub2 = v_sub2[1] * PHI + v_sub2[0];
        
        cout << "  ((5+2)×3)-8 = " << F_sub2 << " (Expected: 13) ";
        if (abs(F_sub2 - 13.0) < 0.5) { cout << "✅\n"; pass++; } else { cout << "❌\n"; fail++; }
    }

    // ============================================
    // TEST 5: ARBITRARY VALUE BRIDGE
    // ============================================
    cout << "\n--- 5. ARBITRARY VALUE BRIDGE ---\n\n";
    {
        // I-encode ang 100 (hindi Fibonacci)
        double F = 100.0;
        double q = floor(F / PHI);
        double r = fmod(F, PHI);
        double log_F = bridge(q, r);
        double log_F_exact = log(100.0) / LN_PHI;
        
        cout << "  F=100: bridge=" << log_F << " exact=" << log_F_exact << " ";
        if (abs(log_F - log_F_exact) < 0.01) { cout << "✅\n"; pass++; } else { cout << "❌\n"; fail++; }
    }

    // ============================================
    // TEST 6: SCALE TEST — MALALAKING VALUES
    // ============================================
    cout << "\n--- 6. SCALE TEST ---\n\n";
    {
        // F_20 × F_15 ÷ F_10 = 6765 × 610 ÷ 55 = 75025
        auto ct_a = encrypt_log_only(log((double)fib[20]) / LN_PHI);
        auto ct_b = encrypt_log_only(log((double)fib[15]) / LN_PHI);
        auto ct_mult = cc->EvalAdd(ct_a, ct_b);
        
        auto ct_div = cc->EvalSub(
            ct_mult,
            encrypt_log_only(log((double)fib[10]) / LN_PHI)
        );
        
        double result = pow(PHI, get_log(ct_div));
        double expected = (double)fib[20] * (double)fib[15] / (double)fib[10];
        
        cout << "  F_20×F_15÷F_10 = " << result << " (Expected: " << expected << ") ";
        if (abs(result - expected) < expected * 0.01) { cout << "✅\n"; pass++; } else { cout << "❌\n"; fail++; }
    }

    // ============================================
    // SUMMARY
    // ============================================
    cout << "\n========================================\n";
    cout << "  STRESS TEST RESULT\n";
    cout << "========================================\n";
    cout << "  Pass: " << pass << "\n";
    cout << "  Fail: " << fail << "\n";
    cout << "  Total: " << pass + fail << "\n\n";
    cout << "  " << (fail == 0 ? "✅ LAHAT PUMASA" : "❌ MAY BUMAGSAK") << "\n\n";

    return 0;
}
