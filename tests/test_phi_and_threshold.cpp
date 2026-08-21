// AND VIA THRESHOLD — 0-LEVEL ATTEMPT
// AND(a,b) = φ² kung a+b >= 2φ², 0 kung hindi
// Subukan natin ang threshold na 0-level

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  AND VIA THRESHOLD — 0-LEVEL\n";
    std::cout << "  Ang Nawawalang Piraso para sa NAND\n";
    std::cout << "========================================\n\n";

    const double phi = 1.6180339887498948482;
    const double phi_sq = phi * phi;
    const double two_phi_sq = 2 * phi_sq;

    CCParams<CryptoContextCKKSRNS> params;
    params.SetMultiplicativeDepth(5);
    params.SetScalingModSize(50);
    params.SetBatchSize(256);
    params.SetFirstModSize(60);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(params);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);

    auto keys = cc->KeyGen();
    cc->EvalMultKeyGen(keys.secretKey);
    auto slots = cc->GetEncodingParams()->GetBatchSize();

    auto make_ct = [&](double val) {
        std::vector<std::complex<double>> vec(slots, {0.0, 0.0});
        vec[0] = {val, 0.0};
        return cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(vec));
    };

    auto decrypt_val = [&](auto ct) {
        Plaintext pt;
        cc->Decrypt(keys.secretKey, ct, &pt);
        return pt->GetCKKSPackedValue()[0].real();
    };

    auto ct_phi_sq = make_ct(phi_sq);
    auto ct_zero = make_ct(0.0);
    auto ct_two_phi_sq = make_ct(two_phi_sq);
    auto ct_three_phi_sq = make_ct(3 * phi_sq);

    // ============================================
    // AND TRUTH TABLE
    // ============================================
    std::cout << "AND TRUTH TABLE:\n";
    std::cout << "================\n";
    std::cout << "  AND(0,0) = 0\n";
    std::cout << "  AND(0,φ²) = 0\n";
    std::cout << "  AND(φ²,0) = 0\n";
    std::cout << "  AND(φ²,φ²) = φ²\n\n";

    // ============================================
    // THRESHOLD APPROACH
    // ============================================
    std::cout << "THRESHOLD APPROACH:\n";
    std::cout << "===================\n\n";
    
    // sum = a + b
    // sum ∈ {0, φ², 2φ²}
    // AND = φ² kung sum >= 2φ², 0 kung hindi
    //
    // Threshold: sum - 2φ²
    // sum=0: -2φ² (negative)
    // sum=φ²: -φ² (negative)
    // sum=2φ²: 0 (zero)
    //
    // AND = φ² kung (sum - 2φ²) >= 0
    // AND = 0 kung (sum - 2φ²) < 0
    
    std::cout << "sum - 2φ²:\n";
    std::cout << "  AND(0,0): 0 - 2φ² = -2φ²\n";
    std::cout << "  AND(0,φ²): φ² - 2φ² = -φ²\n";
    std::cout << "  AND(φ²,φ²): 2φ² - 2φ² = 0\n\n";
    
    // ============================================
    // OSCILLATION PARA SA SIGN DETECTION
    // ============================================
    std::cout << "OSCILLATION SIGN DETECTION:\n";
    std::cout << "===========================\n\n";
    
    // f(x) = φ² - x
    // Para sa negative values, nagiging positive
    // Para sa zero, nagiging φ²
    
    auto test_neg_2phi = make_ct(-two_phi_sq);
    auto test_neg_phi = make_ct(-phi_sq);
    auto test_zero = ct_zero;
    
    std::cout << "f(x) = φ² - x:\n";
    std::cout << "  f(-2φ²) = φ² - (-2φ²) = 3φ²\n";
    std::cout << "  f(-φ²) = φ² - (-φ²) = 2φ²\n";
    std::cout << "  f(0) = φ² - 0 = φ²\n\n";
    
    // Test sa CKKS
    auto osc_neg_2phi = cc->EvalSub(ct_phi_sq, test_neg_2phi);
    auto osc_neg_phi = cc->EvalSub(ct_phi_sq, test_neg_phi);
    auto osc_zero = cc->EvalSub(ct_phi_sq, test_zero);
    
    std::cout << "CKKS Results:\n";
    std::cout << "  f(-2φ²) = " << decrypt_val(osc_neg_2phi) << " (expect 3φ²)\n";
    std::cout << "  f(-φ²) = " << decrypt_val(osc_neg_phi) << " (expect 2φ²)\n";
    std::cout << "  f(0) = " << decrypt_val(osc_zero) << " (expect φ²)\n";
    std::cout << "  Level: " << osc_neg_2phi->GetLevel() << "\n\n";
    
    // ============================================
    // ANG PATTERN
    // ============================================
    std::cout << "ANG PATTERN:\n";
    std::cout << "============\n";
    std::cout << "  sum=0 → -2φ² → f(-2φ²) = 3φ²\n";
    std::cout << "  sum=φ² → -φ² → f(-φ²) = 2φ²\n";
    std::cout << "  sum=2φ² → 0 → f(0) = φ²\n\n";
    
    std::cout << "Lahat ay MULTIPLES ng φ²:\n";
    std::cout << "  3φ² para sa sum=0\n";
    std::cout << "  2φ² para sa sum=φ²\n";
    std::cout << "  φ² para sa sum=2φ²\n\n";
    
    std::cout << "Kaya ang AND ay:\n";
    std::cout << "  AND = φ² kung f(sum-2φ²) = φ²\n";
    std::cout << "  AND = 0 kung f(sum-2φ²) > φ²\n\n";
    
    // ============================================
    // SECOND OSCILLATION PARA SA THRESHOLD
    // ============================================
    std::cout << "SECOND OSCILLATION:\n";
    std::cout << "===================\n\n";
    
    // g(x) = 2φ² - x
    // g(3φ²) = -φ²
    // g(2φ²) = 0
    // g(φ²) = φ²
    
    auto ct_two_phi = make_ct(two_phi_sq);
    
    auto g_3phi = cc->EvalSub(ct_two_phi, osc_neg_2phi);
    auto g_2phi = cc->EvalSub(ct_two_phi, osc_neg_phi);
    auto g_phi = cc->EvalSub(ct_two_phi, osc_zero);
    
    std::cout << "g(x) = 2φ² - f(x):\n";
    std::cout << "  g(3φ²) = " << decrypt_val(g_3phi) << " (expect -φ²)\n";
    std::cout << "  g(2φ²) = " << decrypt_val(g_2phi) << " (expect 0)\n";
    std::cout << "  g(φ²) = " << decrypt_val(g_phi) << " (expect φ²)\n";
    std::cout << "  Level: " << g_3phi->GetLevel() << "\n\n";
    
    // ============================================
    // AND IMPLEMENTATION
    // ============================================
    std::cout << "AND IMPLEMENTATION:\n";
    std::cout << "===================\n\n";
    
    // AND = φ² kung g(f(sum-2φ²)) = φ²
    // AND = 0 kung g(f(sum-2φ²)) <= 0
    
    // Test cases
    auto test_a0 = ct_zero;
    auto test_b0 = ct_zero;
    auto sum_00 = cc->EvalAdd(test_a0, test_b0);
    auto diff_00 = cc->EvalSub(sum_00, ct_two_phi_sq);
    auto f_00 = cc->EvalSub(ct_phi_sq, diff_00);
    auto g_00 = cc->EvalSub(ct_two_phi, f_00);
    
    auto test_a1 = ct_phi_sq;
    auto test_b0_2 = ct_zero;
    auto sum_10 = cc->EvalAdd(test_a1, test_b0_2);
    auto diff_10 = cc->EvalSub(sum_10, ct_two_phi_sq);
    auto f_10 = cc->EvalSub(ct_phi_sq, diff_10);
    auto g_10 = cc->EvalSub(ct_two_phi, f_10);
    
    auto test_a1_2 = ct_phi_sq;
    auto test_b1 = ct_phi_sq;
    auto sum_11 = cc->EvalAdd(test_a1_2, test_b1);
    auto diff_11 = cc->EvalSub(sum_11, ct_two_phi_sq);
    auto f_11 = cc->EvalSub(ct_phi_sq, diff_11);
    auto g_11 = cc->EvalSub(ct_two_phi, f_11);
    
    std::cout << "AND RESULTS:\n";
    std::cout << "  AND(0,0) = " << decrypt_val(g_00) << " (expect 0)\n";
    std::cout << "  AND(φ²,0) = " << decrypt_val(g_10) << " (expect 0)\n";
    std::cout << "  AND(φ²,φ²) = " << decrypt_val(g_11) << " (expect φ²)\n";
    std::cout << "  Level: " << g_00->GetLevel() << "\n\n";
    
    // ============================================
    // NAND IMPLEMENTATION
    // ============================================
    std::cout << "NAND IMPLEMENTATION:\n";
    std::cout << "====================\n\n";
    
    // NAND = NOT(AND) = φ² - AND
    auto nand_00 = cc->EvalSub(ct_phi_sq, g_00);
    auto nand_10 = cc->EvalSub(ct_phi_sq, g_10);
    auto nand_11 = cc->EvalSub(ct_phi_sq, g_11);
    
    std::cout << "NAND RESULTS:\n";
    std::cout << "  NAND(0,0) = " << decrypt_val(nand_00) << " (expect φ²)\n";
    std::cout << "  NAND(φ²,0) = " << decrypt_val(nand_10) << " (expect φ²)\n";
    std::cout << "  NAND(φ²,φ²) = " << decrypt_val(nand_11) << " (expect 0)\n";
    std::cout << "  Level: " << nand_00->GetLevel() << "\n\n";
    
    std::cout << "STATUS:\n";
    std::cout << "=======\n";
    bool success = true;
    if (std::abs(decrypt_val(nand_00) - phi_sq) > 0.001) success = false;
    if (std::abs(decrypt_val(nand_10) - phi_sq) > 0.001) success = false;
    if (std::abs(decrypt_val(nand_11) - 0.0) > 0.001) success = false;
    
    std::cout << (success ? "✅ 0-LEVEL NAND ACHIEVED!" : "❌ Kailangan pa ng adjustment") << "\n";
    
    return 0;
}
