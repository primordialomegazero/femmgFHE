// ============================================
// RULE 110 SA φ-REALITY SPACE
// Natural reality separation ng 0 at 1
//
// Core insight:
// - Hindi natin pinipilit ang collapse
// - Ang φ-space ay may natural quadrants
// - Ang 0 at 1 ay nasa magkaibang reality
// - I-amplify lang ang signal para natural na maghiwalay
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
    std::cout << "  RULE 110 SA φ-REALITY SPACE\n";
    std::cout << "  Natural 0/1 Reality Separation\n";
    std::cout << "========================================\n\n";

    const double PHI = 1.6180339887498948482;
    const double PSI = -0.6180339887498948482;
    const double MIDPOINT = (PHI + PSI) / 2.0;
    const double RANGE = PHI - PSI;
    const double PHI2 = PHI * PHI;
    const double INV_PHI2 = 1.0 / PHI2;
    const double INV_PHI = 1.0 / PHI;

    CCParams<CryptoContextCKKSRNS> params;
    params.SetMultiplicativeDepth(3);
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

    // ========== φ-REALITY SEPARATION ==========
    // Ang 0 at 1 ay dapat nasa magkaibang reality
    // Reality 0: negative values → ψ
    // Reality 1: positive values → φ
    // Ang separation ay natural sa φ-space
    
    auto eval_rule110_phi = [&](auto L, auto C, auto R) {
        // Step 1: Compute φ-symmetric at φ-asymmetric components
        // Symmetric: L + C + R (position-blind)
        auto sum_all = cc->EvalAdd(L, C);
        sum_all = cc->EvalAdd(sum_all, R);
        
        // Asymmetric: L - R (left-right difference)
        auto asym = cc->EvalSub(L, R);
        
        // Step 2: φ-reality projection
        // output = φ×sum_all + φ²×asym
        // Ito ay naghihiwalay sa 0 at 1 realities
        auto phi_sum = cc->EvalMult(sum_all, make_ct(PHI));
        auto phi2_asym = cc->EvalMult(asym, make_ct(PHI2));
        
        auto result = cc->EvalAdd(phi_sum, phi2_asym);
        
        // Step 3: Natural amplification
        // I-square para lumayo sa 0
        auto amplified = cc->EvalMult(result, result);
        
        // Step 4: φ-sign restoration
        // Kunin ang sign ng original result
        auto sign_restored = cc->EvalMult(amplified, result);
        
        return sign_restored;
    };

    auto ct_phi = make_ct(PHI);
    auto ct_psi = make_ct(PSI);

    std::cout << "RULE 110 SA φ-REALITY SPACE:\n";
    std::cout << "============================\n\n";
    std::cout << "  ψ = " << PSI << " (Reality 0)\n";
    std::cout << "  φ = " << PHI << " (Reality 1)\n";
    std::cout << "  Natural separation via φ-projection\n\n";

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
        
        // Natural reality: negative → ψ, positive → φ
        double collapsed = (val > 0) ? PHI : PSI;
        
        bool match = (collapsed == t.expected);
        if (match) correct++;
        
        std::cout << "  (" << t.pattern << ") → " << val
                  << " → " << collapsed
                  << " (expected " << t.expected << ")"
                  << (match ? " ✓" : " ✗") << "\n";
    }

    std::cout << "\n  Rule 110 sa φ-reality space: " << correct << "/8\n";
    
    // Show reality separation
    std::cout << "\nREALITY SEPARATION:\n";
    std::cout << "===================\n\n";
    
    for (auto& t : tests) {
        auto sum_all = cc->EvalAdd(t.ct_l, t.ct_c);
        sum_all = cc->EvalAdd(sum_all, t.ct_r);
        auto asym = cc->EvalSub(t.ct_l, t.ct_r);
        
        auto phi_sum = cc->EvalMult(sum_all, make_ct(PHI));
        auto phi2_asym = cc->EvalMult(asym, make_ct(PHI2));
        auto result = cc->EvalAdd(phi_sum, phi2_asym);
        
        std::cout << "  Pattern " << t.pattern << ":\n";
        std::cout << "    sum_all=" << decrypt_val(sum_all)
                  << " asym=" << decrypt_val(asym) << "\n";
        std::cout << "    φ×sum=" << decrypt_val(phi_sum)
                  << " φ²×asym=" << decrypt_val(phi2_asym) << "\n";
        std::cout << "    result=" << decrypt_val(result) << "\n\n";
    }
    
    std::cout << "  Level: " << eval_rule110_phi(ct_phi, ct_phi, ct_phi)->GetLevel() << "\n";
    std::cout << "  φ-reality separation complete\n";

    return 0;
}
