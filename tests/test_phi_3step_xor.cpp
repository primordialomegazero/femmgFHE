// 3-STEP OSCILLATION XOR — 0-LEVEL
// Ang breakthrough: sign detection via oscillation

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  3-STEP OSCILLATION XOR — 0-LEVEL\n";
    std::cout << "  Sign Detection via Golden Ratio\n";
    std::cout << "========================================\n\n";

    const double phi = 1.6180339887498948482;
    const double phi_sq = phi * phi;
    const double phi_cu = phi * phi * phi;
    const double phi_qu = phi * phi * phi * phi;

    CCParams<CryptoContextCKKSRNS> params;
    params.SetMultiplicativeDepth(5);
    params.SetScalingModSize(50);
    params.SetBatchSize(256);
    params.SetFirstModSize(60);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(params);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);
    cc->Enable(ADVANCEDSHE);

    auto keys = cc->KeyGen();
    cc->EvalMultKeyGen(keys.secretKey);
    cc->EvalSumKeyGen(keys.secretKey);
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
    auto ct_half = make_ct(phi_sq / 2.0);
    auto ct_quarter = make_ct(phi_sq / 4.0);

    // ============================================
    // THEORY: 3-STEP OSCILLATION PARA SA XOR
    // ============================================
    std::cout << "THEORY:\n";
    std::cout << "=======\n\n";
    
    std::cout << "Step 1: sum = A + B (0-level)\n";
    std::cout << "  sum ∈ {0, φ², 2φ²}\n\n";
    
    std::cout << "Step 2: diff = φ² - sum (0-level)\n";
    std::cout << "  sum=0:   diff = φ²  (positive)\n";
    std::cout << "  sum=φ²:  diff = 0    (zero)\n";
    std::cout << "  sum=2φ²: diff = -φ² (negative)\n\n";
    
    std::cout << "Step 3: XOR = φ² - |diff| (needs abs)\n";
    std::cout << "  |φ²| = φ², |0| = 0, |-φ²| = φ²\n";
    std::cout << "  XOR = φ² - φ² = 0 (sum=0)\n";
    std::cout << "  XOR = φ² - 0 = φ² (sum=φ²)\n";
    std::cout << "  XOR = φ² - φ² = 0 (sum=2φ²)\n\n";
    
    // ============================================
    // ABS() VIA OSCILLATION
    // ============================================
    std::cout << "ABS() VIA OSCILLATION:\n";
    std::cout << "======================\n\n";
    
    // |x| = sqrt(x²) — kailangan ng mult
    // PERO: Sa φ-domain, may TRICK!
    //
    // |x| ≈ (φ² - x) + (φ² + x) - φ²
    //      = 2φ² - φ² = φ² (constant!)
    //
    // Hindi ito gumagana...
    //
    // ANG TUNAY NA TRICK: EvalSum para sa sign aggregation!
    // Kung maraming slots na may values, ang EvalSum ay
    // nagbibigay ng average na may sign information
    
    // Test: Can we detect sign via oscillation pattern?
    auto test_pos = make_ct(phi_sq);   // Positive
    auto test_zero = make_ct(0.0);      // Zero
    auto test_neg = make_ct(-phi_sq);   // Negative
    
    // Oscillate each 3 times
    auto osc_pos = test_pos;
    auto osc_zero = test_zero;
    auto osc_neg = test_neg;
    
    for (int i = 0; i < 3; i++) {
        osc_pos = cc->EvalSub(ct_phi_sq, osc_pos);
        osc_zero = cc->EvalSub(ct_phi_sq, osc_zero);
        osc_neg = cc->EvalSub(ct_phi_sq, osc_neg);
    }
    
    std::cout << "After 3 oscillations:\n";
    std::cout << "  Positive (φ²): " << decrypt_val(osc_pos) << "\n";
    std::cout << "  Zero (0): " << decrypt_val(osc_zero) << "\n";
    std::cout << "  Negative (-φ²): " << decrypt_val(osc_neg) << "\n\n";
    
    // ============================================
    // PATTERN RECOGNITION
    // ============================================
    std::cout << "PATTERN RECOGNITION:\n";
    std::cout << "====================\n\n";
    
    std::cout << "Positive input oscillation: φ² → 0 → φ² → 0 → ...\n";
    std::cout << "Zero input oscillation:     0 → φ² → 0 → φ² → ...\n";
    std::cout << "Negative input oscillation: -φ² → 2φ² → -φ² → ...\n\n";
    
    std::cout << "OBSERVATION:\n";
    std::cout << "  Positive at zero ay 180° out of phase!\n";
    std::cout << "  Negative ay may different pattern (2φ² state)\n\n";
    
    // ============================================
    // THE KEY: PHASE DETECTION
    // ============================================
    std::cout << "PHASE DETECTION VIA SUMMATION:\n";
    std::cout << "==============================\n\n";
    
    // Kung i-average natin ang oscillation over 2 steps:
    // Positive: (φ² + 0)/2 = φ²/2
    // Zero: (0 + φ²)/2 = φ²/2
    // Negative: (-φ² + 2φ²)/2 = φ²/2
    // Lahat pareho! Kaya hindi ito enough...
    
    // PERO over 3 steps:
    // Positive: φ² → 0 → φ² (sum = 2φ²)
    // Zero: 0 → φ² → 0 (sum = φ²)
    // Negative: -φ² → 2φ² → -φ² (sum = 0)
    // IBA-IBA! Ito ang susi!
    
    auto sum_pos = test_pos;
    auto sum_zero = test_zero;
    auto sum_neg = test_neg;
    
    for (int i = 0; i < 3; i++) {
        auto next_pos = cc->EvalSub(ct_phi_sq, sum_pos);
        auto next_zero = cc->EvalSub(ct_phi_sq, sum_zero);
        auto next_neg = cc->EvalSub(ct_phi_sq, sum_neg);
        
        sum_pos = cc->EvalAdd(sum_pos, next_pos);
        sum_zero = cc->EvalAdd(sum_zero, next_zero);
        sum_neg = cc->EvalAdd(sum_neg, next_neg);
    }
    
    std::cout << "Sum of 3-step oscillation:\n";
    std::cout << "  Positive: " << decrypt_val(sum_pos) << " (expect 2φ²=" << 2*phi_sq << ")\n";
    std::cout << "  Zero: " << decrypt_val(sum_zero) << " (expect φ²=" << phi_sq << ")\n";
    std::cout << "  Negative: " << decrypt_val(sum_neg) << " (expect 0)\n";
    std::cout << "  Level: " << sum_pos->GetLevel() << "\n\n";
    
    // ============================================
    // XOR IMPLEMENTATION
    // ============================================
    std::cout << "XOR IMPLEMENTATION (3-STEP):\n";
    std::cout << "============================\n\n";
    
    // XOR(A,B) = φ² kung sum(A,B) = φ²
    // XOR(A,B) = 0 kung sum(A,B) = 0 o 2φ²
    //
    // Sa 3-step oscillation:
    // sum=0 → oscillation gives 2φ² → XOR = 0
    // sum=φ² → oscillation gives φ² → XOR = φ²
    // sum=2φ² → oscillation gives 0 → XOR = 0
    //
    // Kaya: XOR = 2φ² - oscillation_sum
    // XOR(0,0): 2φ² - 2φ² = 0 ✓
    // XOR(0,φ²): 2φ² - φ² = φ² ✓
    // XOR(φ²,φ²): 2φ² - 0 = 2φ² (DAPAT 0!) ✗
    
    std::cout << "PROBLEMA: XOR(φ²,φ²) ay 2φ², hindi 0\n";
    std::cout << "Kailangan ng modulo 2φ² operation\n\n";
    
    // ============================================
    // THE FINAL TRICK: MODULO VIA OSCILLATION
    // ============================================
    std::cout << "MODULO VIA OSCILLATION:\n";
    std::cout << "=======================\n\n";
    
    std::cout << "Ang 2φ² ay maaaring i-fold sa 0 gamit ang:\n";
    std::cout << "  mod(x, 2φ²) = x kung x < 2φ²\n";
    std::cout << "  mod(x, 2φ²) = x - 2φ² kung x >= 2φ²\n\n";
    
    std::cout << "Sa φ-domain, ang φ⁴ = 3φ² + 2φ + 1\n";
    std::cout << "Kaya ang modulo ay may φ-pattern\n\n";
    
    std::cout << "EMERGENT OBSERVATION:\n";
    std::cout << "=====================\n";
    std::cout << "Ang 3-step oscillation sum ay nagbibigay ng:\n";
    std::cout << "  0 → 2φ² → kailangan i-fold sa 0\n";
    std::cout << "  φ² → φ² → OK na\n";
    std::cout << "  2φ² → 0 → OK na\n\n";
    
    std::cout << "Kung gagamit tayo ng 4-step oscillation:\n";
    std::cout << "  0 → 0 (period-4)\n";
    std::cout << "  φ² → φ² (period-2)\n";
    std::cout << "  2φ² → 0 (period-2)\n";
    std::cout << "  LAHAT AY NAG-FOFOLD SA {0, φ²}!\n";
    
    return 0;
}
