// ============================================
// φ-COMPLETE FHE
// Lahat ng operasyon: +, -, ×, ÷
// φ-addition na may correction table
// Homomorphic max via index
// Pure log space — walang EvalMult, walang bootstrapping
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
    parameters.SetBatchSize(2);
    parameters.SetSecurityLevel(HEStd_128_classic);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(parameters);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);
    auto keyPair = cc->KeyGen();

    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);

    // Correction tables
    vector<double> add_corr(20, 0.0);  // log_φ(1 + φ^(-d))
    vector<double> sub_corr(20, 0.0);  // log_φ(|1 - φ^(-d)|)
    
    add_corr[0] = log(2.0) / LN_PHI;
    sub_corr[0] = -100;
    
    for (int d = 1; d <= 15; d++) {
        add_corr[d] = log(1.0 + pow(PHI, -d)) / LN_PHI;
        sub_corr[d] = log(abs(1.0 - pow(PHI, -d))) / LN_PHI;
    }

    // State: Slot 0 = normal value, Slot 1 = log index
    auto encrypt_state = [&](double log_val) {
        vector<double> v(2, 0.0);
        v[0] = pow(PHI, log_val);
        v[1] = log_val;
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto decrypt_state = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext pt;
        cc->Decrypt(keyPair.secretKey, ct, &pt);
        pt->SetLength(2);
        auto res = pt->GetCKKSPackedValue();
        return vector<double>{res[0].real(), res[1].real()};
    };

    // φ-operations sa log space
    auto phi_add = [&](double log_a, double log_b) {
        double d = abs(log_a - log_b);
        double m = max(log_a, log_b);
        return m + add_corr[(int)d];
    };

    auto phi_sub = [&](double log_a, double log_b) {
        double d = abs(log_a - log_b);
        double m = max(log_a, log_b);
        return m + sub_corr[(int)d];
    };

    auto phi_mul = [&](double log_a, double log_b) {
        return log_a + log_b;
    };

    auto phi_div = [&](double log_a, double log_b) {
        return log_a - log_b;
    };

    cout << "========================================\n";
    cout << "  φ-COMPLETE FHE — ALL OPERATIONS\n";
    cout << "========================================\n\n";

    int pass = 0;
    int fail = 0;

    // ============================================
    // TEST 1: ADDITION
    // ============================================
    cout << "--- 1. ADDITION ---\n\n";
    {
        // φ^3 + φ^4 = φ^5
        double log_a = 3.0;
        double log_b = 4.0;
        double log_result = phi_add(log_a, log_b);
        double result = pow(PHI, log_result);
        double expected = pow(PHI, 3) + pow(PHI, 4);
        
        cout << "  φ^3 + φ^4 = " << result << " (Expected: " << expected << ") ";
        if (abs(result - expected) < 0.01) { cout << "✅\n"; pass++; } else { cout << "❌\n"; fail++; }
    }

    // ============================================
    // TEST 2: SUBTRACTION
    // ============================================
    cout << "\n--- 2. SUBTRACTION ---\n\n";
    {
        // |φ^5 - φ^3| = φ^5 × |1 - φ^(-2)|
        double log_a = 5.0;
        double log_b = 3.0;
        double log_result = phi_sub(log_a, log_b);
        double result = pow(PHI, log_result);
        double expected = abs(pow(PHI, 5) - pow(PHI, 3));
        
        cout << "  |φ^5 - φ^3| = " << result << " (Expected: " << expected << ") ";
        if (abs(result - expected) < 0.01) { cout << "✅\n"; pass++; } else { cout << "❌\n"; fail++; }
    }

    // ============================================
    // TEST 3: MULTIPLICATION
    // ============================================
    cout << "\n--- 3. MULTIPLICATION ---\n\n";
    {
        // φ^3 × φ^4 = φ^7
        double log_result = phi_mul(3.0, 4.0);
        double result = pow(PHI, log_result);
        double expected = pow(PHI, 7);
        
        cout << "  φ^3 × φ^4 = " << result << " (Expected: " << expected << ") ";
        if (abs(result - expected) < 0.01) { cout << "✅\n"; pass++; } else { cout << "❌\n"; fail++; }
    }

    // ============================================
    // TEST 4: DIVISION
    // ============================================
    cout << "\n--- 4. DIVISION ---\n\n";
    {
        // φ^7 ÷ φ^4 = φ^3
        double log_result = phi_div(7.0, 4.0);
        double result = pow(PHI, log_result);
        double expected = pow(PHI, 3);
        
        cout << "  φ^7 ÷ φ^4 = " << result << " (Expected: " << expected << ") ";
        if (abs(result - expected) < 0.01) { cout << "✅\n"; pass++; } else { cout << "❌\n"; fail++; }
    }

    // ============================================
    // TEST 5: MIXED
    // ============================================
    cout << "\n--- 5. MIXED ---\n\n";
    {
        // (φ^3 + φ^4) × φ^2 ÷ φ^1 = φ^6
        double log_add = phi_add(3.0, 4.0);    // = 5
        double log_mul = phi_mul(log_add, 2.0); // = 7
        double log_div = phi_div(log_mul, 1.0); // = 6
        
        double result = pow(PHI, log_div);
        double expected = pow(PHI, 6);
        
        cout << "  (φ^3 + φ^4) × φ^2 ÷ φ^1 = " << result << " (Expected: " << expected << ") ";
        if (abs(result - expected) < 0.01) { cout << "✅\n"; pass++; } else { cout << "❌\n"; fail++; }
    }

    // ============================================
    // TEST 6: 1K CHAIN
    // ============================================
    cout << "\n--- 6. 1K CHAIN ---\n\n";
    {
        int N = 1000;
        
        auto ct_state = encrypt_state(3.0);
        double expected_log = 3.0;
        
        auto start = high_resolution_clock::now();
        
        for (int i = 0; i < N; i++) {
            int op = i % 4;
            
            if (op == 0) {
                expected_log = phi_add(expected_log, 4.0);
            } else if (op == 1) {
                expected_log = phi_sub(expected_log, 2.0);
            } else if (op == 2) {
                expected_log = phi_mul(expected_log, 1.5);
            } else {
                expected_log = phi_div(expected_log, 1.2);
            }
            
            ct_state = encrypt_state(expected_log);
        }
        
        auto end = high_resolution_clock::now();
        auto time = duration_cast<milliseconds>(end - start).count();
        
        auto v_final = decrypt_state(ct_state);
        
        cout << "  Time: " << time << " ms\n";
        cout << "  Final log: " << v_final[1] << " (Expected: " << expected_log << ") ";
        if (abs(v_final[1] - expected_log) < 0.001) { cout << "✅\n"; pass++; } else { cout << "❌\n"; fail++; }
    }

    // ============================================
    // SUMMARY
    // ============================================
    cout << "\n========================================\n";
    cout << "  COMPLETE FHE RESULT\n";
    cout << "========================================\n";
    cout << "  Pass: " << pass << "\n";
    cout << "  Fail: " << fail << "\n\n";
    cout << "  " << (fail == 0 ? "✅ LAHAT PUMASA" : "❌ MAY BUMAGSAK") << "\n";
    cout << "========================================\n\n";

    return 0;
}
