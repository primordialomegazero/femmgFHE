// ============================================
// RULE 110 SA φ-SELF-REFERENTIAL SPACE
// Ang φ mismo ang gumagawa ng desisyon
//
// Core insight:
// - φ² = φ + 1 — self-referential property
// - Ang output ay φ-state na galing sa sarili
// - Walang arbitrary formula — φ ang natural na nagde-decide
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
    std::cout << "  RULE 110 SA φ-SELF-REFERENTIAL SPACE\n";
    std::cout << "  φ ang Nagde-decide\n";
    std::cout << "========================================\n\n";

    const double PHI = 1.6180339887498948482;
    const double PSI = -0.6180339887498948482;
    const double PHI2 = PHI * PHI;
    const double INV_PHI = 1.0 / PHI;

    CCParams<CryptoContextCKKSRNS> params;
    params.SetMultiplicativeDepth(4);
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

    // ========== φ-SELF-REFERENTIAL RULE 110 ==========
    // Ang φ ay may natural na cycle: φ → φ² → φ³ → ...
    // φ³ = 2φ + 1 = 4.236
    // φ⁴ = 3φ + 2 = 6.854
    //
    // Ang Rule 110 ay φ-cycle sa neighborhood:
    // output = (φ²×L + φ×C + R) / φ³
    // Ito ay natural na φ-weighted average
    
    auto eval_rule110_phi = [&](auto L, auto C, auto R) {
        // φ-cycle weights
        auto wL = cc->EvalMult(L, make_ct(PHI2));
        auto wC = cc->EvalMult(C, make_ct(PHI));
        
        // φ-weighted sum
        auto sum = cc->EvalAdd(wL, wC);
        sum = cc->EvalAdd(sum, R);
        
        // φ³ normalization
        auto normalized = cc->EvalMult(sum, make_ct(1.0 / (PHI2 + PHI + 1.0)));
        
        // φ-self-reference: output = φ × normalized - ψ
        auto result = cc->EvalSub(
            cc->EvalMult(normalized, make_ct(PHI)),
            make_ct(PSI)
        );
        
        return result;
    };

    auto ct_phi = make_ct(PHI);
    auto ct_psi = make_ct(PSI);

    std::cout << "RULE 110 SA φ-SELF-REFERENTIAL SPACE:\n";
    std::cout << "=====================================\n\n";
    std::cout << "  ψ = " << PSI << " (False)\n";
    std::cout << "  φ = " << PHI << " (True)\n";
    std::cout << "  φ-weights: [" << PHI2 << ", " << PHI << ", 1]\n\n";

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
        
        // φ-natural threshold: 0 (midpoint ng φ at ψ)
        double collapsed = (val > 0) ? PHI : PSI;
        
        bool match = (collapsed == t.expected);
        if (match) correct++;
        
        std::cout << "  (" << t.pattern << ") → " << val
                  << " → " << collapsed
                  << " (expected " << t.expected << ")"
                  << (match ? " ✓" : " ✗") << "\n";
    }

    std::cout << "\n  Rule 110 sa φ-self-referential space: " << correct << "/8\n";
    
    // Show φ-cycle weights
    std::cout << "\nφ-CYCLE WEIGHTS:\n";
    std::cout << "================\n\n";
    
    for (auto& t : tests) {
        auto wL = cc->EvalMult(t.ct_l, make_ct(PHI2));
        auto wC = cc->EvalMult(t.ct_c, make_ct(PHI));
        auto sum = cc->EvalAdd(wL, wC);
        sum = cc->EvalAdd(sum, t.ct_r);
        auto normalized = cc->EvalMult(sum, make_ct(1.0 / (PHI2 + PHI + 1.0)));
        
        std::cout << "  Pattern " << t.pattern << ":\n";
        std::cout << "    wL=" << decrypt_val(wL)
                  << " wC=" << decrypt_val(wC)
                  << " R=" << decrypt_val(t.ct_r) << "\n";
        std::cout << "    normalized=" << decrypt_val(normalized) << "\n\n";
    }
    
    std::cout << "  Level: " << eval_rule110_phi(ct_phi, ct_phi, ct_phi)->GetLevel() << "\n";
    std::cout << "  φ-self-referential transformation complete\n";

    return 0;
}
