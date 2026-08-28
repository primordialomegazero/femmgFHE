// ============================================
// RULE 110 SA φ-SYMMETRIC SPACE
// Elementary symmetric polynomials na may φ-scaling
//
// Core insight:
// - Rule 110 ay hindi parity logic
// - Ito ay φ-context logic na nangangailangan ng position info
// - Symmetric polynomials: e1 = sum, e2 = pairwise, e3 = triple
// - output = φ×e1 - e2 + φ^(-1)×e3
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
    std::cout << "  RULE 110 SA φ-SYMMETRIC SPACE\n";
    std::cout << "  Elementary Symmetric Polynomials\n";
    std::cout << "========================================\n\n";

    const double PHI = 1.6180339887498948482;
    const double PSI = -0.6180339887498948482;
    const double MIDPOINT = (PHI + PSI) / 2.0;
    const double RANGE = PHI - PSI;
    const double INV_PHI = 1.0 / PHI;

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

    // ========== φ-SYMMETRIC RULE 110 ==========
    // output = φ×(L+C+R) - (L×C + C×R + L×R) + φ^(-1)×(L×C×R)
    // 3 multiplications total, depth-4 safe
    
    auto eval_rule110_phi = [&](auto L, auto C, auto R) {
        // e1 = L + C + R (0 multiplications)
        auto e1 = cc->EvalAdd(cc->EvalAdd(L, C), R);
        
        // e2 = L×C + C×R + L×R (3 multiplications)
        auto LC = cc->EvalMult(L, C);
        auto CR = cc->EvalMult(C, R);
        auto LR = cc->EvalMult(L, R);
        auto e2 = cc->EvalAdd(cc->EvalAdd(LC, CR), LR);
        
        // e3 = L×C×R (1 multiplication on top of LC)
        auto e3 = cc->EvalMult(LC, R);
        
        // φ-scaled combination
        auto phi_e1 = cc->EvalMult(e1, make_ct(PHI));
        auto inv_phi_e3 = cc->EvalMult(e3, make_ct(INV_PHI));
        
        auto result = cc->EvalSub(phi_e1, e2);
        result = cc->EvalAdd(result, inv_phi_e3);
        
        return result;
    };

    auto ct_phi = make_ct(PHI);
    auto ct_psi = make_ct(PSI);

    std::cout << "RULE 110 SA φ-SYMMETRIC SPACE:\n";
    std::cout << "==============================\n\n";
    std::cout << "  ψ = " << PSI << " (False)\n";
    std::cout << "  φ = " << PHI << " (True)\n";
    std::cout << "  Formula: φ×e1 - e2 + φ^(-1)×e3\n\n";

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

    std::cout << "\n  Rule 110 sa φ-symmetric space: " << correct << "/8\n";
    
    // Show symmetric polynomial breakdown
    std::cout << "\nSYMMETRIC BREAKDOWN:\n";
    std::cout << "====================\n\n";
    
    for (auto& t : tests) {
        auto e1 = cc->EvalAdd(cc->EvalAdd(t.ct_l, t.ct_c), t.ct_r);
        auto LC = cc->EvalMult(t.ct_l, t.ct_c);
        auto CR = cc->EvalMult(t.ct_c, t.ct_r);
        auto LR = cc->EvalMult(t.ct_l, t.ct_r);
        auto e2 = cc->EvalAdd(cc->EvalAdd(LC, CR), LR);
        auto e3 = cc->EvalMult(LC, t.ct_r);
        
        std::cout << "  Pattern " << t.pattern << ":\n";
        std::cout << "    e1=" << decrypt_val(e1)
                  << " e2=" << decrypt_val(e2)
                  << " e3=" << decrypt_val(e3) << "\n\n";
    }
    
    std::cout << "  Level: " << eval_rule110_phi(ct_phi, ct_phi, ct_phi)->GetLevel() << "\n";
    std::cout << "  φ-symmetric transformation complete\n";

    return 0;
}
