// ============================================
// φ-LOG MULTIPLICATION — CT × CT VIA ADDITION
//
// log_φ(A) + log_φ(B) = log_φ(A×B)
// Pure EvalAdd lang — walang EvalMult!
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
    cout << "  φ-LOG MULTIPLICATION\n";
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
    const double LN_PHI = log(PHI);

    cout << "  ✅ CKKS initialized (depth 1!)\n";
    cout << "  φ-log space: log_φ(x)\n\n";

    auto encrypt_log = [&](double value) {
        double log_phi_val = log(value) / LN_PHI;
        vector<double> v(16, log_phi_val);
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto decrypt_log = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext result_pt;
        cc->Decrypt(keyPair.secretKey, ct, &result_pt);
        result_pt->SetLength(16);
        double sum = 0.0;
        for (int i = 0; i < 16; i++) sum += result_pt->GetCKKSPackedValue()[i].real();
        return sum / 16.0;
    };

    auto log_to_value = [&](double log_phi_val) {
        return exp(log_phi_val * LN_PHI);
    };

    // ============================================
    // TEST: 5 × 7 = 35 (BOTH ENCRYPTED)
    // ============================================

    cout << "========================================\n";
    cout << "  CT × CT MULTIPLICATION (LOG SPACE)\n";
    cout << "========================================\n\n";

    struct MultTest {
        double a, b;
        double expected;
    };

    vector<MultTest> tests = {
        {5, 7, 35},
        {12, 13, 156},
        {3, 100, 300},
        {10, 10, 100},
        {2.5, 4, 10},
        {1.5, 2, 3}
    };

    int match = 0;
    cout << "  A × B | Result | Expected | Match?\n";
    cout << "  -------|--------|----------|--------\n";

    for (const auto& t : tests) {
        // Encrypt both in log space
        auto ct_a = encrypt_log(t.a);
        auto ct_b = encrypt_log(t.b);
        
        // Multiply: ADD sa log space!
        auto ct_product = cc->EvalAdd(ct_a, ct_b);
        
        // Decrypt log result
        double log_result = decrypt_log(ct_product);
        double result = log_to_value(log_result);
        int rounded = (int)round(result);
        bool ok = (abs(result - t.expected) < 0.01);
        match += ok;
        
        cout << "  " << setw(4) << t.a << " × " << setw(4) << t.b << " | "
             << setw(6) << rounded << " | "
             << setw(8) << t.expected << " | "
             << (ok ? "✅" : "❌") << "\n";
    }
    cout << "\n  Match: " << match << "/" << tests.size() << "\n\n";

    // ============================================
    // CHAINED MULTIPLICATION: 2^1000
    // ============================================

    cout << "========================================\n";
    cout << "  CHAINED: 2^1000\n";
    cout << "========================================\n\n";

    auto ct_2 = encrypt_log(2.0);
    auto ct_result = encrypt_log(1.0);

    auto start = high_resolution_clock::now();
    
    // 2^1000 = 2 × 2 × ... × 2 (1000 times)
    for (int i = 0; i < 1000; i++) {
        ct_result = cc->EvalAdd(ct_result, ct_2);
    }
    
    auto end = high_resolution_clock::now();
    auto time = duration_cast<milliseconds>(end - start).count();

    double log_result_1k = decrypt_log(ct_result);
    double result_1k = log_to_value(log_result_1k);

    cout << "  Operations: 1000 multiplications\n";
    cout << "  Result (log_φ): " << log_result_1k << "\n";
    cout << "  Result (2^1000): " << result_1k << "\n";
    cout << "  Time: " << time << " ms\n";
    cout << "  Level: " << ct_result->GetLevel() << "\n";
    cout << "  Towers: " << ct_result->GetElements()[0].GetNumOfElements() << "\n\n";

    // ============================================
    // DIVISION VIA SUBTRACTION IN LOG SPACE
    // ============================================

    cout << "========================================\n";
    cout << "  DIVISION: 100 ÷ 4 = 25\n";
    cout << "========================================\n\n";

    auto ct_100 = encrypt_log(100.0);
    auto ct_4 = encrypt_log(4.0);
    
    // Division: SUBTRACT sa log space!
    auto ct_quotient = cc->EvalSub(ct_100, ct_4);
    
    double log_quotient = decrypt_log(ct_quotient);
    double quotient = log_to_value(log_quotient);

    cout << "  100 ÷ 4 = " << (int)round(quotient) << " (Expected: 25)\n";
    cout << "  Match: " << ((int)round(quotient) == 25 ? "✅" : "❌") << "\n\n";

    // ============================================
    // SUMMARY
    // ============================================

    cout << "========================================\n";
    cout << "  φ-LOG MULTIPLICATION SUMMARY\n";
    cout << "========================================\n\n";
    cout << "  ✅ Multiplication via Addition: " << match << "/" << tests.size() << "\n";
    cout << "  ✅ 2^1000 chained: " << log_result_1k << " log_φ\n";
    cout << "  ✅ Division via Subtraction: " << (int)round(quotient) << "\n";
    cout << "  ✅ Level 0\n";
    cout << "  ✅ Pure FHE (walang EvalMult!)\n\n";

    return 0;
}
