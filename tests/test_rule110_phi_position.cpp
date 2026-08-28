// ============================================
// RULE 110 SA φ-POSITION SPACE
// Position-aware φ-polynomial
//
// Core fix:
// - Ang left, center, right ay may iba't ibang φ-weights
// - φ^(-2)×L + φ^0×C + φ^(+2)×R - φ^(-1)×LC - φ^(+1)×CR + LR
// - Ang position asymmetry ang nagdi-distinguish sa 001 vs 100
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
    std::cout << "  RULE 110 SA φ-POSITION SPACE\n";
    std::cout << "  Position-Aware φ-Polynomial\n";
    std::cout << "========================================\n\n";

    const double PHI = 1.6180339887498948482;
    const double PSI = -0.6180339887498948482;
    const double MIDPOINT = (PHI + PSI) / 2.0;
    const double RANGE = PHI - PSI;
    const double PHI2 = PHI * PHI;
    const double INV_PHI = 1.0 / PHI;
    const double INV_PHI2 = 1.0 / PHI2;

    CCParams<CryptoContextCKKSRNS> params;
    params.SetMultiplicativeDepth(4);
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

    // ========== φ-POSITION-AWARE RULE 110 ==========
    // output = φ^(-2)×L + φ^0×C + φ^(+2)×R 
    //          - φ^(-1)×LC - φ^(+1)×CR + LR
    // 4 multiplications total
    
    auto eval_rule110_phi = [&](auto L, auto C, auto R) {
        // Position-weighted linear terms
        auto wL = cc->EvalMult(L, make_ct(INV_PHI2));
        auto wR = cc->EvalMult(R, make_ct(PHI2));
        
        // Pairwise interaction terms
        auto LC = cc->EvalMult(L, C);
        auto CR = cc->EvalMult(C, R);
        auto LR = cc->EvalMult(L, R);
        
        auto wLC = cc->EvalMult(LC, make_ct(INV_PHI));
        auto wCR = cc->EvalMult(CR, make_ct(PHI));
        
        // Combine: linear - pairwise + triple
        auto sum_linear = cc->EvalAdd(wL, C);
        sum_linear = cc->EvalAdd(sum_linear, wR);
        
        auto sum_pairwise = cc->EvalAdd(wLC, wCR);
        sum_pairwise = cc->EvalAdd(sum_pairwise, LR);
        
        auto result = cc->EvalSub(sum_linear, sum_pairwise);
        
        return result;
    };

    auto ct_phi = make_ct(PHI);
    auto ct_psi = make_ct(PSI);

    std::cout << "RULE 110 SA φ-POSITION SPACE:\n";
    std::cout << "=============================\n\n";
    std::cout << "  ψ = " << PSI << " (False)\n";
    std::cout << "  φ = " << PHI << " (True)\n";
    std::cout << "  Weights: [" << INV_PHI2 << ", 1, " << PHI2 << "]\n";
    std::cout << "  Pairwise: [" << INV_PHI << ", " << PHI << ", 1]\n\n";

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
        
        // Check saang side ng midpoint
        double collapsed = (val > MIDPOINT) ? PHI : PSI;
        
        bool match = (collapsed == t.expected);
        if (match) correct++;
        
        std::cout << "  (" << t.pattern << ") → " << val
                  << " → " << collapsed
                  << " (expected " << t.expected << ")"
                  << (match ? " ✓" : " ✗") << "\n";
    }

    std::cout << "\n  Rule 110 sa φ-position space: " << correct << "/8\n";
    
    // Show position-aware breakdown
    std::cout << "\nPOSITION BREAKDOWN:\n";
    std::cout << "===================\n\n";
    
    for (auto& t : tests) {
        auto wL = cc->EvalMult(t.ct_l, make_ct(INV_PHI2));
        auto wR = cc->EvalMult(t.ct_r, make_ct(PHI2));
        
        auto LC = cc->EvalMult(t.ct_l, t.ct_c);
        auto CR = cc->EvalMult(t.ct_c, t.ct_r);
        auto LR = cc->EvalMult(t.ct_l, t.ct_r);
        
        auto wLC = cc->EvalMult(LC, make_ct(INV_PHI));
        auto wCR = cc->EvalMult(CR, make_ct(PHI));
        
        std::cout << "  Pattern " << t.pattern << ":\n";
        std::cout << "    wL=" << decrypt_val(wL)
                  << " C=" << decrypt_val(t.ct_c)
                  << " wR=" << decrypt_val(wR) << "\n";
        std::cout << "    wLC=" << decrypt_val(wLC)
                  << " wCR=" << decrypt_val(wCR)
                  << " LR=" << decrypt_val(LR) << "\n\n";
    }
    
    std::cout << "  Level: " << eval_rule110_phi(ct_phi, ct_phi, ct_phi)->GetLevel() << "\n";
    std::cout << "  φ-position transformation complete\n";

    return 0;
}
