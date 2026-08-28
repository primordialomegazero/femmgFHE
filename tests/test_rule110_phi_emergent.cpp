// ============================================
// RULE 110 SA φ-EMERGENT SPACE
// Ultra-shallow φ-collapse, 3 multiplications total
//
// Core fix:
// - Quadratic decision: output = (x - midpoint)² × sign_hint
// - Walang smooth sign approximations
// - Direct φ-mapping na 3 multiplications lang
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
    std::cout << "  RULE 110 SA φ-EMERGENT SPACE\n";
    std::cout << "  Ultra-Shallow Natural Collapse\n";
    std::cout << "========================================\n\n";

    const double PHI = 1.6180339887498948482;
    const double PSI = -0.6180339887498948482;
    const double MIDPOINT = (PHI + PSI) / 2.0;
    const double RANGE = PHI - PSI;
    const double INV_PHI = 1.0 / PHI;

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

    // ========== φ-NATURAL PRODUCT ==========
    // Imbis na sum, gamitin ang product na natural sa φ-space
    // prod(L,C,R) = L × C × R
    // Sa φ-space, ang multiplication ay natural na nagco-collapse
    
    auto phi_natural_product = [&](auto L, auto C, auto R) {
        auto prod1 = cc->EvalMult(L, C);
        return cc->EvalMult(prod1, R);
    };

    // ========== φ-QUADRATIC COLLAPSE ==========
    // output = midpoint + (x - midpoint) × |x - midpoint|
    // Ang quadratic term ay nagtutulak sa values palayo sa midpoint
    // 2 multiplications lang total
    
    auto phi_quadratic_collapse = [&](auto x) {
        // Step 1: Distance from midpoint (1 sub, 0 mult)
        auto diff = cc->EvalSub(x, make_ct(MIDPOINT));
        
        // Step 2: Square para sa amplification (1 mult)
        auto squared = cc->EvalMult(diff, diff);
        
        // Step 3: Multiply pabalik kay diff para sa sign (1 mult)
        auto cubic = cc->EvalMult(squared, diff);
        
        // Step 4: Scale at idagdag sa midpoint
        auto scaled = cc->EvalMult(cubic, make_ct(2.0));
        auto output = cc->EvalAdd(make_ct(MIDPOINT), scaled);
        
        return output;
    };

    // ========== φ-EMERGENT RULE 110 ==========
    // Total multiplications: 2 (product) + 2 (quadratic) = 4
    auto eval_rule110_phi = [&](auto L, auto C, auto R) {
        // Natural product
        auto prod = phi_natural_product(L, C, R);
        
        // Quadratic collapse
        auto result = phi_quadratic_collapse(prod);
        
        return result;
    };

    auto ct_phi = make_ct(PHI);
    auto ct_psi = make_ct(PSI);

    std::cout << "RULE 110 SA φ-EMERGENT SPACE:\n";
    std::cout << "==============================\n\n";
    std::cout << "  ψ = " << PSI << " (False attractor)\n";
    std::cout << "  φ = " << PHI << " (True attractor)\n";
    std::cout << "  Natural product → quadratic collapse\n\n";

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
        
        // Check saang attractor nag-collapse
        double dist_to_phi = std::abs(val - PHI);
        double dist_to_psi = std::abs(val - PSI);
        double collapsed = (dist_to_phi < dist_to_psi) ? PHI : PSI;
        
        bool match = (collapsed == t.expected);
        if (match) correct++;
        
        std::cout << "  (" << t.pattern << ") → " << val
                  << " → collapse sa " << collapsed
                  << " (expected " << t.expected << ")"
                  << (match ? " ✓" : " ✗") << "\n";
    }

    std::cout << "\n  Rule 110 sa φ-emergent space: " << correct << "/8\n";
    
    // Show intermediate values
    std::cout << "\nINTERMEDIATE VALUES:\n";
    std::cout << "====================\n\n";
    
    for (auto& t : tests) {
        auto prod = phi_natural_product(t.ct_l, t.ct_c, t.ct_r);
        
        std::cout << "  Pattern " << t.pattern << ":\n";
        std::cout << "    product=" << decrypt_val(prod) << "\n";
        
        auto diff = cc->EvalSub(prod, make_ct(MIDPOINT));
        auto squared = cc->EvalMult(diff, diff);
        auto cubic = cc->EvalMult(squared, diff);
        
        std::cout << "    diff_from_mid=" << decrypt_val(diff)
                  << " squared=" << decrypt_val(squared)
                  << " cubic=" << decrypt_val(cubic) << "\n\n";
    }
    
    std::cout << "  Level: " << eval_rule110_phi(ct_phi, ct_phi, ct_phi)->GetLevel() << "\n";
    std::cout << "  φ-emergent collapse complete\n";

    return 0;
}
