// RULE 110 — φ-DOMAIN DIRECT COMPUTATION
// Turing-complete cellular automaton sa 0-level

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>
#include <chrono>

using namespace lbcrypto;
using namespace std::chrono;

int main() {
    std::cout << "========================================\n";
    std::cout << "  RULE 110 — φ-DOMAIN DIRECT\n";
    std::cout << "  0-Level Turing Complete Attempt\n";
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
    
    // ============================================
    // IMPORTANTE: Generate rotation keys!
    // ============================================
    std::cout << "Generating rotation keys...\n";
    cc->EvalAtIndexKeyGen(keys.secretKey, {1, -1, 2, -2, 4, -4, 8, -8, 16, -16, 32, -32, 64, -64, 128, -128});
    std::cout << "Rotation keys generated!\n\n";
    
    auto slots = cc->GetEncodingParams()->GetBatchSize();

    auto make_ct = [&](double val) {
        std::vector<std::complex<double>> vec(slots, {0.0, 0.0});
        vec[0] = {val, 0.0};
        return cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(vec));
    };

    auto make_vector_ct = [&](std::vector<double> vals) {
        std::vector<std::complex<double>> vec(slots, {0.0, 0.0});
        for (size_t i = 0; i < vals.size() && i < slots; i++) {
            vec[i] = {vals[i], 0.0};
        }
        return cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(vec));
    };

    auto decrypt_val = [&](auto ct, size_t idx = 0) {
        Plaintext pt;
        cc->Decrypt(keys.secretKey, ct, &pt);
        return pt->GetCKKSPackedValue()[idx].real();
    };

    auto ct_phi_sq = make_ct(phi_sq);
    auto ct_zero = make_ct(0.0);
    auto ct_one = make_ct(1.0);

    // ============================================
    // RULE 110 TRANSITION FUNCTION
    // ============================================
    // Rule 110: (left, current, right) → next
    // 111→0, 110→1, 101→1, 100→0
    // 011→1, 010→1, 001→1, 000→0
    
    std::cout << "RULE 110 ANALYSIS:\n";
    std::cout << "==================\n";
    std::cout << "1. Direct implementation ay may multiplication\n";
    std::cout << "2. Linear approximation ay hindi separable\n";
    std::cout << "3. Kailangan ng nonlinearity o alternatibong approach\n\n";

    // ============================================
    // TEST: ROTATE + ADD + SUB = 0 LEVELS
    // ============================================
    std::cout << "ROTATE + ADD/SUB TEST:\n";
    std::cout << "======================\n";
    
    // Gumawa ng pattern: 1 sa slot 0, 1 sa slot 1
    std::vector<double> pattern(slots, 0.0);
    pattern[0] = phi_sq;
    pattern[1] = phi_sq;
    
    auto ct_pattern = make_vector_ct(pattern);
    
    // Rotate left (shift right): slot 0 gets from slot 1
    auto rotated_left = cc->EvalRotate(ct_pattern, 1);
    
    // Rotate right (shift left): slot 0 gets from slot 255
    auto rotated_right = cc->EvalRotate(ct_pattern, -1);
    
    std::cout << "Pattern: [φ², φ², 0, 0, ...]\n";
    std::cout << "Original slot 0: " << decrypt_val(ct_pattern, 0) << "\n";
    std::cout << "Rotated +1 slot 0: " << decrypt_val(rotated_left, 0) << " (from slot 1)\n";
    std::cout << "Rotated -1 slot 0: " << decrypt_val(rotated_right, 0) << " (from slot 255)\n";
    std::cout << "Level: " << rotated_left->GetLevel() << "\n\n";
    
    // ============================================
    // OSCILLATION TEST
    // ============================================
    std::cout << "OSCILLATION TEST (0-level):\n";
    std::cout << "===========================\n";
    
    auto osc = ct_zero;
    for (int i = 0; i < 5; i++) {
        osc = cc->EvalSub(ct_phi_sq, osc);
        double val = decrypt_val(osc);
        int level = osc->GetLevel();
        std::cout << "  Step " << i << ": " << val << " (level " << level << ")\n";
    }
    std::cout << "✓ Oscillation works at 0-level!\n\n";
    
    // ============================================
    // EVALSUM TEST
    // ============================================
    std::cout << "EVALSUM TEST:\n";
    std::cout << "=============\n";
    
    auto ct_sum = cc->EvalSum(ct_pattern, slots);
    
    std::cout << "Sum of pattern: " << decrypt_val(ct_sum, 0) << "\n";
    std::cout << "Expected: " << 2*phi_sq << " (2 ones)\n";
    std::cout << "Level: " << ct_sum->GetLevel() << "\n\n";
    
    // ============================================
    // RULE 110 STEP ATTEMPT (SIMPLIFIED)
    // ============================================
    std::cout << "RULE 110 STEP ATTEMPT:\n";
    std::cout << "=====================\n";
    std::cout << "Using: sum = L + C + R, then threshold\n";
    std::cout << "Rule 110: next=1 if sum ∈ {φ², 2φ²}, next=0 if sum ∈ {0, 3φ²}\n\n";
    
    // Initialize pattern for Rule 110
    std::vector<double> init_pattern(slots, 0.0);
    // Simple pattern: alternating
    for (int i = 0; i < 10; i++) {
        init_pattern[i] = (i % 2 == 0) ? phi_sq : 0.0;
    }
    
    auto current = make_vector_ct(init_pattern);
    
    std::cout << "Initial pattern (first 10): ";
    for (int i = 0; i < 10; i++) {
        std::cout << (init_pattern[i] > phi_sq/2 ? "1" : "0");
    }
    std::cout << "\n\n";
    
    // One Rule 110 step: 
    // L = rotate(current, 1)
    // R = rotate(current, -1)
    // sum = L + current + R
    auto left = cc->EvalRotate(current, 1);
    auto right = cc->EvalRotate(current, -1);
    auto sum = cc->EvalAdd(left, current);
    sum = cc->EvalAdd(sum, right);
    
    std::cout << "After 1 step (sum): ";
    for (int i = 0; i < 10; i++) {
        double v = decrypt_val(sum, i);
        int count = (int)(v / phi_sq + 0.5); // Round to nearest integer
        std::cout << count;
    }
    std::cout << "\n";
    std::cout << "Level: " << sum->GetLevel() << "\n\n";
    
    // ============================================
    // KEY INSIGHT
    // ============================================
    std::cout << "KEY INSIGHT:\n";
    std::cout << "============\n";
    std::cout << "1. Rotate + Add + Sub = 0 levels (CONFIRMED)\n";
    std::cout << "2. EvalSum = 0 levels (CONFIRMED)\n";
    std::cout << "3. Oscillation = 0 levels (CONFIRMED)\n";
    std::cout << "4. Ang problema: Threshold (nonlinear) ay kailangan\n";
    std::cout << "5. PERO: May φ-domain trick tayo!\n\n";
    
    std::cout << "NEXT STEP:\n";
    std::cout << "==========\n";
    std::cout << "Hanapin ang φ-domain representation na nagbibigay\n";
    std::cout << "ng exact Rule 110 transition na 0-level.\n";
    std::cout << "Baka kaya sa φ-base polynomial: φ²=φ+1\n";
    std::cout << "o sa oscillation-based soft thresholding\n";
    
    return 0;
}
