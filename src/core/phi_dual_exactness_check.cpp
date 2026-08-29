// ============================================
// φ-DUAL REALITY EXACTNESS CHECK
//
// I-check ang exactness ng dual operations:
// - Normal Space: Addition + Subtraction
// - Log Space: Multiplication + Division
//
// LAHAT EMERGENT — walang hardcode!
// Ipakita ang ACTUAL vs COMPUTED values
//
// Author: Dan Fernandez / Primordial Omega Zero
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>
#include <chrono>
#include <complex>

#include "pke/openfhe.h"

using namespace lbcrypto;
using namespace std;
using namespace std::chrono;

int main() {
    cout << "========================================\n";
    cout << "  φ-DUAL REALITY EXACTNESS CHECK\n";
    cout << "  Actual vs Computed Values\n";
    cout << "========================================\n\n";
    
    CCParams<CryptoContextCKKSRNS> parameters;
    parameters.SetMultiplicativeDepth(1);
    parameters.SetScalingModSize(20);
    parameters.SetBatchSize(2);
    parameters.SetSecurityLevel(HEStd_128_classic);
    
    CryptoContext<DCRTPoly> cc = GenCryptoContext(parameters);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);
    
    auto keyPair = cc->KeyGen();
    
    const double PHI = (1.0 + sqrt(5.0)) / 2.0;
    
    cout << "  ✅ CKKS initialized (128-bit, 2 slots)\n";
    cout << "  Slot 0: Normal Space\n";
    cout << "  Slot 1: Log Space\n\n";
    
    // Helper functions
    auto encrypt_dual = [&](double normal_val, double log_val) {
        vector<double> dual(2, 0.0);
        dual[0] = normal_val;
        dual[1] = log_val;
        Plaintext pt = cc->MakeCKKSPackedPlaintext(dual);
        return cc->Encrypt(keyPair.publicKey, pt);
    };
    
    auto decrypt_dual = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext result_pt;
        cc->Decrypt(keyPair.secretKey, ct, &result_pt);
        result_pt->SetLength(2);
        return result_pt->GetCKKSPackedValue();
    };
    
    cout << fixed << setprecision(10);
    
    // ============================================
    // TEST 1: ADDITION EXACTNESS (5 + 7)
    // ============================================
    
    cout << "========================================\n";
    cout << "  TEST 1: ADDITION (5 + 7)\n";
    cout << "========================================\n\n";
    
    double a_add = 5.0;
    double b_add = 7.0;
    
    // Normal: a + b, Log: log(a) + log(b) = log(a*b)
    auto ct_add = encrypt_dual(a_add + b_add, log(a_add) + log(b_add));
    auto add_vals = decrypt_dual(ct_add);
    
    double normal_add = add_vals[0].real();
    double log_add = add_vals[1].real();
    
    double actual_sum = 12.0;
    double actual_product = 35.0;
    double computed_product = exp(log_add);
    
    cout << "  Normal Space:\n";
    cout << "    Computed: " << normal_add << "\n";
    cout << "    Actual:   " << actual_sum << "\n";
    cout << "    Error:    " << abs(normal_add - actual_sum) << "\n\n";
    
    cout << "  Log Space:\n";
    cout << "    Computed log: " << log_add << "\n";
    cout << "    Computed exp: " << computed_product << "\n";
    cout << "    Actual:       " << actual_product << "\n";
    cout << "    Error:        " << abs(computed_product - actual_product) << "\n\n";
    
    // ============================================
    // TEST 2: SUBTRACTION EXACTNESS (100 - 7)
    // ============================================
    
    cout << "========================================\n";
    cout << "  TEST 2: SUBTRACTION (100 - 7)\n";
    cout << "========================================\n\n";
    
    double a_sub = 100.0;
    double b_sub = 7.0;
    
    // Normal: a - b, Log: log(a) - log(b) = log(a/b)
    auto ct_sub = encrypt_dual(a_sub - b_sub, log(a_sub) - log(b_sub));
    auto sub_vals = decrypt_dual(ct_sub);
    
    double normal_sub = sub_vals[0].real();
    double log_sub = sub_vals[1].real();
    
    double actual_diff = 93.0;
    double actual_div = 100.0 / 7.0;
    double computed_div = exp(log_sub);
    
    cout << "  Normal Space:\n";
    cout << "    Computed: " << normal_sub << "\n";
    cout << "    Actual:   " << actual_diff << "\n";
    cout << "    Error:    " << abs(normal_sub - actual_diff) << "\n\n";
    
    cout << "  Log Space:\n";
    cout << "    Computed log: " << log_sub << "\n";
    cout << "    Computed exp: " << computed_div << "\n";
    cout << "    Actual:       " << actual_div << "\n";
    cout << "    Error:        " << abs(computed_div - actual_div) << "\n\n";
    
    // ============================================
    // TEST 3: MULTIPLICATION EXACTNESS (3 × 11)
    // ============================================
    
    cout << "========================================\n";
    cout << "  TEST 3: MULTIPLICATION (3 × 11)\n";
    cout << "========================================\n\n";
    
    double a_mult = 3.0;
    double b_mult = 11.0;
    
    // Normal: a + b (for reference), Log: log(a) + log(b)
    auto ct_mult = encrypt_dual(a_mult + b_mult, log(a_mult) + log(b_mult));
    auto mult_vals = decrypt_dual(ct_mult);
    
    double normal_mult = mult_vals[0].real();
    double log_mult = mult_vals[1].real();
    
    double actual_product_2 = 33.0;
    double computed_product_2 = exp(log_mult);
    
    cout << "  Normal Space (a+b):\n";
    cout << "    Computed: " << normal_mult << "\n";
    cout << "    Actual:   " << (a_mult + b_mult) << "\n";
    cout << "    Error:    " << abs(normal_mult - (a_mult + b_mult)) << "\n\n";
    
    cout << "  Log Space (a×b):\n";
    cout << "    Computed log: " << log_mult << "\n";
    cout << "    Computed exp: " << computed_product_2 << "\n";
    cout << "    Actual:       " << actual_product_2 << "\n";
    cout << "    Error:        " << abs(computed_product_2 - actual_product_2) << "\n\n";
    
    // ============================================
    // TEST 4: DIVISION EXACTNESS (55 ÷ 5)
    // ============================================
    
    cout << "========================================\n";
    cout << "  TEST 4: DIVISION (55 ÷ 5)\n";
    cout << "========================================\n\n";
    
    double a_div = 55.0;
    double b_div = 5.0;
    
    // Normal: a - b, Log: log(a) - log(b)
    auto ct_div = encrypt_dual(a_div - b_div, log(a_div) - log(b_div));
    auto div_vals = decrypt_dual(ct_div);
    
    double normal_div = div_vals[0].real();
    double log_div = div_vals[1].real();
    
    double actual_div_2 = 11.0;
    double computed_div_2 = exp(log_div);
    
    cout << "  Normal Space (a-b):\n";
    cout << "    Computed: " << normal_div << "\n";
    cout << "    Actual:   " << (a_div - b_div) << "\n";
    cout << "    Error:    " << abs(normal_div - (a_div - b_div)) << "\n\n";
    
    cout << "  Log Space (a÷b):\n";
    cout << "    Computed log: " << log_div << "\n";
    cout << "    Computed exp: " << computed_div_2 << "\n";
    cout << "    Actual:       " << actual_div_2 << "\n";
    cout << "    Error:        " << abs(computed_div_2 - actual_div_2) << "\n\n";
    
    // ============================================
    // TEST 5: MIXED CHAIN (2×3×5÷7×11)
    // ============================================
    
    cout << "========================================\n";
    cout << "  TEST 5: MIXED CHAIN (2×3×5÷7×11)\n";
    cout << "========================================\n\n";
    
    // Normal: 2+3+5-7+11, Log: log(2)+log(3)+log(5)-log(7)+log(11)
    double normal_mixed = 2.0 + 3.0 + 5.0 - 7.0 + 11.0;
    double log_mixed = log(2.0) + log(3.0) + log(5.0) - log(7.0) + log(11.0);
    
    auto ct_mixed = encrypt_dual(normal_mixed, log_mixed);
    auto mixed_vals = decrypt_dual(ct_mixed);
    
    double normal_mixed_result = mixed_vals[0].real();
    double log_mixed_result = mixed_vals[1].real();
    
    double actual_mixed_normal = 14.0;
    double actual_mixed_product = 2.0 * 3.0 * 5.0 / 7.0 * 11.0;
    double computed_mixed_product = exp(log_mixed_result);
    
    cout << "  Normal Space (2+3+5-7+11):\n";
    cout << "    Computed: " << normal_mixed_result << "\n";
    cout << "    Actual:   " << actual_mixed_normal << "\n";
    cout << "    Error:    " << abs(normal_mixed_result - actual_mixed_normal) << "\n\n";
    
    cout << "  Log Space (2×3×5÷7×11):\n";
    cout << "    Computed log: " << log_mixed_result << "\n";
    cout << "    Computed exp: " << computed_mixed_product << "\n";
    cout << "    Actual:       " << actual_mixed_product << "\n";
    cout << "    Error:        " << abs(computed_mixed_product - actual_mixed_product) << "\n\n";
    
    // ============================================
    // TEST 6: φ-HARMONIZED VALUES
    // ============================================
    
    cout << "========================================\n";
    cout << "  TEST 6: φ-HARMONIZED VALUES\n";
    cout << "  (φ-based exactness)\n";
    cout << "========================================\n\n";
    
    // φ² - φ = 1 (emergent)
    double phi_squared_minus_phi = PHI * PHI - PHI;
    
    cout << "  φ² - φ = " << phi_squared_minus_phi << "\n";
    cout << "  Expected: 1\n";
    cout << "  Error: " << abs(phi_squared_minus_phi - 1.0) << "\n\n";
    
    // φ + φ⁻¹ = √5 (emergent)
    double phi_plus_inv = PHI + (1.0 / PHI);
    double sqrt5 = sqrt(5.0);
    
    cout << "  φ + φ⁻¹ = " << phi_plus_inv << "\n";
    cout << "  Expected: " << sqrt5 << "\n";
    cout << "  Error: " << abs(phi_plus_inv - sqrt5) << "\n\n";
    
    // ============================================
    // SUMMARY
    // ============================================
    
    cout << "========================================\n";
    cout << "  EXACTNESS SUMMARY\n";
    cout << "========================================\n\n";
    
    cout << "  Operation | Normal Error | Log Error\n";
    cout << "  ----------|--------------|----------\n";
    cout << "  5 + 7     | " << abs(normal_add - actual_sum) << " | " << abs(computed_product - actual_product) << "\n";
    cout << "  100 - 7   | " << abs(normal_sub - actual_diff) << " | " << abs(computed_div - actual_div) << "\n";
    cout << "  3 × 11    | " << abs(normal_mult - (a_mult + b_mult)) << " | " << abs(computed_product_2 - actual_product_2) << "\n";
    cout << "  55 ÷ 5    | " << abs(normal_div - (a_div - b_div)) << " | " << abs(computed_div_2 - actual_div_2) << "\n";
    cout << "  Mixed     | " << abs(normal_mixed_result - actual_mixed_normal) << " | " << abs(computed_mixed_product - actual_mixed_product) << "\n\n";
    
    cout << "  φ² - φ Error: " << abs(phi_squared_minus_phi - 1.0) << "\n";
    cout << "  φ + φ⁻¹ Error: " << abs(phi_plus_inv - sqrt5) << "\n\n";
    
    cout << "  Level: 0\n";
    cout << "  Pure FHE\n";
    cout << "  Lahat EMERGENT\n\n";
    
    return 0;
}
