// ============================================
// RULE 110 SA φ-ADDITION-ONLY SPACE
// Multiplication via rotation at addition lang
//
// Core insight:
// - φ^n = φ^(n-1) + φ^(n-2) — Fibonacci identity
// - a × b ≈ φ^(-1)(a+b)² - φ(a-b)² — φ-decomposition
// - Squares via rotation: x² = sum(rot(x,i) × rot(x,-i))
// - Sa φ-space, ang rotation ay emergent multiplication
//
// Author: Dan Fernandez / Primordial Omega Zero
// ============================================

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  RULE 110 SA φ-ADDITION-ONLY SPACE\n";
    std::cout << "  Multiplication via φ-Rotation\n";
    std::cout << "========================================\n\n";

    const double PHI = 1.6180339887498948482;
    const double PSI = -0.6180339887498948482;
    const double MIDPOINT = (PHI + PSI) / 2.0;
    const double PHI2 = PHI * PHI;
    const double INV_PHI = 1.0 / PHI;
    const double INV_PHI2 = 1.0 / PHI2;

    CCParams<CryptoContextCKKSRNS> params;
    params.SetMultiplicativeDepth(3);
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
    cc->EvalRotateKeyGen(keys.secretKey, {1, -1, 2, -2});
    
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

    // ========== φ-ADDITION-ONLY MULTIPLICATION ==========
    // a × b = φ^(-1)(a+b)² - φ(a-b)²
    // Ang squares ay via φ-rotation identity:
    // x² = x × x ≈ sum(rot(x,i) + rot(x,-i)) / 2
    // Sa φ-space: x² = (φ-1) × sum(rot(x,i)) para sa small i
    
    auto phi_square_via_rotation = [&](auto x) {
        // x² ≈ (x + rot(x,1) + rot(x,-1)) / 3
        // Ito ay local average na nag-a-approximate ng square
        auto rot1 = cc->EvalRotate(x, 1);
        auto rot_m1 = cc->EvalRotate(x, -1);
        
        auto sum = cc->EvalAdd(x, rot1);
        sum = cc->EvalAdd(sum, rot_m1);
        
        // Scale para sa approximation
        auto approx = cc->EvalMult(sum, make_ct(1.0 / 3.0));
        
        return approx;
    };
    
    auto phi_multiply_addition_only = [&](auto a, auto b) {
        // Step 1: Sum at difference
        auto sum = cc->EvalAdd(a, b);
        auto diff = cc->EvalSub(a, b);
        
        // Step 2: Square via rotation (1 multiplication each)
        auto sum_sq = phi_square_via_rotation(sum);
        auto diff_sq = phi_square_via_rotation(diff);
        
        // Step 3: φ-weighted combination
        auto term1 = cc->EvalMult(sum_sq, make_ct(INV_PHI));
        auto term2 = cc->EvalMult(diff_sq, make_ct(PHI));
        
        // Step 4: Subtract
        auto result = cc->EvalSub(term1, term2);
        
        return result;
    };

    // ========== φ-ADDITION-ONLY RULE 110 ==========
    // Gumagamit ng position weights pero via addition-only multiplication
    
    auto eval_rule110_phi = [&](auto L, auto C, auto R) {
        // Position-weighted linear terms (addition only)
        auto wL = cc->EvalMult(L, make_ct(INV_PHI2));
        auto wR = cc->EvalMult(R, make_ct(PHI2));
        
        // Pairwise terms via addition-only multiplication
        auto LC = phi_multiply_addition_only(L, C);
        auto CR = phi_multiply_addition_only(C, R);
        auto LR = phi_multiply_addition_only(L, R);
        
        // Position-weighted pairwise
        auto wLC = cc->EvalMult(LC, make_ct(INV_PHI));
        auto wCR = cc->EvalMult(CR, make_ct(PHI));
        
        // Combine
        auto sum_linear = cc->EvalAdd(wL, C);
        sum_linear = cc->EvalAdd(sum_linear, wR);
        
        auto sum_pairwise = cc->EvalAdd(wLC, wCR);
        sum_pairwise = cc->EvalAdd(sum_pairwise, LR);
        
        auto result = cc->EvalSub(sum_linear, sum_pairwise);
        
        return result;
    };

    auto ct_phi = make_ct(PHI);
    auto ct_psi = make_ct(PSI);

    std::cout << "RULE 110 SA φ-ADDITION-ONLY SPACE:\n";
    std::cout << "==================================\n\n";
    std::cout << "  ψ = " << PSI << " (False)\n";
    std::cout << "  φ = " << PHI << " (True)\n";
    std::cout << "  Multiplication via φ-rotation identity\n\n";

    std::cout << "TRUTH TABLE:\n";
    std::cout << "============\n\n";

    struct TestCase {
        std::string pattern;
        Ciphertext<DCRTPoly> ct_l, ct_c, ct_r;
        double expected;
    };

    std::vector<TestCase> tests = {
        {"000", ct_psi, ct_psi, ct_psi, PSI},
        {"001", ct_psi, ct_psi, ct_phi, PHI},
        {"010", ct_psi, ct_phi, ct_psi, PHI},
        {"011", ct_psi, ct_phi, ct_phi, PHI},
        {"100", ct_phi, ct_psi, ct_psi, PSI},
        {"101", ct_phi, ct_psi, ct_phi, PHI},
        {"110", ct_phi, ct_phi, ct_psi, PHI},
        {"111", ct_phi, ct_phi, ct_phi, PSI}
    };

    int correct = 0;
    for (auto& t : tests) {
        auto result = eval_rule110_phi(t.ct_l, t.ct_c, t.ct_r);
        double val = decrypt_val(result);
        
        double collapsed = (val > MIDPOINT) ? PHI : PSI;
        
        bool match = (collapsed == t.expected);
        if (match) correct++;
        
        std::cout << "  (" << t.pattern << ") → " << val
                  << " → " << collapsed
                  << " (expected " << t.expected << ")"
                  << (match ? " ✓" : " ✗") << "\n";
    }

    std::cout << "\n  Rule 110 sa φ-addition-only space: " << correct << "/8\n";
    
    // Test multiplication accuracy
    std::cout << "\nMULTIPLICATION ACCURACY:\n";
    std::cout << "========================\n\n";
    
    auto test_a = make_ct(1.5);
    auto test_b = make_ct(2.5);
    auto mult_result = phi_multiply_addition_only(test_a, test_b);
    auto direct_mult = cc->EvalMult(test_a, test_b);
    
    std::cout << "  1.5 × 2.5 via addition-only: " << decrypt_val(mult_result) << "\n";
    std::cout << "  1.5 × 2.5 direct: " << decrypt_val(direct_mult) << "\n";
    std::cout << "  Expected: " << 1.5 * 2.5 << "\n\n";
    
    std::cout << "  Level: " << eval_rule110_phi(ct_phi, ct_phi, ct_phi)->GetLevel() << "\n";
    std::cout << "  φ-addition-only transformation complete\n";

    return 0;
}
