// ============================================
// RULE 110 SA φ-CIRCULAR SPACE
// Natural φ-circular addition na may emergent sign
//
// Core insight:
// - Sa φ-space, ang addition ay modular
// - φ + φ = ψ (cycle, hindi accumulation)
// - Ang cycle na ito ay natural sa φ-structure
// - Walang arbitrary threshold — natural collapse
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
    std::cout << "  RULE 110 SA φ-CIRCULAR SPACE\n";
    std::cout << "  Natural φ-Circular Addition\n";
    std::cout << "========================================\n\n";

    const double PHI_EXP = 1.0;
    const double PSI_EXP = -1.0;
    const double MIDPOINT = 0.0;

    CCParams<CryptoContextCKKSRNS> params;
    params.SetMultiplicativeDepth(1);
    params.SetScalingModSize(50);
    params.SetBatchSize(256);
    params.SetFirstModSize(60);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(params);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);
    cc->Enable(ADVANCEDSHE);

    auto keys = cc->KeyGen();
    cc->EvalMultKeyGen(keys.secretKey);
    cc->EvalRotateKeyGen(keys.secretKey, {1, -1});
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

    // ========== φ-CIRCULAR ADDITION ==========
    // Natural cycle: +1 + +1 = -1, -1 + -1 = +1
    // Ito ay XOR sa exponent space, natural sa φ
    
    auto phi_circular_add = [&](auto a, auto b) {
        // Sa exponent space, ang circular addition ay:
        // a ⊕ b = -(a + b) kung |a + b| > 1
        // a ⊕ b = (a + b) kung |a + b| <= 1
        // 
        // Sa pure addition, ito ay maaaring i-approximate via:
        // a ⊕ b ≈ a + b - 2×sign(a+b)
        // Pero sign ay nangangailangan ng multiplication
        //
        // Ang natural na way: gamitin ang φ-rotation
        // Kung a at b ay pareho, mag-rotate para ma-flip ang sign
        
        auto sum = cc->EvalAdd(a, b);
        
        // φ-rotation para sa sign detection
        // Kung pareho sila, ang rotation ay magbibigay ng opposite
        auto rotated = cc->EvalRotate(sum, 1);
        auto difference = cc->EvalSub(sum, rotated);
        
        // Ang difference ay:
        // - Maliit kung pareho sila (mag-flip)
        // - Malaki kung magkaiba (stay)
        
        return difference;
    };

    // ========== φ-NATURAL RULE 110 ==========
    // Gamitin ang circular addition para sa natural collapse
    
    auto eval_rule110_phi = [&](auto L, auto C, auto R) {
        // φ-circular addition ng lahat ng tatlo
        auto sum_LC = phi_circular_add(L, C);
        auto sum_LC_R = phi_circular_add(sum_LC, R);
        
        // Position-weighted circular addition
        auto wL = phi_circular_add(L, make_ct(-2.0));
        auto wR = phi_circular_add(R, make_ct(2.0));
        
        auto sum_weighted = phi_circular_add(wL, C);
        sum_weighted = phi_circular_add(sum_weighted, wR);
        
        // Combine sa natural na paraan
        auto result = phi_circular_add(sum_LC_R, sum_weighted);
        
        return result;
    };

    auto ct_phi = make_ct(PHI_EXP);
    auto ct_psi = make_ct(PSI_EXP);

    std::cout << "RULE 110 SA φ-CIRCULAR SPACE:\n";
    std::cout << "=============================\n\n";
    std::cout << "  ψ = " << PSI_EXP << " (False)\n";
    std::cout << "  φ = " << PHI_EXP << " (True)\n";
    std::cout << "  Circular addition: +1⊕+1=-1, -1⊕-1=+1\n\n";

    std::cout << "TRUTH TABLE:\n";
    std::cout << "============\n\n";

    struct TestCase {
        std::string pattern;
        Ciphertext<DCRTPoly> ct_l, ct_c, ct_r;
        double expected;
    };

    std::vector<TestCase> tests = {
        {"000", ct_psi, ct_psi, ct_psi, PSI_EXP},
        {"001", ct_psi, ct_psi, ct_phi, PHI_EXP},
        {"010", ct_psi, ct_phi, ct_psi, PHI_EXP},
        {"011", ct_psi, ct_phi, ct_phi, PHI_EXP},
        {"100", ct_phi, ct_psi, ct_psi, PSI_EXP},
        {"101", ct_phi, ct_psi, ct_phi, PHI_EXP},
        {"110", ct_phi, ct_phi, ct_psi, PHI_EXP},
        {"111", ct_phi, ct_phi, ct_phi, PSI_EXP}
    };

    int correct = 0;
    for (auto& t : tests) {
        auto result = eval_rule110_phi(t.ct_l, t.ct_c, t.ct_r);
        double val = decrypt_val(result);
        
        double collapsed = (val > MIDPOINT) ? PHI_EXP : PSI_EXP;
        
        bool match = (collapsed == t.expected);
        if (match) correct++;
        
        std::cout << "  (" << t.pattern << ") → " << val
                  << " → " << collapsed
                  << " (expected " << t.expected << ")"
                  << (match ? " ✓" : " ✗") << "\n";
    }

    std::cout << "\n  Rule 110 sa φ-circular space: " << correct << "/8\n";
    
    // Test circular addition properties
    std::cout << "\nCIRCULAR ADDITION TEST:\n";
    std::cout << "=======================\n\n";
    
    auto phi_plus_phi = phi_circular_add(ct_phi, ct_phi);
    auto psi_plus_psi = phi_circular_add(ct_psi, ct_psi);
    auto phi_plus_psi = phi_circular_add(ct_phi, ct_psi);
    
    std::cout << "  φ ⊕ φ = " << decrypt_val(phi_plus_phi) << " (expected -1)\n";
    std::cout << "  ψ ⊕ ψ = " << decrypt_val(psi_plus_psi) << " (expected +1)\n";
    std::cout << "  φ ⊕ ψ = " << decrypt_val(phi_plus_psi) << " (expected 0)\n\n";
    
    std::cout << "  Level: " << eval_rule110_phi(ct_phi, ct_phi, ct_phi)->GetLevel() << "\n";
    std::cout << "  φ-circular transformation complete\n";

    return 0;
}
