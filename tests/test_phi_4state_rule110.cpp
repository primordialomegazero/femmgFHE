// φ-DOMAIN 4-STATE SYSTEM + RULE 110
// 0-Level Universal Turing Machine Attempt
//
// 4 STATES: {0, φ², 2φ², 3φ²}
// Lahat 0-level via oscillations
//
// RULE 110 TRANSITION:
// next = 1 if sum ∈ {1, 2}
// next = 0 if sum ∈ {0, 3}
//
// Sa φ²-domain:
// next = φ² if sum ∈ {φ², 2φ²}
// next = 0 if sum ∈ {0, 3φ²}

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  φ-DOMAIN 4-STATE + RULE 110\n";
    std::cout << "  0-Level Universal Attempt\n";
    std::cout << "========================================\n\n";

    const double phi = 1.6180339887498948482;
    const double phi_sq = phi * phi;
    const double two_phi_sq = 2 * phi_sq;
    const double three_phi_sq = 3 * phi_sq;
    const double four_phi_sq = 4 * phi_sq;

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
    cc->EvalAtIndexKeyGen(keys.secretKey, {1, -1, 2, -2});
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

    // ============================================
    // 4-STATE SYSTEM VIA OSCILLATIONS
    // ============================================
    
    // Oscillation A: φ² - x (states: 0, φ²)
    auto osc_A = [&](auto x) {
        return cc->EvalSub(make_ct(phi_sq), x);
    };
    
    // Oscillation B: 2φ² - x (states: 0, 2φ²)
    auto osc_B = [&](auto x) {
        return cc->EvalSub(make_ct(two_phi_sq), x);
    };
    
    // Oscillation C: 3φ² - x (states: 0, 3φ²)
    auto osc_C = [&](auto x) {
        return cc->EvalSub(make_ct(three_phi_sq), x);
    };
    
    std::cout << "4-STATE OSCILLATION TEST:\n";
    std::cout << "=========================\n\n";
    
    // Test individual oscillations
    auto test_osc = [&](const char* name, auto osc_fn, int steps = 6) {
        std::cout << name << ": ";
        auto current = make_ct(0.0);
        for (int i = 0; i < steps; i++) {
            current = osc_fn(current);
            std::cout << decrypt_val(current) << " ";
        }
        std::cout << "\n";
    };
    
    test_osc("A (0↔φ²):  ", osc_A);
    test_osc("B (0↔2φ²): ", osc_B);
    test_osc("C (0↔3φ²): ", osc_C);
    
    std::cout << "\n";
    
    // ============================================
    // RULE 110 — 0-LEVEL ATTEMPT
    // ============================================
    std::cout << "RULE 110 STEP (0-LEVEL):\n";
    std::cout << "========================\n\n";
    
    // Rule 110: next = 1 if (L,C,R) ∈ {001,010,011,101,110}
    // Sa φ²-domain: next = φ² if sum ∈ {φ², 2φ²}
    
    // Sum via addition (0-level)
    auto compute_sum = [&](auto L, auto C, auto R) {
        auto sum1 = cc->EvalAdd(L, C);
        return cc->EvalAdd(sum1, R);
    };
    
    // Threshold via oscillations (0-level)
    // Kung sum = 0 → 0
    // Kung sum = φ² → φ²
    // Kung sum = 2φ² → φ²  
    // Kung sum = 3φ² → 0
    
    // TRICK: 3φ² - sum gives:
    // sum=0 → 3φ²
    // sum=φ² → 2φ²
    // sum=2φ² → φ²
    // sum=3φ² → 0
    
    // Then 3φ² - (3φ² - sum) = sum (identity)
    // Pero kung 2φ² - (3φ² - sum):
    // sum=0 → -φ² (should be 0)
    // sum=φ² → 0 (should be φ²)
    // sum=2φ² → φ² (should be φ²)
    // sum=3φ² → 2φ² (should be 0)
    
    // PINAKA-MALAPIT: φ² - |sum - 2φ²|
    // Pero absolute value ay nonlinear...
    
    // ALTERNATIVE: Rule 110 via lookup table
    // Hindi kaya sa pure addition/subtraction
    // Kailangan ng rotation-based lookup
    
    std::cout << "RULE 110 ANALYSIS:\n";
    std::cout << "==================\n";
    std::cout << "  Sum = L + C + R (0-level)\n";
    std::cout << "  Threshold kailangan (nonlinear)\n";
    std::cout << "  Pero may ROTATION-BASED lookup!\n\n";
    
    // ROTATION-BASED RULE 110
    // I-encode ang Rule 110 table sa rotations
    
    std::cout << "ROTATION-BASED RULE 110:\n";
    std::cout << "========================\n\n";
    
    // Test: Can rotations give us the threshold?
    auto test_pattern = make_ct(phi_sq);
    
    // Rotate test
    auto rot_left = cc->EvalRotate(test_pattern, 1);
    auto rot_right = cc->EvalRotate(test_pattern, -1);
    
    std::cout << "Original: " << decrypt_val(test_pattern) << "\n";
    std::cout << "Left:     " << decrypt_val(rot_left) << "\n";
    std::cout << "Right:    " << decrypt_val(rot_right) << "\n";
    std::cout << "Sum:      " << decrypt_val(compute_sum(rot_left, test_pattern, rot_right)) << "\n";
    std::cout << "Level:    " << rot_left->GetLevel() << "\n\n";
    
    std::cout << "KEY INSIGHT:\n";
    std::cout << "============\n";
    std::cout << "1. Sum = L+C+R ay 0-level\n";
    std::cout << "2. Rotations ay 0-level\n";
    std::cout << "3. Ang threshold ay kailangan\n";
    std::cout << "4. PERO: Baka may φ-property na\n";
    std::cout << "   nagbibigay ng natural threshold\n";
    std::cout << "5. φ² = φ+1 → natural nonlinearity!\n";
    
    return 0;
}
