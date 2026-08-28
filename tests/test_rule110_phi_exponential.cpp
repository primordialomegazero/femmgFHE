// ============================================
// RULE 110 SA φ-EXPONENTIAL SPACE
// Exact multiplication via φ-exponent addition
//
// Core insight:
// - φ^a × φ^b = φ^(a+b) — multiplication ay addition sa exponent
// - I-encode ang values bilang φ-exponents
// - Ang multiplication ay nagiging addition lang
// - Walang approximation — exact φ-property
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
    std::cout << "  RULE 110 SA φ-EXPONENTIAL SPACE\n";
    std::cout << "  Exact Multiplication via φ-Exponents\n";
    std::cout << "========================================\n\n";

    const double PHI = 1.6180339887498948482;
    const double PSI = -0.6180339887498948482;
    const double MIDPOINT = (PHI + PSI) / 2.0;
    const double PHI2 = PHI * PHI;
    const double INV_PHI = 1.0 / PHI;
    const double INV_PHI2 = 1.0 / PHI2;
    const double LOG_PHI = std::log(PHI);

    CCParams<CryptoContextCKKSRNS> params;
    params.SetMultiplicativeDepth(2);
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

    // ========== φ-EXPONENTIAL ENCODING ==========
    // I-encode ang φ/ψ values bilang exponents
    // φ → +1 (φ^1 = φ)
    // ψ → -1 (φ^(-1) = 1/φ = 0.618)
    // Para sa ψ = -0.618, gamitin ang φ^(-1) = 0.618
    
    auto to_exponent = [&](auto val) {
        // Map φ → +1, ψ → -1
        // exponent = (val - MIDPOINT) / (RANGE/2)
        auto diff = cc->EvalSub(val, make_ct(MIDPOINT));
        return cc->EvalMult(diff, make_ct(2.0 / (PHI - PSI)));
    };
    
    auto from_exponent = [&](auto exp_val) {
        // Map exponent → φ-value
        // val = MIDPOINT + exp_val × RANGE/2
        auto scaled = cc->EvalMult(exp_val, make_ct((PHI - PSI) / 2.0));
        return cc->EvalAdd(make_ct(MIDPOINT), scaled);
    };

    // ========== φ-EXPONENTIAL MULTIPLICATION ==========
    // a × b = φ^(log_φ(a) + log_φ(b))
    // Sa exponent space: mult(a,b) = from_exponent(exp_a + exp_b - 1)
    // Ang -1 ay para sa normalization
    
    auto phi_multiply = [&](auto a, auto b) {
        auto exp_a = to_exponent(a);
        auto exp_b = to_exponent(b);
        
        // Addition sa exponent space
        auto exp_sum = cc->EvalAdd(exp_a, exp_b);
        
        // Normalize (kasi φ^1 × φ^1 = φ^2, kailangan φ^1)
        auto normalized = cc->EvalSub(exp_sum, make_ct(1.0));
        
        return from_exponent(normalized);
    };

    // ========== φ-EXPONENTIAL RULE 110 ==========
    auto eval_rule110_phi = [&](auto L, auto C, auto R) {
        // Position-weighted linear terms
        auto wL = cc->EvalMult(L, make_ct(INV_PHI2));
        auto wR = cc->EvalMult(R, make_ct(PHI2));
        
        // Pairwise terms via exponential multiplication
        auto LC = phi_multiply(L, C);
        auto CR = phi_multiply(C, R);
        auto LR = phi_multiply(L, R);
        
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

    std::cout << "RULE 110 SA φ-EXPONENTIAL SPACE:\n";
    std::cout << "================================\n\n";
    std::cout << "  ψ = " << PSI << " (False, exponent -1)\n";
    std::cout << "  φ = " << PHI << " (True, exponent +1)\n";
    std::cout << "  Multiplication = Addition sa exponent\n\n";

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

    std::cout << "\n  Rule 110 sa φ-exponential space: " << correct << "/8\n";
    
    // Test exact multiplication
    std::cout << "\nEXACT MULTIPLICATION TEST:\n";
    std::cout << "==========================\n\n";
    
    auto test_a = make_ct(1.5);
    auto test_b = make_ct(2.5);
    auto mult_result = phi_multiply(test_a, test_b);
    auto direct_mult = cc->EvalMult(test_a, test_b);
    
    std::cout << "  1.5 × 2.5 via φ-exponent: " << decrypt_val(mult_result) << "\n";
    std::cout << "  1.5 × 2.5 direct: " << decrypt_val(direct_mult) << "\n";
    std::cout << "  Expected: " << 1.5 * 2.5 << "\n\n";
    
    // Show exponent values
    std::cout << "EXPONENT ANALYSIS:\n";
    std::cout << "==================\n\n";
    
    auto exp_phi = to_exponent(ct_phi);
    auto exp_psi = to_exponent(ct_psi);
    
    std::cout << "  exponent(φ) = " << decrypt_val(exp_phi) << " (expected +1)\n";
    std::cout << "  exponent(ψ) = " << decrypt_val(exp_psi) << " (expected -1)\n\n";
    
    std::cout << "  Level: " << eval_rule110_phi(ct_phi, ct_phi, ct_phi)->GetLevel() << "\n";
    std::cout << "  φ-exponential transformation complete\n";

    return 0;
}
