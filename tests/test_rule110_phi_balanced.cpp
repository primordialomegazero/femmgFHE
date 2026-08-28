// ============================================
// RULE 110 SA φ-BALANCED SPACE
// Logarithmic φ-resonance na hindi sumasabog
//
// Core fix:
// - φ-values ay nasa [ψ, φ] range
// - Bawat operation ay may immediate normalization
// - φ-logarithmic mapping: [ψ, φ] → [0, 1]
// - Mod 1 para sa cyclic φ-structure
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
    std::cout << "  RULE 110 SA φ-BALANCED SPACE\n";
    std::cout << "  Logarithmic φ-Resonance\n";
    std::cout << "========================================\n\n";

    const double PHI = 1.6180339887498948482;
    const double PSI = -0.6180339887498948482;
    const double PHI2 = PHI * PHI;
    const double INV_PHI = 1.0 / PHI;
    const double MIDPOINT = (PHI + PSI) / 2.0;
    const double RANGE = PHI - PSI;

    CCParams<CryptoContextCKKSRNS> params;
    params.SetMultiplicativeDepth(15);
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

    // ========== φ-NORMALIZATION ==========
    // Clamp value to [ψ, φ] range using smooth function
    auto phi_normalize = [&](auto val) {
        // Use tanh to smoothly clamp
        // normalized = MIDPOINT + RANGE/2 * tanh((val - MIDPOINT) / (RANGE/2))
        auto diff = cc->EvalSub(val, make_ct(MIDPOINT));
        auto scaled = cc->EvalMult(diff, make_ct(2.0 / RANGE));
        
        // tanh approximation: x - x^3/3 + 2x^5/15
        auto x2 = cc->EvalMult(scaled, scaled);
        auto x3 = cc->EvalMult(x2, scaled);
        auto x5 = cc->EvalMult(x3, x2);
        
        auto tanh_approx = cc->EvalSub(scaled, cc->EvalMult(x3, make_ct(1.0/3.0)));
        tanh_approx = cc->EvalAdd(tanh_approx, cc->EvalMult(x5, make_ct(2.0/15.0)));
        
        auto result = cc->EvalMult(tanh_approx, make_ct(RANGE / 2.0));
        return cc->EvalAdd(result, make_ct(MIDPOINT));
    };

    // ========== φ-LOGARITHMIC MAPPING ==========
    // Map [ψ, φ] → [0, 1]
    auto to_phi_log = [&](auto state) {
        auto diff = cc->EvalSub(state, make_ct(PSI));
        return cc->EvalMult(diff, make_ct(1.0 / RANGE));
    };

    // Map [0, 1] → [ψ, φ]
    auto from_phi_log = [&](auto log_val) {
        auto scaled = cc->EvalMult(log_val, make_ct(RANGE));
        return cc->EvalAdd(scaled, make_ct(PSI));
    };

    // ========== φ-BALANCED RULE 110 ==========
    // R(L,C,R) = φ_normalize( φ × (log(L) + log(C) + log(R)) )
    // Ang sum ng logs ay nasa [0, 3], kailangan i-normalize
    
    auto eval_rule110_phi = [&](auto L, auto C, auto R) {
        // Step 1: Convert to φ-log space [0, 1]
        auto logL = to_phi_log(L);
        auto logC = to_phi_log(C);
        auto logR = to_phi_log(R);
        
        // Step 2: Weighted sum with φ-weights
        // Past (L): φ^(-1), Present (C): φ^0, Future (R): φ^1
        auto wL = cc->EvalMult(logL, make_ct(INV_PHI));
        auto wR = cc->EvalMult(logR, make_ct(PHI));
        
        auto weighted_sum = cc->EvalAdd(wL, logC);
        weighted_sum = cc->EvalAdd(weighted_sum, wR);
        
        // Step 3: Normalize by (φ^(-1) + 1 + φ)
        auto norm_factor = make_ct(1.0 / (INV_PHI + 1.0 + PHI));
        auto normalized = cc->EvalMult(weighted_sum, norm_factor);
        
        // Step 4: Map back to φ-space
        auto result = from_phi_log(normalized);
        
        // Step 5: Apply φ-sigmoid for crisp decision
        auto diff = cc->EvalSub(result, make_ct(MIDPOINT));
        auto gain = make_ct(5.0);
        auto scaled = cc->EvalMult(diff, gain);
        
        auto x2 = cc->EvalMult(scaled, scaled);
        auto x3 = cc->EvalMult(x2, scaled);
        auto x5 = cc->EvalMult(x3, x2);
        
        auto tanh_approx = cc->EvalSub(scaled, cc->EvalMult(x3, make_ct(1.0/3.0)));
        tanh_approx = cc->EvalAdd(tanh_approx, cc->EvalMult(x5, make_ct(2.0/15.0)));
        
        auto output = cc->EvalMult(tanh_approx, make_ct(RANGE / 2.0));
        output = cc->EvalAdd(output, make_ct(MIDPOINT));
        
        return output;
    };

    auto ct_phi = make_ct(PHI);
    auto ct_psi = make_ct(PSI);

    std::cout << "RULE 110 SA φ-BALANCED SPACE:\n";
    std::cout << "==============================\n\n";
    std::cout << "  ψ = " << PSI << " (False)\n";
    std::cout << "  φ = " << PHI << " (True)\n";
    std::cout << "  φ-log mapping: [ψ, φ] → [0, 1]\n\n";

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

    std::cout << "\n  Rule 110 sa φ-balanced space: " << correct << "/8\n";
    
    // Show intermediate values
    std::cout << "\nINTERMEDIATE ANALYSIS:\n";
    std::cout << "======================\n\n";
    
    for (auto& t : tests) {
        auto logL = to_phi_log(t.ct_l);
        auto logC = to_phi_log(t.ct_c);
        auto logR = to_phi_log(t.ct_r);
        
        std::cout << "  Pattern " << t.pattern << ":\n";
        std::cout << "    log(L)=" << decrypt_val(logL) 
                  << " log(C)=" << decrypt_val(logC)
                  << " log(R)=" << decrypt_val(logR) << "\n";
    }
    
    std::cout << "\n  Level: " << eval_rule110_phi(ct_phi, ct_phi, ct_phi)->GetLevel() << "\n";
    std::cout << "  φ-balanced transformation complete\n";

    return 0;
}
