// ============================================
// RULE 110 SA φ-DIFFERENCE SPACE
// Natural collapse via φ-differences
//
// Core insight:
// - Pure states (000, 111) → diff = 0 → ψ
// - Mixed states → diff ≠ 0 → φ
// - Exception: 100 → ψ (left φ, others ψ)
// - Lahat ay addition/subtraction lang
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
    std::cout << "  RULE 110 SA φ-DIFFERENCE SPACE\n";
    std::cout << "  Natural φ-Difference Collapse\n";
    std::cout << "========================================\n\n";

    const double PHI = 1.6180339887498948482;
    const double PSI = -0.6180339887498948482;
    const double MIDPOINT = (PHI + PSI) / 2.0;
    const double RANGE = PHI - PSI;

    CCParams<CryptoContextCKKSRNS> params;
    params.SetMultiplicativeDepth(1);
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

    // ========== φ-DIFFERENCE COLLAPSE ==========
    // diff = |L-C| + |C-R| + |L-R|
    // Pure states: diff = 0 → ψ
    // Mixed states: diff > 0 → φ
    // 
    // Sa FHE, ang absolute value ay approximation via:
    // |x| ≈ sqrt(x²) ≈ x² (para sa small values)
    // Pero para sa φ/ψ, ang square ay sapat na
    
    auto eval_rule110_phi = [&](auto L, auto C, auto R) {
        // Compute differences (addition/subtraction lang)
        auto diff_LC = cc->EvalSub(L, C);
        auto diff_CR = cc->EvalSub(C, R);
        auto diff_LR = cc->EvalSub(L, R);
        
        // Square para sa absolute value approximation (1 mult each)
        auto sq_LC = cc->EvalMult(diff_LC, diff_LC);
        auto sq_CR = cc->EvalMult(diff_CR, diff_CR);
        auto sq_LR = cc->EvalMult(diff_LR, diff_LR);
        
        // Sum of squares (addition lang)
        auto sum_sq = cc->EvalAdd(sq_LC, sq_CR);
        sum_sq = cc->EvalAdd(sum_sq, sq_LR);
        
        // φ-natural threshold: kung sum_sq > 0, φ; kung = 0, ψ
        // Ang sum_sq ay natural na nagco-collapse:
        // - Pure states: 0 → ψ
        // - Mixed states: positive → φ
        
        // I-adjust para sa 100 special case
        // Ang 100 ay may L=φ, C=ψ, R=ψ
        // diff_LC = φ-ψ = RANGE (malaki)
        // diff_CR = ψ-ψ = 0
        // diff_LR = φ-ψ = RANGE (malaki)
        // sum_sq = 2×RANGE² (dapat ψ)
        
        // φ-asymmetry: ang left φ ay may ibang treatment
        // Kung L=φ at C=ψ at R=ψ, dapat ψ
        // Ito ay nangyayari kung ang left position ay may φ-bias
        
        // Position-weighted difference
        auto wL = cc->EvalSub(L, make_ct(PSI));
        auto wR = cc->EvalSub(R, make_ct(PSI));
        
        // Kung ang left ay φ at ang right ay ψ, mag-flip
        auto flip_condition = cc->EvalSub(wL, wR);
        
        // Combine: sum_sq - φ×flip_condition
        auto result = cc->EvalSub(sum_sq, flip_condition);
        
        return result;
    };

    auto ct_phi = make_ct(PHI);
    auto ct_psi = make_ct(PSI);

    std::cout << "RULE 110 SA φ-DIFFERENCE SPACE:\n";
    std::cout << "===============================\n\n";
    std::cout << "  ψ = " << PSI << " (False)\n";
    std::cout << "  φ = " << PHI << " (True)\n";
    std::cout << "  Pure states → ψ, Mixed → φ\n\n";

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

    std::cout << "\n  Rule 110 sa φ-difference space: " << correct << "/8\n";
    
    // Show difference breakdown
    std::cout << "\nDIFFERENCE BREAKDOWN:\n";
    std::cout << "=====================\n\n";
    
    for (auto& t : tests) {
        auto diff_LC = cc->EvalSub(t.ct_l, t.ct_c);
        auto diff_CR = cc->EvalSub(t.ct_c, t.ct_r);
        auto diff_LR = cc->EvalSub(t.ct_l, t.ct_r);
        
        auto sq_LC = cc->EvalMult(diff_LC, diff_LC);
        auto sq_CR = cc->EvalMult(diff_CR, diff_CR);
        auto sq_LR = cc->EvalMult(diff_LR, diff_LR);
        
        auto sum_sq = cc->EvalAdd(sq_LC, sq_CR);
        sum_sq = cc->EvalAdd(sum_sq, sq_LR);
        
        std::cout << "  Pattern " << t.pattern << ":\n";
        std::cout << "    diff_LC=" << decrypt_val(diff_LC)
                  << " diff_CR=" << decrypt_val(diff_CR)
                  << " diff_LR=" << decrypt_val(diff_LR) << "\n";
        std::cout << "    sum_sq=" << decrypt_val(sum_sq) << "\n\n";
    }
    
    std::cout << "  Level: " << eval_rule110_phi(ct_phi, ct_phi, ct_phi)->GetLevel() << "\n";
    std::cout << "  φ-difference transformation complete\n";

    return 0;
}
