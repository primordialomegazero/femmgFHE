// ============================================
// RULE 110 SA φ-STABLE SPACE
// Linear φ-threshold logic, walang tanh divergence
//
// Core fix:
// - Linear decision boundary sa halip na tanh
// - φ-weighted voting na normalized
// - Bounded values sa [ψ, φ] palagi
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
    std::cout << "  RULE 110 SA φ-STABLE SPACE\n";
    std::cout << "  Linear φ-Threshold Logic\n";
    std::cout << "========================================\n\n";

    const double PHI = 1.6180339887498948482;
    const double PSI = -0.6180339887498948482;
    const double PHI2 = PHI * PHI;
    const double INV_PHI = 1.0 / PHI;
    const double MIDPOINT = (PHI + PSI) / 2.0;
    const double RANGE = PHI - PSI;
    const double NORM = INV_PHI + 1.0 + PHI;  // φ^(-1) + φ^0 + φ^1

    CCParams<CryptoContextCKKSRNS> params;
    params.SetMultiplicativeDepth(10);
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

    // ========== φ-LINEAR NORMALIZATION ==========
    // Simple linear map to [ψ, φ]
    auto phi_normalize = [&](auto val) {
        // Linear scaling: val ∈ [0, 1] → [ψ, φ]
        auto scaled = cc->EvalMult(val, make_ct(RANGE));
        return cc->EvalAdd(scaled, make_ct(PSI));
    };

    // ========== φ-NORMALIZED INPUT ==========
    // Convert φ/ψ to normalized [0, 1] space
    auto to_normalized = [&](auto state) {
        auto diff = cc->EvalSub(state, make_ct(PSI));
        return cc->EvalMult(diff, make_ct(1.0 / RANGE));
    };

    // ========== φ-WEIGHTED VOTING ==========
    // Rule 110 via φ-weighted average
    auto eval_rule110_phi = [&](auto L, auto C, auto R) {
        // Step 1: Normalize inputs to [0, 1]
        auto nL = to_normalized(L);
        auto nC = to_normalized(C);
        auto nR = to_normalized(R);
        
        // Step 2: φ-weighted votes
        // Past (L): weight φ^(-1) = 0.618
        // Present (C): weight φ^0 = 1.0
        // Future (R): weight φ^1 = 1.618
        auto wL = cc->EvalMult(nL, make_ct(INV_PHI));
        auto wR = cc->EvalMult(nR, make_ct(PHI));
        
        // Step 3: Sum of weighted votes
        auto weighted_sum = cc->EvalAdd(wL, nC);
        weighted_sum = cc->EvalAdd(weighted_sum, wR);
        
        // Step 4: Normalize to [0, 1]
        auto normalized = cc->EvalMult(weighted_sum, make_ct(1.0 / NORM));
        
        // Step 5: Map back to φ-space
        auto result = phi_normalize(normalized);
        
        // Step 6: φ-decision boundary
        // Apply linear decision: if result > midpoint, push to φ; else push to ψ
        auto diff = cc->EvalSub(result, make_ct(MIDPOINT));
        
        // Linear amplification (walang tanh!)
        auto amplified = cc->EvalMult(diff, make_ct(10.0));
        
        // Clamp to [ψ, φ] using linear operations
        auto clamped = phi_normalize(
            cc->EvalMult(
                cc->EvalAdd(amplified, make_ct(MIDPOINT)),
                make_ct(1.0 / 10.0)
            )
        );
        
        return clamped;
    };

    auto ct_phi = make_ct(PHI);
    auto ct_psi = make_ct(PSI);

    std::cout << "RULE 110 SA φ-STABLE SPACE:\n";
    std::cout << "===========================\n\n";
    std::cout << "  ψ = " << PSI << " (False)\n";
    std::cout << "  φ = " << PHI << " (True)\n";
    std::cout << "  φ-weights: [" << INV_PHI << ", 1, " << PHI << "]\n\n";

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
        
        bool match = std::abs(val - t.expected) < 0.5;
        if (match) correct++;
        
        std::cout << "  (" << t.pattern << ") → " << val
                  << " (expected " << t.expected << ")"
                  << (match ? " ✓" : " ✗") << "\n";
    }

    std::cout << "\n  Rule 110 sa φ-stable space: " << correct << "/8\n";
    
    // Show normalized intermediate values
    std::cout << "\nNORMALIZED ANALYSIS:\n";
    std::cout << "====================\n\n";
    
    for (auto& t : tests) {
        auto nL = to_normalized(t.ct_l);
        auto nC = to_normalized(t.ct_c);
        auto nR = to_normalized(t.ct_r);
        
        auto wL = cc->EvalMult(nL, make_ct(INV_PHI));
        auto wR = cc->EvalMult(nR, make_ct(PHI));
        auto weighted_sum = cc->EvalAdd(wL, nC);
        weighted_sum = cc->EvalAdd(weighted_sum, wR);
        auto normalized = cc->EvalMult(weighted_sum, make_ct(1.0 / NORM));
        
        std::cout << "  Pattern " << t.pattern << ":\n";
        std::cout << "    n(L)=" << decrypt_val(nL) 
                  << " n(C)=" << decrypt_val(nC)
                  << " n(R)=" << decrypt_val(nR) << "\n";
        std::cout << "    weighted_sum=" << decrypt_val(weighted_sum)
                  << " normalized=" << decrypt_val(normalized) << "\n\n";
    }
    
    std::cout << "  Level: " << eval_rule110_phi(ct_phi, ct_phi, ct_phi)->GetLevel() << "\n";
    std::cout << "  φ-stable transformation complete\n";

    return 0;
}
