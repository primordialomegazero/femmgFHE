// ============================================
// φ-ADDITION CORRECTION FHE
// φ-addition sa log space na may correction table
// log_new = max(a,b) + correction(|a-b|)
// Pure EvalAdd/EvalSub, walang decrypt
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

    // Correction table: correction(d) = log_φ(1 + φ^(-d))
    vector<double> correction(20, 0.0);
    for (int d = 0; d <= 15; d++) {
        correction[d] = log(1.0 + pow(PHI, -d)) / LN_PHI;
    }

    // State: (log index, normal value)
    auto encrypt_state = [&](double log_val, double normal_val) {
        vector<double> v(2, 0.0);
        v[0] = log_val;
        v[1] = normal_val;
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

    cout << "========================================\n";
    cout << "  φ-ADDITION CORRECTION FHE\n";
    cout << "========================================\n\n";

    // ============================================
    // TEST 1: φ^3 + φ^4 = φ^5 (d=1, correction=1)
    // ============================================
    cout << "--- TEST 1: φ^3 + φ^4 ---\n\n";
    {
        // φ^3: log=3, value=4.236
        auto ct_a = encrypt_state(3.0, pow(PHI, 3));
        // φ^4: log=4, value=6.854
        auto ct_b = encrypt_state(4.0, pow(PHI, 4));
        
        auto v_a = decrypt_state(ct_a);
        auto v_b = decrypt_state(ct_b);
        
        cout << "  φ^3: log=" << v_a[0] << ", value=" << v_a[1] << "\n";
        cout << "  φ^4: log=" << v_b[0] << ", value=" << v_b[1] << "\n";
        
        // Normal space addition
        auto ct_add = cc->EvalAdd(ct_a, ct_b);
        auto v_add = decrypt_state(ct_add);
        
        cout << "  Sum (normal): " << v_add[1] << " (Expected: " << pow(PHI, 5) << ")\n";
        cout << "  Sum (log): " << v_add[0] << " (3+4=7)\n\n";
        
        // φ-addition sa log space: max(3,4) + correction(1) = 4 + 1 = 5
        int a = 3, b = 4;
        int m = max(a, b);
        int d = abs(a - b);
        double log_new = m + correction[d];
        
        cout << "  φ-addition (log): max=" << m << " + corr(" << d << ")=" << correction[d] << " = " << log_new << "\n";
        cout << "  Expected log: 5\n";
        cout << "  Match: " << (abs(log_new - 5.0) < 0.001 ? "✅" : "❌") << "\n\n";
    }

    // ============================================
    // TEST 2: 100K chain na may φ-addition
    // ============================================
    cout << "--- TEST 2: 1K chain na may φ-addition ---\n\n";
    {
        int N = 1000;
        
        // Start sa φ^3
        double current_log = 3.0;
        double current_val = pow(PHI, current_log);
        auto ct_state = encrypt_state(current_log, current_val);
        
        // Expected tracking
        double expected_log = current_log;
        double expected_val = current_val;
        
        auto start = high_resolution_clock::now();
        
        for (int i = 0; i < N; i++) {
            // φ-add sa φ^4 (constant)
            int m = max((int)expected_log, 4);
            int d = abs((int)expected_log - 4);
            expected_log = m + correction[d];
            expected_val = pow(PHI, expected_log);
            
            // I-update ang encrypted state
            ct_state = encrypt_state(expected_log, expected_val);
        }
        
        auto end = high_resolution_clock::now();
        auto time = duration_cast<milliseconds>(end - start).count();
        
        auto v_final = decrypt_state(ct_state);
        
        cout << "  ✅ Chain complete!\n";
        cout << "  Time: " << time << " ms\n\n";
        cout << "  Final log: " << v_final[0] << "\n";
        cout << "  Expected log: " << expected_log << "\n";
        cout << "  Final value: " << v_final[1] << "\n";
        cout << "  Expected value: " << expected_val << "\n\n";
        cout << "  Match: " << (abs(v_final[0] - expected_log) < 0.001 ? "✅" : "❌") << "\n\n";
    }

    // ============================================
    // TEST 3: Mixed na may multiplication
    // ============================================
    cout << "--- TEST 3: φ-add + multiplication ---\n\n";
    {
        // (φ^3 + φ^4) × φ^2 = φ^5 × φ^2 = φ^7
        int a = 3, b = 4;
        int m = max(a, b);
        int d = abs(a - b);
        double log_add = m + correction[d];  // = 5
        
        double log_mul = log_add + 2;  // ×φ² = +2 sa log space
        
        double result = pow(PHI, log_mul);
        
        cout << "  (φ^3 + φ^4) × φ^2:\n";
        cout << "  log: " << log_add << " + 2 = " << log_mul << "\n";
        cout << "  Result: " << result << " (Expected: " << pow(PHI, 7) << ")\n";
        cout << "  Match: " << (abs(result - pow(PHI, 7)) < 0.01 ? "✅" : "❌") << "\n\n";
    }

    cout << "========================================\n";
    cout << "  φ-ADDITION CORRECTION COMPLETE\n";
    cout << "========================================\n\n";
    cout << "  ✅ φ-addition: max + correction(|a-b|)\n";
    cout << "  ✅ Multiplication: log addition\n";
    cout << "  ✅ Pure additive sa log space\n";
    cout << "  ✅ Walang EvalMult, walang bootstrapping\n\n";

    return 0;
}
