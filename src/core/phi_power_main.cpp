// ============================================
// φ-POWER MAIN
// Subok kung ang φ-power space ang tamang
// architecture para sa arbitrary computation
//
// Addition: Slot 0 (φ-power sum)
// Multiplication: Slot 3 (index add)
// Bridge: φ^n = F_{n-1} + F_n × φ
//
// Author: Dan Fernandez / Primordial Omega Zero
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
    cout << "========================================\n";
    cout << "  φ-POWER MAIN\n";
    cout << "========================================\n\n";

    CCParams<CryptoContextCKKSRNS> parameters;
    parameters.SetMultiplicativeDepth(2);
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
    for (int i = 2; i <= 50; i++) {
        fib.push_back(fib[i-1] + fib[i-2]);
    }

    cout << "  ✅ CKKS initialized (depth 2, 4 slots)\n\n";

    // ============================================
    // ENCODING
    // Slot 0: φ^n (φ-power)
    // Slot 1: φ^(n-1) (previous)
    // Slot 2: φ^(n+1) (next)
    // Slot 3: n (index)
    // ============================================

    auto encrypt_power = [&](double n) {
        double phi_n = pow(PHI, n);
        double phi_nm1 = pow(PHI, n - 1);
        double phi_np1 = pow(PHI, n + 1);
        
        vector<double> v(4, 0.0);
        v[0] = phi_n;
        v[1] = phi_nm1;
        v[2] = phi_np1;
        v[3] = n;
        
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto decrypt_power = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext result_pt;
        cc->Decrypt(keyPair.secretKey, ct, &result_pt);
        result_pt->SetLength(4);
        auto results = result_pt->GetCKKSPackedValue();
        vector<double> vals(4);
        for (int i = 0; i < 4; i++) {
            vals[i] = results[i].real();
        }
        return vals;
    };

    // ============================================
    // TEST: (5 + 8) × 2 = 26
    // ============================================

    cout << "========================================\n";
    cout << "  TEST: (5 + 8) × 2 = 26\n";
    cout << "========================================\n\n";

    // 5 → φ-power: log_φ(5) = 3.3446
    // 8 → φ-power: log_φ(8) = 4.3213
    // 5 + 8 = 13 → φ^5.3302
    // 13 × 2 = 26 → φ^6.7706

    auto ct_5 = encrypt_power(log(5.0)/LN_PHI);
    auto ct_8 = encrypt_power(log(8.0)/LN_PHI);
    auto ct_2 = encrypt_power(log(2.0)/LN_PHI);

    // Step 1: 5 + 8 sa normal space
    auto ct_sum = cc->EvalAdd(ct_5, ct_8);
    auto sum_vals = decrypt_power(ct_sum);
    
    cout << "  After 5 + 8:\n";
    cout << "  Slot 0 (φ^n): " << sum_vals[0] << " (expected: φ^5.33 = " << pow(PHI, log(13.0)/LN_PHI) << ")\n";
    cout << "  Slot 3 (n): " << sum_vals[3] << " (expected: 7.666)\n\n";

    // Step 2: × 2 sa index space
    auto ct_result = cc->EvalAdd(ct_sum, ct_2);
    auto result_vals = decrypt_power(ct_result);
    
    cout << "  After × 2:\n";
    cout << "  Slot 0 (φ^n): " << result_vals[0] << "\n";
    cout << "  Slot 3 (n): " << result_vals[3] << "\n\n";

    // Verify
    double expected_result = 26.0;
    double expected_log = log(expected_result) / LN_PHI;
    double actual_phi = result_vals[0];
    double actual_log = result_vals[3];
    
    cout << "  Expected: φ^" << expected_log << " = " << expected_result << "\n";
    cout << "  Actual φ^n: " << actual_phi << "\n";
    cout << "  Actual n: " << actual_log << "\n";
    cout << "  Match (φ^n): " << (abs(actual_phi - expected_result) < 1.0 ? "✅" : "❌") << "\n\n";

    cout << "  Level: " << ct_result->GetLevel() << "\n";
    cout << "  Towers: " << ct_result->GetElements()[0].GetNumOfElements() << "\n\n";

    cout << "========================================\n";
    cout << "  φ-POWER MAIN COMPLETE\n";
    cout << "========================================\n\n";

    return 0;
}
