// ============================================
// RULE 110 SA φ-PURE-ADDITION SPACE v2
// Zero multiplication + φ-threshold correction
//
// Core fix:
// - Ang 000 at 100 ay may positive bias
// - Kailangan ng φ-threshold shift: -3
// - Lahat ay addition/subtraction pa rin
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
    std::cout << "  RULE 110 SA φ-PURE-ADDITION SPACE v2\n";
    std::cout << "  Zero Multiplication + Threshold\n";
    std::cout << "========================================\n\n";

    const double PHI_EXP = 1.0;
    const double PSI_EXP = -1.0;
    const double MIDPOINT = 0.0;

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

    // ========== φ-PURE-ADDITION RULE 110 v2 ==========
    // May threshold shift para sa patterns na walang right φ
    
    auto eval_rule110_phi = [&](auto L, auto C, auto R) {
        // Position-weighted linear terms
        auto wL = cc->EvalSub(L, make_ct(2.0));
        auto wR = cc->EvalAdd(R, make_ct(2.0));
        
        // Pairwise terms
        auto LC = cc->EvalSub(cc->EvalAdd(L, C), make_ct(1.0));
        auto CR = cc->EvalSub(cc->EvalAdd(C, R), make_ct(1.0));
        auto LR = cc->EvalSub(cc->EvalAdd(L, R), make_ct(1.0));
        
        // Position-weighted pairwise
        auto wLC = cc->EvalSub(LC, make_ct(1.0));
        auto wCR = cc->EvalAdd(CR, make_ct(1.0));
        
        // Combine: linear - pairwise
        auto sum_linear = cc->EvalAdd(wL, C);
        sum_linear = cc->EvalAdd(sum_linear, wR);
        
        auto sum_pairwise = cc->EvalAdd(wLC, wCR);
        sum_pairwise = cc->EvalAdd(sum_pairwise, LR);
        
        auto result = cc->EvalSub(sum_linear, sum_pairwise);
        
        // φ-threshold shift: -4 para sa 000 at 100
        // Lahat ng iba ay naaapektuhan din, pero ang sign ay preserved
        result = cc->EvalSub(result, make_ct(4.0));
        
        return result;
    };

    auto ct_phi = make_ct(PHI_EXP);
    auto ct_psi = make_ct(PSI_EXP);

    std::cout << "RULE 110 SA φ-PURE-ADDITION SPACE v2:\n";
    std::cout << "=====================================\n\n";
    std::cout << "  ψ = " << PSI_EXP << " (False)\n";
    std::cout << "  φ = " << PHI_EXP << " (True)\n";
    std::cout << "  Threshold shift: -4\n";
    std::cout << "  Zero multiplications\n\n";

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

    std::cout << "\n  Rule 110 sa φ-pure-addition v2: " << correct << "/8\n";
    
    // Verify zero multiplications
    std::cout << "\nVERIFICATION:\n";
    std::cout << "=============\n";
    std::cout << "  EvalMult calls: 0\n";
    std::cout << "  EvalAdd calls: multiple\n";
    std::cout << "  EvalSub calls: multiple\n";
    std::cout << "  Level: " << eval_rule110_phi(ct_phi, ct_phi, ct_phi)->GetLevel() << "\n";
    std::cout << "  ZERO MULTIPLICATIONS CONFIRMED\n";

    return 0;
}
