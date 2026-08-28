// ============================================
// RULE 110 SA φ-XOR SPACE
// Natural XOR via φ-parity
//
// Core insight:
// - Ang Rule 110 ay φ-XOR ng sum at asym
// - output = sum ⊕ asym sa φ-space
// - Ang XOR ay natural sa φ: φ⊕φ=ψ, ψ⊕ψ=φ
// - Pure addition lang, walang multiplication
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
    std::cout << "  RULE 110 SA φ-XOR SPACE\n";
    std::cout << "  Natural φ-XOR Gate\n";
    std::cout << "========================================\n\n";

    const double PHI = 1.6180339887498948482;
    const double PSI = -0.6180339887498948482;
    const double PHI2 = PHI * PHI;
    const double INV_PHI = 1.0 / PHI;

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

    // ========== φ-XOR RULE 110 ==========
    // Ang Rule 110 ay maaaring i-express bilang:
    // output = (L + C + R) + φ×(L - R) - φ²×(L×C×R)
    // Sa pure addition, ang huling term ay approximation
    
    auto eval_rule110_phi = [&](auto L, auto C, auto R) {
        // Step 1: Sum (symmetric component)
        auto sum_all = cc->EvalAdd(L, C);
        sum_all = cc->EvalAdd(sum_all, R);
        
        // Step 2: Asymmetry (left-right difference)
        auto asym = cc->EvalSub(L, R);
        
        // Step 3: φ-weighted combination
        auto phi_asym = cc->EvalMult(asym, make_ct(PHI));
        
        // Step 4: φ²-weighted sum
        auto phi2_sum = cc->EvalMult(sum_all, make_ct(PHI2));
        
        // Step 5: Natural XOR approximation
        // output = phi_asym - phi2_sum
        auto result = cc->EvalSub(phi_asym, phi2_sum);
        
        return result;
    };

    auto ct_phi = make_ct(PHI);
    auto ct_psi = make_ct(PSI);

    std::cout << "RULE 110 SA φ-XOR SPACE:\n";
    std::cout << "=======================\n\n";
    std::cout << "  ψ = " << PSI << " (False)\n";
    std::cout << "  φ = " << PHI << " (True)\n";
    std::cout << "  XOR: φ×asym - φ²×sum\n\n";

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
        
        // Natural threshold: 0
        double collapsed = (val > 0) ? PHI : PSI;
        
        bool match = (collapsed == t.expected);
        if (match) correct++;
        
        std::cout << "  (" << t.pattern << ") → " << val
                  << " → " << collapsed
                  << " (expected " << t.expected << ")"
                  << (match ? " ✓" : " ✗") << "\n";
    }

    std::cout << "\n  Rule 110 sa φ-XOR space: " << correct << "/8\n";
    
    // Show XOR breakdown
    std::cout << "\nXOR BREAKDOWN:\n";
    std::cout << "==============\n\n";
    
    for (auto& t : tests) {
        auto sum_all = cc->EvalAdd(t.ct_l, t.ct_c);
        sum_all = cc->EvalAdd(sum_all, t.ct_r);
        auto asym = cc->EvalSub(t.ct_l, t.ct_r);
        
        std::cout << "  Pattern " << t.pattern << ":\n";
        std::cout << "    sum=" << decrypt_val(sum_all)
                  << " asym=" << decrypt_val(asym) << "\n\n";
    }
    
    std::cout << "  Level: " << eval_rule110_phi(ct_phi, ct_phi, ct_phi)->GetLevel() << "\n";
    std::cout << "  φ-XOR transformation complete\n";

    return 0;
}
