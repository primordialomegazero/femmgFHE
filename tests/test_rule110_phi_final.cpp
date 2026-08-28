// ============================================
// RULE 110 SA φ-FINAL SPACE
// Position-encoded pure state detection
//
// Core fix:
// - Ang pure detector ay dapat position-aware
// - 000 at 111 lang ang pure
// - 011, 101, 110 ay mixed pa rin
// - φ-weights: [φ^(-2), 1, φ^(+2)]
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
    std::cout << "  RULE 110 SA φ-FINAL SPACE\n";
    std::cout << "  Position-Encoded Pure Detection\n";
    std::cout << "========================================\n\n";

    const double PHI = 1.6180339887498948482;
    const double PSI = -0.6180339887498948482;
    const double MIDPOINT = (PHI + PSI) / 2.0;
    const double RANGE = PHI - PSI;
    const double PHI2 = PHI * PHI;
    const double INV_PHI2 = 1.0 / PHI2;

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

    // ========== φ-FINAL RULE 110 ==========
    // gumagamit ng position-encoded weights
    
    auto eval_rule110_phi = [&](auto L, auto C, auto R) {
        // Position-weighted sum
        auto wL = cc->EvalMult(L, make_ct(INV_PHI2));
        auto wR = cc->EvalMult(R, make_ct(PHI2));
        
        auto weighted_sum = cc->EvalAdd(wL, C);
        weighted_sum = cc->EvalAdd(weighted_sum, wR);
        
        // Pure detector: (weighted_sum - expected_pure)²
        // Para sa 000: weighted_sum = (INV_PHI2 + 1 + PHI2)×ψ
        // Para sa 111: weighted_sum = (INV_PHI2 + 1 + PHI2)×φ
        auto expected_norm = make_ct(INV_PHI2 + 1.0 + PHI2);
        
        // Para sa pure states, weighted_sum = ±expected_norm
        // Para sa mixed, iba
        auto diff_from_pure = cc->EvalSub(weighted_sum, expected_norm);
        auto pure_signal = cc->EvalMult(diff_from_pure, diff_from_pure);
        
        // Left asymmetry para sa 100
        auto sum_CR = cc->EvalAdd(C, R);
        auto left_bias = cc->EvalSub(L, sum_CR);
        auto phi_left = cc->EvalMult(left_bias, make_ct(2.0 * PHI));
        
        // Difference-based mixed detection
        auto diff_LC = cc->EvalSub(L, C);
        auto diff_CR = cc->EvalSub(C, R);
        auto diff_LR = cc->EvalSub(L, R);
        
        auto sq_LC = cc->EvalMult(diff_LC, diff_LC);
        auto sq_CR = cc->EvalMult(diff_CR, diff_CR);
        auto sq_LR = cc->EvalMult(diff_LR, diff_LR);
        
        auto sum_sq = cc->EvalAdd(sq_LC, sq_CR);
        sum_sq = cc->EvalAdd(sum_sq, sq_LR);
        
        // Combine: sum_sq - φ_left - pure_signal
        auto result = cc->EvalSub(sum_sq, phi_left);
        result = cc->EvalSub(result, pure_signal);
        
        return result;
    };

    auto ct_phi = make_ct(PHI);
    auto ct_psi = make_ct(PSI);

    std::cout << "RULE 110 SA φ-FINAL SPACE:\n";
    std::cout << "=========================\n\n";
    std::cout << "  ψ = " << PSI << " (False)\n";
    std::cout << "  φ = " << PHI << " (True)\n";
    std::cout << "  Position-encoded pure detection\n\n";

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

    std::cout << "\n  Rule 110 sa φ-final space: " << correct << "/8\n";
    
    // Show position-encoded breakdown
    std::cout << "\nPOSITION-ENCODED BREAKDOWN:\n";
    std::cout << "============================\n\n";
    
    for (auto& t : tests) {
        auto wL = cc->EvalMult(t.ct_l, make_ct(INV_PHI2));
        auto wR = cc->EvalMult(t.ct_r, make_ct(PHI2));
        auto weighted_sum = cc->EvalAdd(wL, t.ct_c);
        weighted_sum = cc->EvalAdd(weighted_sum, wR);
        
        std::cout << "  Pattern " << t.pattern << ":\n";
        std::cout << "    wL=" << decrypt_val(wL)
                  << " C=" << decrypt_val(t.ct_c)
                  << " wR=" << decrypt_val(wR) << "\n";
        std::cout << "    weighted_sum=" << decrypt_val(weighted_sum) << "\n\n";
    }
    
    std::cout << "  Level: " << eval_rule110_phi(ct_phi, ct_phi, ct_phi)->GetLevel() << "\n";
    std::cout << "  φ-final transformation complete\n";

    return 0;
}
