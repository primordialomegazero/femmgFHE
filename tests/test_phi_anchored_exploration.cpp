// ============================================
// φ-ANCHORED EXPLORATION
// Pure golden ratio anchoring
//
// Core approach:
// - Lahat ay naka-anchor sa φ
// - I-explore ang φ-space nang walang assumptions
// - Obserbahan ang emergent properties
// - Walang arbitrary constants — φ lang
//
// Author: Dan Fernandez / Primordial Omega Zero
// ============================================

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <cmath>
#include <complex>
#include <algorithm>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  φ-ANCHORED EXPLORATION\n";
    std::cout << "  Pure Golden Ratio Anchoring\n";
    std::cout << "========================================\n\n";

    const double PHI = 1.6180339887498948482;
    const double PSI = -0.6180339887498948482;
    const double PHI2 = PHI * PHI;
    const double PHI3 = PHI2 * PHI;
    const double INV_PHI = 1.0 / PHI;
    const double INV_PHI2 = 1.0 / PHI2;
    const double INV_PHI3 = 1.0 / PHI3;
    const double GOLDEN_ANGLE = 137.50776405003785;

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

    // ========== φ-ANCHORED RULE 110 ==========
    // Lahat ng weights ay φ-powers — walang arbitrary constants
    // I-explore ang iba't ibang φ-anchoring
    
    auto eval_rule110_phi = [&](auto L, auto C, auto R) {
        // φ-anchored linear weights
        auto wL = cc->EvalMult(L, make_ct(INV_PHI3));  // φ^(-3)
        auto wC = cc->EvalMult(C, make_ct(INV_PHI));    // φ^(-1)
        auto wR = cc->EvalMult(R, make_ct(PHI3));       // φ^(+3)
        
        // φ-anchored pairwise weights
        auto LC = cc->EvalMult(L, C);
        auto CR = cc->EvalMult(C, R);
        auto LR = cc->EvalMult(L, R);
        
        auto wLC = cc->EvalMult(LC, make_ct(INV_PHI2)); // φ^(-2)
        auto wCR = cc->EvalMult(CR, make_ct(PHI2));      // φ^(+2)
        auto wLR = cc->EvalMult(LR, make_ct(INV_PHI));   // φ^(-1)
        
        // φ-anchored combination
        auto sum_linear = cc->EvalAdd(wL, wC);
        sum_linear = cc->EvalAdd(sum_linear, wR);
        
        auto sum_pairwise = cc->EvalAdd(wLC, wCR);
        sum_pairwise = cc->EvalAdd(sum_pairwise, wLR);
        
        // Triple interaction na may φ-anchor
        auto LCR = cc->EvalMult(LR, C);
        auto wLCR = cc->EvalMult(LCR, make_ct(PHI));
        
        // Complete φ-anchored formula
        auto result = cc->EvalSub(sum_linear, sum_pairwise);
        result = cc->EvalAdd(result, wLCR);
        
        return result;
    };

    auto ct_phi = make_ct(PHI);
    auto ct_psi = make_ct(PSI);

    std::cout << "φ-ANCHORED TEST:\n";
    std::cout << "================\n\n";
    std::cout << "  φ = " << PHI << "\n";
    std::cout << "  ψ = " << PSI << "\n";
    std::cout << "  φ³ = " << PHI3 << "\n";
    std::cout << "  1/φ³ = " << INV_PHI3 << "\n\n";

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
        
        // φ-natural threshold: 0
        double collapsed = (val > 0) ? PHI : PSI;
        
        bool match = (collapsed == t.expected);
        if (match) correct++;
        
        std::cout << "  (" << t.pattern << ") → " << val
                  << " → " << collapsed
                  << " (expected " << t.expected << ")"
                  << (match ? " ✓" : " ✗") << "\n";
    }

    std::cout << "\n  Rule 110 sa φ-anchored: " << correct << "/8\n";
    
    // φ-anchored breakdown
    std::cout << "\nφ-ANCHORED BREAKDOWN:\n";
    std::cout << "=====================\n\n";
    
    for (auto& t : tests) {
        auto wL = cc->EvalMult(t.ct_l, make_ct(INV_PHI3));
        auto wC = cc->EvalMult(t.ct_c, make_ct(INV_PHI));
        auto wR = cc->EvalMult(t.ct_r, make_ct(PHI3));
        
        auto LC = cc->EvalMult(t.ct_l, t.ct_c);
        auto CR = cc->EvalMult(t.ct_c, t.ct_r);
        auto LR = cc->EvalMult(t.ct_l, t.ct_r);
        
        auto wLC = cc->EvalMult(LC, make_ct(INV_PHI2));
        auto wCR = cc->EvalMult(CR, make_ct(PHI2));
        auto wLR = cc->EvalMult(LR, make_ct(INV_PHI));
        
        auto LCR = cc->EvalMult(LR, t.ct_c);
        auto wLCR = cc->EvalMult(LCR, make_ct(PHI));
        
        std::cout << "  Pattern " << t.pattern << ":\n";
        std::cout << "    wL=" << decrypt_val(wL)
                  << " wC=" << decrypt_val(wC)
                  << " wR=" << decrypt_val(wR) << "\n";
        std::cout << "    wLC=" << decrypt_val(wLC)
                  << " wCR=" << decrypt_val(wCR)
                  << " wLR=" << decrypt_val(wLR) << "\n";
        std::cout << "    wLCR=" << decrypt_val(wLCR) << "\n\n";
    }
    
    std::cout << "  Level: " << eval_rule110_phi(ct_phi, ct_phi, ct_phi)->GetLevel() << "\n";
    std::cout << "  φ-ANCHORED EXPLORATION COMPLETE\n";

    return 0;
}
