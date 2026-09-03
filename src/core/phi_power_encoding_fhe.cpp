// ============================================
// φ-POWER ENCODING FHE
// 4-Slot: [φ^n, φ^(n-1), φ^(n+1), n]
// Addition: Slot 0 + Slot 1 = Slot 2
// Multiplication: Slot 3 + Slot 3 = m+n
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
    cout << "  φ-POWER ENCODING FHE\n";
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

    cout << "  ✅ CKKS initialized (depth 2, 4 slots)\n\n";

    // ============================================
    // ENCODING
    // Slot 0: φ^n
    // Slot 1: φ^(n-1)
    // Slot 2: φ^(n+1)
    // Slot 3: n
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
    // ADDITION TEST
    // ============================================

    cout << "========================================\n";
    cout << "  ADDITION: φ^m + φ^(m-1)\n";
    cout << "========================================\n\n";

    for (int m : {2, 3, 4, 5}) {
        auto ct_m = encrypt_power(m);
        auto ct_m1 = encrypt_power(m - 1);
        
        auto ct_sum = cc->EvalAdd(ct_m, ct_m1);
        auto sum_vals = decrypt_power(ct_sum);
        
        // Expected: Slot 2 ng ct_m = φ^(m+1)
        auto expected_vals = decrypt_power(ct_m);
        
        cout << "  m=" << m << ": "
             << "Slot 0 = " << sum_vals[0]
             << " | Slot 2 = " << sum_vals[2]
             << " | Expected φ^" << (m+1) << " = " << expected_vals[2]
             << " | " << (abs(sum_vals[2] - expected_vals[2]) < 0.01 ? "✅" : "❌") << "\n";
    }

    // ============================================
    // MULTIPLICATION TEST
    // ============================================

    cout << "\n========================================\n";
    cout << "  MULTIPLICATION: φ^m × φ^n\n";
    cout << "========================================\n\n";

    for (int m : {2, 3}) {
        for (int n : {2, 3}) {
            auto ct_m = encrypt_power(m);
            auto ct_n = encrypt_power(n);
            
            auto ct_mult = cc->EvalAdd(ct_m, ct_n);  // index add
            auto mult_vals = decrypt_power(ct_mult);
            
            double expected_index = m + n;
            
            cout << "  m=" << m << ", n=" << n << ": "
                 << "index = " << mult_vals[3]
                 << " | Expected = " << expected_index
                 << " | " << (abs(mult_vals[3] - expected_index) < 0.01 ? "✅" : "❌") << "\n";
        }
    }

    cout << "\n  Level: " << decrypt_power(encrypt_power(2.0))[0] << "\n\n";

    return 0;
}
