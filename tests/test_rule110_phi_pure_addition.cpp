// ============================================
// RULE 110 SA φ-PURE-ADDITION SPACE
// Zero multiplication — pure addition at subtraction
//
// Core insight:
// - Ang φ/ψ values ay naka-pre-encode na bilang exponents
// - φ → +1, ψ → -1 (hindi na kailangan ng transformation)
// - Multiplication = addition sa exponent space
// - Lahat ng operations ay addition/subtraction lang
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
    std::cout << "  RULE 110 SA φ-PURE-ADDITION SPACE\n";
    std::cout << "  Zero Multiplication\n";
    std::cout << "========================================\n\n";

    const double PHI_EXP = 1.0;   // φ bilang exponent
    const double PSI_EXP = -1.0;  // ψ bilang exponent
    const double MIDPOINT = 0.0;  // midpoint sa exponent space
    const double PHI2 = 2.0;      // φ^2 exponent
    const double INV_PHI2 = -2.0; // φ^(-2) exponent
    
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

    // ========== φ-PURE-ADDITION RULE 110 ==========
    // Lahat ng values ay naka-exponent na
    // φ → +1, ψ → -1
    // Ang "multiplication" ay addition sa exponent
    // Ang position weights ay addition ng constants
    
    auto eval_rule110_phi = [&](auto L, auto C, auto R) {
        // Position-weighted linear terms (addition lang)
        // wL = L + (-2) = L - 2
        auto wL = cc->EvalSub(L, make_ct(2.0));
        // wR = R + 2
        auto wR = cc->EvalAdd(R, make_ct(2.0));
        
        // Pairwise "multiplication" = addition sa exponent
        // LC = L + C - 1 (kasi φ^1 × φ^1 = φ^2, normalize sa φ^1)
        auto LC = cc->EvalSub(cc->EvalAdd(L, C), make_ct(1.0));
        auto CR = cc->EvalSub(cc->EvalAdd(C, R), make_ct(1.0));
        auto LR = cc->EvalSub(cc->EvalAdd(L, R), make_ct(1.0));
        
        // Position-weighted pairwise
        auto wLC = cc->EvalSub(LC, make_ct(1.0));  // φ^(-1)
        auto wCR = cc->EvalAdd(CR, make_ct(1.0));  // φ^(+1)
        
        // Combine: linear - pairwise
        auto sum_linear = cc->EvalAdd(wL, C);
        sum_linear = cc->EvalAdd(sum_linear, wR);
        
        auto sum_pairwise = cc->EvalAdd(wLC, wCR);
        sum_pairwise = cc->EvalAdd(sum_pairwise, LR);
        
        auto result = cc->EvalSub(sum_linear, sum_pairwise);
        
        return result;
    };

    auto ct_phi = make_ct(PHI_EXP);
    auto ct_psi = make_ct(PSI_EXP);

    std::cout << "RULE 110 SA φ-PURE-ADDITION SPACE:\n";
    std::cout << "==================================\n\n";
    std::cout << "  ψ = " << PSI_EXP << " (False, exponent -1)\n";
    std::cout << "  φ = " << PHI_EXP << " (True, exponent +1)\n";
    std::cout << "  Lahat ng operations ay addition/subtraction\n\n";

    std::cout << "TRUTH TABLE:\n";
    std::cout << "============\n\n";

    struct TestCase {
        std::string pattern;
        Ciphertext<DCRTPoly> ct_l, ct_c, ct_r;
        double expected;
    };

    std::vector<TestCase> tests = {
        {"000", ct_psi, ct_psi, ct_psi, PSI_EXP},
        {"001", ct_psi, ct_psi, ct_phi, PHI_EXP},
        {"010", ct_psi, ct_phi, ct_psi, PHI_EXP},
        {"011", ct_psi, ct_phi, ct_phi, PHI_EXP},
        {"100", ct_phi, ct_psi, ct_psi, PSI_EXP},
        {"101", ct_phi, ct_psi, ct_phi, PHI_EXP},
        {"110", ct_phi, ct_phi, ct_psi, PHI_EXP},
        {"111", ct_phi, ct_phi, ct_phi, PSI_EXP}
    };

    int correct = 0;
    for (auto& t : tests) {
        auto result = eval_rule110_phi(t.ct_l, t.ct_c, t.ct_r);
        double val = decrypt_val(result);
        
        double collapsed = (val > MIDPOINT) ? PHI_EXP : PSI_EXP;
        
        bool match = (collapsed == t.expected);
        if (match) correct++;
        
        std::cout << "  (" << t.pattern << ") → " << val
                  << " → " << collapsed
                  << " (expected " << t.expected << ")"
                  << (match ? " ✓" : " ✗") << "\n";
    }

    std::cout << "\n  Rule 110 sa φ-pure-addition space: " << correct << "/8\n";
    
    // Show pure addition breakdown
    std::cout << "\nPURE ADDITION BREAKDOWN:\n";
    std::cout << "========================\n\n";
    
    for (auto& t : tests) {
        auto wL = cc->EvalSub(t.ct_l, make_ct(2.0));
        auto wR = cc->EvalAdd(t.ct_r, make_ct(2.0));
        
        auto LC = cc->EvalSub(cc->EvalAdd(t.ct_l, t.ct_c), make_ct(1.0));
        auto CR = cc->EvalSub(cc->EvalAdd(t.ct_c, t.ct_r), make_ct(1.0));
        auto LR = cc->EvalSub(cc->EvalAdd(t.ct_l, t.ct_r), make_ct(1.0));
        
        std::cout << "  Pattern " << t.pattern << ":\n";
        std::cout << "    wL=" << decrypt_val(wL)
                  << " C=" << decrypt_val(t.ct_c)
                  << " wR=" << decrypt_val(wR) << "\n";
        std::cout << "    LC=" << decrypt_val(LC)
                  << " CR=" << decrypt_val(CR)
                  << " LR=" << decrypt_val(LR) << "\n\n";
    }
    
    std::cout << "  Level: " << eval_rule110_phi(ct_phi, ct_phi, ct_phi)->GetLevel() << "\n";
    std::cout << "  φ-pure-addition transformation complete\n";
    std::cout << "  ZERO MULTIPLICATIONS USED\n";

    return 0;
}
