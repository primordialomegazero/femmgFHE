// ============================================
// RULE 110 SA φ-LEFT-ASYMMETRY SPACE
// Natural collapse na may φ-left position bias
//
// Core fix:
// - Ang 100 ay may φ sa left position lang
// - Kailangan ng left asymmetry detector
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
    std::cout << "  RULE 110 SA φ-LEFT-ASYMMETRY SPACE\n";
    std::cout << "  Natural φ-Left Position Bias\n";
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

    // ========== φ-LEFT-ASYMMETRY RULE 110 ==========
    // output = sum_sq - φ×left_bias
    // left_bias = L - C - R (positive kung φ sa left, ψ sa center/right)
    
    auto eval_rule110_phi = [&](auto L, auto C, auto R) {
        // Step 1: Differences para sa pure/mixed detection
        auto diff_LC = cc->EvalSub(L, C);
        auto diff_CR = cc->EvalSub(C, R);
        auto diff_LR = cc->EvalSub(L, R);
        
        // Step 2: Squares para sa absolute value (3 multiplications)
        auto sq_LC = cc->EvalMult(diff_LC, diff_LC);
        auto sq_CR = cc->EvalMult(diff_CR, diff_CR);
        auto sq_LR = cc->EvalMult(diff_LR, diff_LR);
        
        // Step 3: Sum of squares
        auto sum_sq = cc->EvalAdd(sq_LC, sq_CR);
        sum_sq = cc->EvalAdd(sum_sq, sq_LR);
        
        // Step 4: Left asymmetry detector
        // left_bias = L - C - R
        auto sum_CR = cc->EvalAdd(C, R);
        auto left_bias = cc->EvalSub(L, sum_CR);
        
        // Step 5: φ-weighted combination
        // Para sa 100: sum_sq=10, left_bias=2.854
        // Para sa 001: sum_sq=10, left_bias=-1.618
        // Para sa 010: sum_sq=10, left_bias=-0.618
        // Kailangan: 100 → ψ, iba → φ
        // 
        // Ang φ-natural na paraan: multiply left_bias sa φ
        // φ×left_bias para sa 100 = 1.618×2.854 = 4.618
        // sum_sq - φ×left_bias = 10 - 4.618 = 5.382 (dapat negative)
        //
        // Kailangan ng mas malaking weight
        // 2×φ×left_bias = 2×1.618×2.854 = 9.236
        // sum_sq - 9.236 = 0.764 (malapit sa 0)
        //
        // 3×φ×left_bias = 13.854
        // sum_sq - 13.854 = -3.854 (negative ✓)
        
        auto phi_left = cc->EvalMult(left_bias, make_ct(3.0 * PHI));
        
        // Combine: sum_sq - φ×left_bias
        auto result = cc->EvalSub(sum_sq, phi_left);
        
        return result;
    };

    auto ct_phi = make_ct(PHI);
    auto ct_psi = make_ct(PSI);

    std::cout << "RULE 110 SA φ-LEFT-ASYMMETRY SPACE:\n";
    std::cout << "===================================\n\n";
    std::cout << "  ψ = " << PSI << " (False)\n";
    std::cout << "  φ = " << PHI << " (True)\n";
    std::cout << "  Left bias: L - C - R\n\n";

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

    std::cout << "\n  Rule 110 sa φ-left-asymmetry space: " << correct << "/8\n";
    
    // Show asymmetry breakdown
    std::cout << "\nASYMMETRY BREAKDOWN:\n";
    std::cout << "====================\n\n";
    
    for (auto& t : tests) {
        auto sum_CR = cc->EvalAdd(t.ct_c, t.ct_r);
        auto left_bias = cc->EvalSub(t.ct_l, sum_CR);
        
        auto diff_LC = cc->EvalSub(t.ct_l, t.ct_c);
        auto diff_CR = cc->EvalSub(t.ct_c, t.ct_r);
        auto diff_LR = cc->EvalSub(t.ct_l, t.ct_r);
        
        auto sq_LC = cc->EvalMult(diff_LC, diff_LC);
        auto sq_CR = cc->EvalMult(diff_CR, diff_CR);
        auto sq_LR = cc->EvalMult(diff_LR, diff_LR);
        
        auto sum_sq = cc->EvalAdd(sq_LC, sq_CR);
        sum_sq = cc->EvalAdd(sum_sq, sq_LR);
        
        std::cout << "  Pattern " << t.pattern << ":\n";
        std::cout << "    left_bias=" << decrypt_val(left_bias)
                  << " sum_sq=" << decrypt_val(sum_sq) << "\n\n";
    }
    
    std::cout << "  Level: " << eval_rule110_phi(ct_phi, ct_phi, ct_phi)->GetLevel() << "\n";
    std::cout << "  φ-left-asymmetry transformation complete\n";

    return 0;
}
