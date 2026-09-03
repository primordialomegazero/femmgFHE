// ============================================
// φ-PURE STRESS V2 — With rounding fix
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
        int qi = (int)(q + 0.5);  // ROUNDING FIX
        return log_q_table[qi] + 1.0 + log(1.0 + r / (q * PHI)) / LN_PHI;
    };

    cout << "========================================\n";
    cout << "  φ-PURE STRESS V2 — WITH FIX\n";
    cout << "========================================\n\n";

    int pass = 0;
    int fail = 0;

    // TEST 1: (F_5 + F_3) × F_4 = 21
    cout << "--- 1. ADDITION + MULTIPLICATION ---\n\n";
    {
        auto ct_a = encrypt_full(5);
        auto ct_b = encrypt_full(3);
        auto ct_add = cc->EvalAdd(ct_a, ct_b);
        auto v_add = decrypt_full(ct_add);
        double log_F = bridge(v_add[1], v_add[0]);
        auto ct_mult = cc->EvalAdd(encrypt_log_only(log_F), encrypt_log_only(log(3.0)/LN_PHI));
        double result = pow(PHI, get_log(ct_mult));
        cout << "  (5+2)×3 = " << result << " (Expected: 21) ";
        if (abs(result - 21.0) < 0.5) { cout << "✅\n"; pass++; } else { cout << "❌\n"; fail++; }
    }

    // TEST 2: (F_6 - F_3) × F_5 = 30
    cout << "\n--- 2. SUBTRACTION + MULTIPLICATION ---\n\n";
    {
        auto ct_a = encrypt_full(6);
        auto ct_b = encrypt_full(3);
        auto ct_sub = cc->EvalSub(ct_a, ct_b);
        auto v_sub = decrypt_full(ct_sub);
        double log_F = bridge(v_sub[1], v_sub[0]);
        auto ct_mult = cc->EvalAdd(encrypt_log_only(log_F), encrypt_log_only(log(5.0)/LN_PHI));
        double result = pow(PHI, get_log(ct_mult));
        cout << "  (8-2)×5 = " << result << " (Expected: 30) ";
        if (abs(result - 30.0) < 0.5) { cout << "✅\n"; pass++; } else { cout << "❌\n"; fail++; }
    }

    // TEST 3: MIXED WITH DIVISION
    cout << "\n--- 3. MIXED WITH DIVISION ---\n\n";
    {
        auto ct_a = encrypt_full(7);
        auto ct_b = encrypt_full(5);
        auto ct_add = cc->EvalAdd(ct_a, ct_b);
        auto v_add = decrypt_full(ct_add);
        double log_F = bridge(v_add[1], v_add[0]);
        auto ct_mult = cc->EvalAdd(encrypt_log_only(log_F), encrypt_log_only(log(3.0)/LN_PHI));
        auto ct_div = cc->EvalSub(ct_mult, encrypt_log_only(log(2.0)/LN_PHI));
        double result = pow(PHI, get_log(ct_div));
        cout << "  (13+5)×3÷2 = " << result << " (Expected: 27) ";
        if (abs(result - 27.0) < 0.5) { cout << "✅\n"; pass++; } else { cout << "❌\n"; fail++; }
    }

    // TEST 4: ARBITRARY VALUE
    cout << "\n--- 4. ARBITRARY VALUE ---\n\n";
    {
        double F = 100.0;
        double q = floor(F / PHI);
        double r = fmod(F, PHI);
        double log_F = bridge(q, r);
        double log_F_exact = log(100.0) / LN_PHI;
        cout << "  F=100: bridge=" << log_F << " exact=" << log_F_exact << " ";
        if (abs(log_F - log_F_exact) < 0.01) { cout << "✅\n"; pass++; } else { cout << "❌\n"; fail++; }
    }

    // TEST 5: SCALE TEST
    cout << "\n--- 5. SCALE TEST ---\n\n";
    {
        double log_a = log((double)fib[20]) / LN_PHI;
        double log_b = log((double)fib[15]) / LN_PHI;
        double log_c = log((double)fib[10]) / LN_PHI;
        auto ct_mult = cc->EvalAdd(encrypt_log_only(log_a), encrypt_log_only(log_b));
        auto ct_div = cc->EvalSub(ct_mult, encrypt_log_only(log_c));
        double result = pow(PHI, get_log(ct_div));
        double expected = (double)fib[20] * (double)fib[15] / (double)fib[10];
        cout << "  F_20×F_15÷F_10 = " << result << " (Expected: " << expected << ") ";
        if (abs(result - expected) < expected * 0.01) { cout << "✅\n"; pass++; } else { cout << "❌\n"; fail++; }
    }

    cout << "\n========================================\n";
    cout << "  STRESS TEST RESULT\n";
    cout << "========================================\n";
    cout << "  Pass: " << pass << "\n";
    cout << "  Fail: " << fail << "\n\n";
    cout << "  " << (fail == 0 ? "✅ LAHAT PUMASA" : "❌ MAY BUMAGSAK") << "\n\n";

    return 0;
}
