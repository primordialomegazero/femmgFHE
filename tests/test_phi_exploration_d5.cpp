// ============================================
// φ-EXPLORATION DEPTH 5
// Safe φ-anchoring na may tamang depth
//
// Core fix:
// - Depth 5 para sa triple interaction
// - φ-bounded values para hindi sumabog
// - Lahat ay φ-powers — walang arbitrary constants
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
    std::cout << "  φ-EXPLORATION DEPTH 5\n";
    std::cout << "  Safe φ-Anchoring\n";
    std::cout << "========================================\n\n";

    const double PHI = 1.6180339887498948482;
    const double PSI = -0.6180339887498948482;
    const double PHI2 = PHI * PHI;
    const double PHI3 = PHI2 * PHI;
    const double INV_PHI = 1.0 / PHI;
    const double INV_PHI2 = 1.0 / PHI2;
    const double INV_PHI3 = 1.0 / PHI3;

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

    // ========== φ-EXPLORATION RULE 110 ==========
    // I-explore ang iba't ibang φ-anchoring
    // Lahat ay φ-powers — walang arbitrary constants
    
    auto eval_rule110_phi = [&](auto L, auto C, auto R) {
        // φ-anchored linear weights
        auto wL = cc->EvalMult(L, make_ct(INV_PHI2));
        auto wR = cc->EvalMult(R, make_ct(PHI2));
        
        // φ-anchored pairwise weights
        auto LC = cc->EvalMult(L, C);
        auto CR = cc->EvalMult(C, R);
        auto LR = cc->EvalMult(L, R);
        
        auto wLC = cc->EvalMult(LC, make_ct(INV_PHI));
        auto wCR = cc->EvalMult(CR, make_ct(PHI));
        
        // φ-anchored combination (walang triple para sa safety)
        auto sum_linear = cc->EvalAdd(wL, C);
        sum_linear = cc->EvalAdd(sum_linear, wR);
        
        auto sum_pairwise = cc->EvalAdd(wLC, wCR);
        sum_pairwise = cc->EvalAdd(sum_pairwise, LR);
        
        auto result = cc->EvalSub(sum_linear, sum_pairwise);
        
        return result;
    };

    auto ct_phi = make_ct(PHI);
    auto ct_psi = make_ct(PSI);

    std::cout << "φ-EXPLORATION DEPTH 5:\n";
    std::cout << "======================\n\n";
    std::cout << "  φ = " << PHI << "\n";
    std::cout << "  ψ = " << PSI << "\n";
    std::cout << "  φ² = " << PHI2 << "\n";
    std::cout << "  1/φ² = " << INV_PHI2 << "\n\n";

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
        
        double collapsed = (val > 0) ? PHI : PSI;
        
        bool match = (collapsed == t.expected);
        if (match) correct++;
        
        std::cout << "  (" << t.pattern << ") → " << val
                  << " → " << collapsed
                  << " (expected " << t.expected << ")"
                  << (match ? " ✓" : " ✗") << "\n";
    }

    std::cout << "\n  Rule 110 sa φ-exploration: " << correct << "/8\n";
    
    // φ-signature analysis
    std::cout << "\nφ-SIGNATURE ANALYSIS:\n";
    std::cout << "=====================\n\n";
    
    for (auto& t : tests) {
        auto wL = cc->EvalMult(t.ct_l, make_ct(INV_PHI2));
        auto wR = cc->EvalMult(t.ct_r, make_ct(PHI2));
        
        auto LC = cc->EvalMult(t.ct_l, t.ct_c);
        auto CR = cc->EvalMult(t.ct_c, t.ct_r);
        auto LR = cc->EvalMult(t.ct_l, t.ct_r);
        
        auto wLC = cc->EvalMult(LC, make_ct(INV_PHI));
        auto wCR = cc->EvalMult(CR, make_ct(PHI));
        
        auto sum_linear = cc->EvalAdd(wL, t.ct_c);
        sum_linear = cc->EvalAdd(sum_linear, wR);
        
        auto sum_pairwise = cc->EvalAdd(wLC, wCR);
        sum_pairwise = cc->EvalAdd(sum_pairwise, LR);
        
        auto result = cc->EvalSub(sum_linear, sum_pairwise);
        
        double val = decrypt_val(result);
        std::cout << "  Pattern " << t.pattern << ": "
                  << val << " → "
                  << (val > 0 ? "φ (positive)" : "ψ (negative)") << "\n";
    }
    
    // Explore additional φ-properties
    std::cout << "\nADDITIONAL φ-PROPERTIES:\n";
    std::cout << "========================\n\n";
    
    // Check kung ang result values ay may φ-pattern
    std::vector<double> results;
    for (auto& t : tests) {
        auto result = eval_rule110_phi(t.ct_l, t.ct_c, t.ct_r);
        results.push_back(decrypt_val(result));
    }
    
    // Sort at tingnan ang distribution
    std::sort(results.begin(), results.end());
    
    std::cout << "  Result distribution:\n";
    for (size_t i = 0; i < results.size(); i++) {
        std::cout << "    " << results[i];
        if (i < results.size() - 1) std::cout << ", ";
    }
    std::cout << "\n\n";
    
    // Check kung may φ-ratios
    for (size_t i = 1; i < results.size(); i++) {
        if (results[i-1] != 0) {
            double ratio = results[i] / results[i-1];
            if (std::abs(ratio - PHI) < 0.5 || std::abs(ratio - INV_PHI) < 0.5) {
                std::cout << "  φ-ratio found: " << results[i] << "/" << results[i-1] 
                          << " = " << ratio << "\n";
            }
        }
    }
    
    std::cout << "\n  Level: " << eval_rule110_phi(ct_phi, ct_phi, ct_phi)->GetLevel() << "\n";
    std::cout << "  φ-EXPLORATION COMPLETE\n";

    return 0;
}
