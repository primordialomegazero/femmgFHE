// ============================================
// RULE 110 SA φ-DUALITY SPACE
// Final na may pure state detection
//
// Core fix:
// - 000 at 111 ay pure states → ψ
// - Pure state detector: (L+C+R)² 
// - Para sa pure: (3ψ)² o (3φ)² = malaki
// - Para sa mixed: mas maliit
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
    std::cout << "  RULE 110 SA φ-DUALITY SPACE\n";
    std::cout << "  Pure State Detection\n";
    std::cout << "========================================\n\n";

    const double PHI = 1.6180339887498948482;
    const double PSI = -0.6180339887498948482;
    const double MIDPOINT = (PHI + PSI) / 2.0;
    const double RANGE = PHI - PSI;

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

    // ========== φ-DUALITY RULE 110 ==========
    auto eval_rule110_phi = [&](auto L, auto C, auto R) {
        // Step 1: Differences
        auto diff_LC = cc->EvalSub(L, C);
        auto diff_CR = cc->EvalSub(C, R);
        auto diff_LR = cc->EvalSub(L, R);
        
        // Step 2: Squares
        auto sq_LC = cc->EvalMult(diff_LC, diff_LC);
        auto sq_CR = cc->EvalMult(diff_CR, diff_CR);
        auto sq_LR = cc->EvalMult(diff_LR, diff_LR);
        
        // Step 3: Sum of squares
        auto sum_sq = cc->EvalAdd(sq_LC, sq_CR);
        sum_sq = cc->EvalAdd(sum_sq, sq_LR);
        
        // Step 4: Left asymmetry
        auto sum_CR = cc->EvalAdd(C, R);
        auto left_bias = cc->EvalSub(L, sum_CR);
        auto phi_left = cc->EvalMult(left_bias, make_ct(3.0 * PHI));
        
        // Step 5: Pure state detector
        auto sum_all = cc->EvalAdd(L, C);
        sum_all = cc->EvalAdd(sum_all, R);
        auto pure_signal = cc->EvalMult(sum_all, sum_all);
        auto phi2_pure = cc->EvalMult(pure_signal, make_ct(PHI * PHI));
        
        // Combine
        auto result = cc->EvalSub(sum_sq, phi_left);
        result = cc->EvalSub(result, phi2_pure);
        
        return result;
    };

    auto ct_phi = make_ct(PHI);
    auto ct_psi = make_ct(PSI);

    std::cout << "RULE 110 SA φ-DUALITY SPACE:\n";
    std::cout << "============================\n\n";
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

    std::cout << "\n  Rule 110 sa φ-duality space: " << correct << "/8\n";
    
    // Show duality breakdown
    std::cout << "\nDUALITY BREAKDOWN:\n";
    std::cout << "==================\n\n";
    
    for (auto& t : tests) {
        auto sum_all = cc->EvalAdd(t.ct_l, t.ct_c);
        sum_all = cc->EvalAdd(sum_all, t.ct_r);
        auto pure_signal = cc->EvalMult(sum_all, sum_all);
        
        auto sum_CR = cc->EvalAdd(t.ct_c, t.ct_r);
        auto left_bias = cc->EvalSub(t.ct_l, sum_CR);
        
        std::cout << "  Pattern " << t.pattern << ":\n";
        std::cout << "    sum_all=" << decrypt_val(sum_all)
                  << " pure_signal=" << decrypt_val(pure_signal)
                  << " left_bias=" << decrypt_val(left_bias) << "\n\n";
    }
    
    std::cout << "  Level: " << eval_rule110_phi(ct_phi, ct_phi, ct_phi)->GetLevel() << "\n";
    std::cout << "  φ-duality transformation complete\n";

    return 0;
}
