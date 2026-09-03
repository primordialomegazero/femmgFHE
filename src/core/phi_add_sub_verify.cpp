// ============================================
// φ-ADD-SUB VERIFY
// I-verify kung ang addition at subtraction
// ay naconvert nang tama sa φ-space
//
// Ang tanong: kapag nag-EvalAdd tayo sa
// normal space, tugma ba ito sa φ-space?
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
    cout << "  φ-ADD-SUB VERIFY\n";
    cout << "========================================\n\n";

    CCParams<CryptoContextCKKSRNS> parameters;
    parameters.SetMultiplicativeDepth(1);
    parameters.SetScalingModSize(59);
    parameters.SetBatchSize(8);
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

    cout << "  ✅ CKKS initialized (depth 1, modsize 59, 8 slots)\n\n";

    // ============================================
    // DUAL SPACE ENCODING
    // Slot 0: x (normal)
    // Slot 1: n (floor index)
    // Slot 2: frac
    // Slot 3: log_φ(x)
    // ============================================

    auto encrypt_dual = [&](double x) {
        double log_phi_x = log(x) / LN_PHI;
        double n_val = floor(log_phi_x);
        double frac = log_phi_x - n_val;
        int n = (int)n_val;
        
        double phi_frac = pow(PHI, frac);
        double F_nm1 = fib[n-1];
        double F_n = fib[n];
        double phi_n = F_nm1 + F_n * PHI;
        double recon = phi_n * phi_frac;
        
        vector<double> v(8, 0.0);
        v[0] = x;
        v[1] = n_val;
        v[2] = frac;
        v[3] = log_phi_x;
        v[4] = phi_frac;
        v[5] = F_nm1;
        v[6] = F_n;
        v[7] = recon;
        
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto decrypt_dual = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext result_pt;
        cc->Decrypt(keyPair.secretKey, ct, &result_pt);
        result_pt->SetLength(8);
        auto results = result_pt->GetCKKSPackedValue();
        vector<double> vals(8);
        for (int i = 0; i < 8; i++) {
            vals[i] = results[i].real();
        }
        return vals;
    };

    // ============================================
    // TEST 1: ADDITION — 5 + 7 = 12
    // ============================================

    cout << "========================================\n";
    cout << "  TEST 1: ADDITION — 5 + 7\n";
    cout << "========================================\n\n";

    auto ct_5 = encrypt_dual(5.0);
    auto ct_7 = encrypt_dual(7.0);

    auto ct_sum = cc->EvalAdd(ct_5, ct_7);
    auto sum_vals = decrypt_dual(ct_sum);

    cout << "  Slot 0 (normal): " << sum_vals[0] << " (expected: 12)\n";
    cout << "  Slot 1 (n): " << sum_vals[1] << "\n";
    cout << "  Slot 2 (frac): " << sum_vals[2] << "\n";
    cout << "  Slot 3 (log_φ): " << sum_vals[3] << "\n";
    cout << "  Slot 7 (recon): " << sum_vals[7] << "\n\n";

    // ============================================
    // TEST 2: SUBTRACTION — 10 - 3 = 7
    // ============================================

    cout << "========================================\n";
    cout << "  TEST 2: SUBTRACTION — 10 - 3\n";
    cout << "========================================\n\n";

    auto ct_10 = encrypt_dual(10.0);
    auto ct_3 = encrypt_dual(3.0);

    auto ct_diff = cc->EvalSub(ct_10, ct_3);
    auto diff_vals = decrypt_dual(ct_diff);

    cout << "  Slot 0 (normal): " << diff_vals[0] << " (expected: 7)\n";
    cout << "  Slot 1 (n): " << diff_vals[1] << "\n";
    cout << "  Slot 2 (frac): " << diff_vals[2] << "\n";
    cout << "  Slot 3 (log_φ): " << diff_vals[3] << "\n";
    cout << "  Slot 7 (recon): " << diff_vals[7] << "\n\n";

    // ============================================
    // TEST 3: MIXED — (5 × 7) + 3
    // ============================================

    cout << "========================================\n";
    cout << "  TEST 3: MIXED — (5 × 7) + 3\n";
    cout << "========================================\n\n";

    // Step 1: 5 × 7 sa log space
    auto ct_mult = cc->EvalAdd(ct_5, ct_7);
    auto mult_vals = decrypt_dual(ct_mult);
    
    cout << "  After 5 × 7:\n";
    cout << "  Slot 3 (log_φ): " << mult_vals[3] << " (expected: log_φ(35) = 7.3883)\n";
    cout << "  Slot 7 (recon): " << mult_vals[7] << " (expected: 35)\n\n";

    // Step 2: + 3
    auto ct_result = cc->EvalAdd(ct_mult, ct_3);
    auto result_vals = decrypt_dual(ct_result);

    cout << "  After + 3:\n";
    cout << "  Slot 0 (normal): " << result_vals[0] << " (expected: 38)\n";
    cout << "  Slot 3 (log_φ): " << result_vals[3] << "\n";
    cout << "  Slot 7 (recon): " << result_vals[7] << "\n\n";

    cout << "  Level: " << ct_result->GetLevel() << "\n";
    cout << "  Towers: " << ct_result->GetElements()[0].GetNumOfElements() << "\n\n";

    return 0;
}
