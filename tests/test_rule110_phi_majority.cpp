// ============================================
// RULE 110 SA φ-MAJORITY SPACE
// Natural majority na may left exception
//
// Core insight:
// - Rule 110 ay majority logic
// - φ kung may φ sa center o right
// - ψ kung ψ sa center at right
// - Exception: φ lang sa left → ψ
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
    std::cout << "  RULE 110 SA φ-MAJORITY SPACE\n";
    std::cout << "  Natural Majority + Exception\n";
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

    // ========== φ-MAJORITY RULE 110 ==========
    // output = (C + R) - (L - C - R)²
    // (C + R): positive kung may φ sa center o right
    // (L - C - R)²: positive kung φ lang sa left
    
    auto eval_rule110_phi = [&](auto L, auto C, auto R) {
        // Step 1: Center + Right (majority signal)
        auto CR = cc->EvalAdd(C, R);
        
        // Step 2: Left - Center - Right (left exception signal)
        auto sum_CR = cc->EvalAdd(C, R);
        auto left_exception = cc->EvalSub(L, sum_CR);
        
        // Step 3: Square para sa left exception
        auto left_exception_sq = cc->EvalMult(left_exception, left_exception);
        
        // Step 4: Combine
        // Para sa normal: output = CR (positive → φ)
        // Para sa 100: left_exception_sq ay malaki → output = negative → ψ
        auto result = cc->EvalSub(CR, left_exception_sq);
        
        return result;
    };

    auto ct_phi = make_ct(PHI);
    auto ct_psi = make_ct(PSI);

    std::cout << "RULE 110 SA φ-MAJORITY SPACE:\n";
    std::cout << "=============================\n\n";
    std::cout << "  ψ = " << PSI << " (False)\n";
    std::cout << "  φ = " << PHI << " (True)\n";
    std::cout << "  Majority: (C+R) - (L-C-R)²\n\n";

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

    std::cout << "\n  Rule 110 sa φ-majority space: " << correct << "/8\n";
    
    // Show majority breakdown
    std::cout << "\nMAJORITY BREAKDOWN:\n";
    std::cout << "===================\n\n";
    
    for (auto& t : tests) {
        auto CR = cc->EvalAdd(t.ct_c, t.ct_r);
        auto sum_CR = cc->EvalAdd(t.ct_c, t.ct_r);
        auto left_exception = cc->EvalSub(t.ct_l, sum_CR);
        auto left_exception_sq = cc->EvalMult(left_exception, left_exception);
        
        std::cout << "  Pattern " << t.pattern << ":\n";
        std::cout << "    C+R=" << decrypt_val(CR)
                  << " L-C-R=" << decrypt_val(left_exception)
                  << " (L-C-R)²=" << decrypt_val(left_exception_sq) << "\n\n";
    }
    
    std::cout << "  Level: " << eval_rule110_phi(ct_phi, ct_phi, ct_phi)->GetLevel() << "\n";
    std::cout << "  φ-majority transformation complete\n";

    return 0;
}
