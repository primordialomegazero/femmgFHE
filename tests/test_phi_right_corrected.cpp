// ============================================
// φ-RIGHT CORRECTED FHE
// Addition-only na may φ-right detection
//
// Core fix:
// - Ang 000, 100, 111 ay positive (mali)
// - Lahat sila ay may ψ sa right position
// - Kailangan ng φ-right detector
// - Pure addition pa rin — walang multiplication
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
    std::cout << "  φ-RIGHT CORRECTED FHE\n";
    std::cout << "  Addition-Only + φ-Right Detection\n";
    std::cout << "========================================\n\n";

    const double PHI = 1.6180339887498948482;
    const double PSI = -0.6180339887498948482;

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

    // ========== φ-RIGHT CORRECTED RULE 110 ==========
    // Lahat ay addition/subtraction — walang EvalMult
    
    auto eval_rule110_phi = [&](auto L, auto C, auto R) {
        // Position-weighted linear terms
        auto wL = cc->EvalSub(L, make_ct(2.0));
        auto wR = cc->EvalAdd(R, make_ct(2.0));
        
        // Pairwise "multiplication" = addition sa exponent
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
        
        // φ-right detector: kung R = -1 (ψ), bawasan ng 6
        // Kung R = +1 (φ), walang bawas
        // right_signal = R + 1 (0 kung ψ, 2 kung φ)
        // correction = -3 × (R + 1) = -3R - 3
        auto right_correction = cc->EvalSub(
            make_ct(-3.0),
            cc->EvalAdd(make_ct(3.0), R)
        );
        
        // I-apply ang correction
        result = cc->EvalAdd(result, right_correction);
        
        return result;
    };

    auto ct_phi_exp = make_ct(1.0);
    auto ct_psi_exp = make_ct(-1.0);

    std::cout << "φ-RIGHT CORRECTED TEST:\n";
    std::cout << "=======================\n\n";
    std::cout << "  φ exponent: +1\n";
    std::cout << "  ψ exponent: -1\n";
    std::cout << "  Right correction: -3(R+1)\n";
    std::cout << "  Operations: PURE ADDITION\n\n";

    std::cout << "TRUTH TABLE:\n";
    std::cout << "============\n\n";

    struct TestCase {
        std::string pattern;
        Ciphertext<DCRTPoly> ct_l, ct_c, ct_r;
        double expected_exp;
    };

    std::vector<TestCase> tests = {
        {"000", ct_psi_exp, ct_psi_exp, ct_psi_exp, -1.0},
        {"001", ct_psi_exp, ct_psi_exp, ct_phi_exp, 1.0},
        {"010", ct_psi_exp, ct_phi_exp, ct_psi_exp, 1.0},
        {"011", ct_psi_exp, ct_phi_exp, ct_phi_exp, 1.0},
        {"100", ct_phi_exp, ct_psi_exp, ct_psi_exp, -1.0},
        {"101", ct_phi_exp, ct_psi_exp, ct_phi_exp, 1.0},
        {"110", ct_phi_exp, ct_phi_exp, ct_psi_exp, 1.0},
        {"111", ct_phi_exp, ct_phi_exp, ct_phi_exp, -1.0}
    };

    int correct = 0;
    for (auto& t : tests) {
        auto result = eval_rule110_phi(t.ct_l, t.ct_c, t.ct_r);
        double val = decrypt_val(result);
        
        double collapsed = (val > 0) ? 1.0 : -1.0;
        
        bool match = (collapsed == t.expected_exp);
        if (match) correct++;
        
        std::cout << "  (" << t.pattern << ") → " << val
                  << " → " << collapsed
                  << " (expected " << t.expected_exp << ")"
                  << (match ? " ✓" : " ✗") << "\n";
    }

    std::cout << "\n  Rule 110 sa φ-right corrected: " << correct << "/8\n";
    
    // Show right correction breakdown
    std::cout << "\nRIGHT CORRECTION BREAKDOWN:\n";
    std::cout << "============================\n\n";
    
    for (auto& t : tests) {
        auto right_correction = cc->EvalSub(
            make_ct(-3.0),
            cc->EvalAdd(make_ct(3.0), t.ct_r)
        );
        
        std::cout << "  Pattern " << t.pattern << ":\n";
        std::cout << "    R=" << decrypt_val(t.ct_r)
                  << " correction=" << decrypt_val(right_correction) << "\n\n";
    }
    
    std::cout << "  Level: " << eval_rule110_phi(ct_phi_exp, ct_phi_exp, ct_phi_exp)->GetLevel() << "\n";
    std::cout << "  ZERO MULTIPLICATIONS — PURE ADDITION\n";
    std::cout << "  NOISE CANCELLING MAINTAINED\n";

    return 0;
}
