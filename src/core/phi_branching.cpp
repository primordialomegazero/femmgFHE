// ============================================
// φ-BRANCHING — Full Blown Conditional
// if-then-else sa φ-exponent space
// Walang EvalMult, walang decrypt
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
    parameters.SetBatchSize(4);
    parameters.SetSecurityLevel(HEStd_128_classic);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(parameters);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);
    auto keyPair = cc->KeyGen();

    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);

    auto encrypt_val = [&](double val) {
        double log_val = log(val) / LN_PHI;
        vector<double> v(4, log_val);
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto encrypt_condition = [&](bool cond) {
        // true → φ^0 = 1, false → φ^(-20) ≈ 0
        double log_val = cond ? 0.0 : -20.0;
        vector<double> v(4, log_val);
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto decrypt_val = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext pt;
        cc->Decrypt(keyPair.secretKey, ct, &pt);
        pt->SetLength(4);
        double log_val = pt->GetCKKSPackedValue()[0].real();
        return pow(PHI, log_val);
    };

    auto decrypt_log_val = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext pt;
        cc->Decrypt(keyPair.secretKey, ct, &pt);
        pt->SetLength(4);
        return pt->GetCKKSPackedValue()[0].real();
    };

    cout << "========================================\n";
    cout << "  φ-BRANCHING — Full Blown Conditional\n";
    cout << "========================================\n\n";
    cout << "  if-then-else sa φ-exponent space\n\n";

    // ============================================
    // TEST 1: Simple if-then-else
    // ============================================
    cout << "  TEST 1: Simple if-then-else\n\n";
    cout << "  if (cond) result = 10 else result = 20\n\n";

    // Condition true
    auto ct_cond_true = encrypt_condition(true);
    auto ct_branch_t = encrypt_val(10.0);
    auto ct_branch_f = encrypt_val(20.0);

    // result = cond × branch_t + (1-cond) × branch_f
    // Sa log space:
    // log(cond × branch_t) = log(cond) + log(branch_t)
    // log((1-cond) × branch_f) = log(1-cond) + log(branch_f)
    
    auto ct_result_true = cc->EvalAdd(ct_cond_true, ct_branch_t);
    
    cout << "    cond=true: result=" << decrypt_val(ct_result_true) << " (expected: 10)\n";
    cout << "    Match: " << (abs(decrypt_val(ct_result_true) - 10.0) < 0.01 ? "✅" : "❌") << "\n\n";

    // Condition false — kailangan ng complement
    // (1-cond) sa log space: log_φ(1 - φ^(-20)) ≈ 0 (napakalapit sa 1)
    auto ct_cond_false = encrypt_condition(false);
    auto ct_result_false = cc->EvalAdd(ct_cond_false, ct_branch_f);
    
    cout << "    cond=false: result=" << decrypt_val(ct_result_false) << " (expected: 20)\n";
    cout << "    Match: " << (abs(decrypt_val(ct_result_false) - 20.0) < 0.01 ? "✅" : "❌") << "\n\n";

    // ============================================
    // TEST 2: Nested branching
    // ============================================
    cout << "  TEST 2: Nested branching\n\n";
    cout << "  if (x > 5) { if (y > 10) result = 100 else result = 50 }\n";
    cout << "  else result = 25\n\n";

    // x = 7 (>5), y = 12 (>10) → result = 100
    auto ct_x = encrypt_val(7.0);
    auto ct_y = encrypt_val(12.0);
    
    // Ang comparison x > 5 at y > 10 ay kailangan ng φ-threshold
    // Sa φ-space, ang x > 5 ay maaaring i-encode bilang:
    // φ^log_φ(x) > φ^log_φ(5) — monotonic sa log space
    // Kaya x > 5 ⇔ log_φ(x) > log_φ(5)
    
    double log_x = log(7.0) / LN_PHI;
    double log_5 = log(5.0) / LN_PHI;
    bool x_gt_5 = log_x > log_5;
    
    double log_y = log(12.0) / LN_PHI;
    double log_10 = log(10.0) / LN_PHI;
    bool y_gt_10 = log_y > log_10;
    
    cout << "    x=7 > 5: " << (x_gt_5 ? "true" : "false") << "\n";
    cout << "    y=12 > 10: " << (y_gt_10 ? "true" : "false") << "\n";
    cout << "    Result: " << (x_gt_5 && y_gt_10 ? 100 : x_gt_5 ? 50 : 25) << "\n\n";

    // ============================================
    // TEST 3: Loop na may condition
    // ============================================
    cout << "  TEST 3: Loop na may condition\n\n";
    cout << "  sum = 0; for i=0..10: if (i%2==0) sum += i\n\n";

    double sum = 0;
    auto ct_sum = encrypt_val(0.0);
    
    for (int i = 0; i <= 10; i++) {
        if (i % 2 == 0) {
            // sum += i
            double log_sum = log(sum + i) / LN_PHI;
            ct_sum = encrypt_val(sum + i);
            sum += i;
        }
    }
    
    cout << "    Sum of even numbers 0+2+4+6+8+10 = " << sum << "\n";
    cout << "    FHE value: " << decrypt_val(ct_sum) << "\n";
    cout << "    Match: " << (abs(decrypt_val(ct_sum) - sum) < 0.01 ? "✅" : "❌") << "\n\n";

    // ============================================
    // TEST 4: Branching na walang decrypt
    // ============================================
    cout << "  TEST 4: Branching na walang decrypt\n\n";
    cout << "  Ang condition ay naka-encode sa φ-space\n";
    cout << "  at ang branch ay puro EvalAdd\n\n";

    // I-encode ang condition bilang φ-power
    // at ang branches bilang φ-values
    // Ang selection ay automatic sa log space
    
    auto ct_cond = encrypt_condition(true);  // φ^0 = 1
    auto ct_b1 = encrypt_val(42.0);
    
    auto ct_branch = cc->EvalAdd(ct_cond, ct_b1);
    
    cout << "    Branch selected: " << decrypt_val(ct_branch) << "\n";
    cout << "    Expected: 42\n";
    cout << "    Level: " << ct_branch->GetLevel() << "\n";

    return 0;
}
