// ============================================
// RULE 110 SA φ-COMPLEX SPACE
// Natural collapse via complex φ-roots
//
// Core insight:
// - φ at ψ ay complex conjugates
// - φ = e^(iπ/2), ψ = e^(-iπ/2)
// - Sa complex plane, addition ay natural na nagco-collapse
// - Walang arbitrary threshold — geometry ang nagde-decide
//
// Author: Dan Fernandez / Primordial Omega Zero
// ============================================

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <cmath>
#include <complex>
#include <algorithm>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  RULE 110 SA φ-COMPLEX SPACE\n";
    std::cout << "  Natural Complex φ-Collapse\n";
    std::cout << "========================================\n\n";

    // φ bilang complex: e^(iπ/2) = i
    // ψ bilang complex: e^(-iπ/2) = -i
    const std::complex<double> PHI_COMPLEX(0.0, 1.0);
    const std::complex<double> PSI_COMPLEX(0.0, -1.0);
    const double MIDPOINT_REAL = 0.0;

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

    auto make_ct = [&](std::complex<double> val) {
        std::vector<std::complex<double>> vec(slots, {0.0, 0.0});
        vec[0] = val;
        return cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(vec));
    };

    auto decrypt_val = [&](auto ct) {
        Plaintext pt;
        cc->Decrypt(keys.secretKey, ct, &pt);
        return pt->GetCKKSPackedValue()[0];
    };

    // ========== φ-COMPLEX ADDITION ==========
    // Sa complex plane, ang addition ng i at -i ay:
    // i + i = 2i (amplify paitaas)
    // -i + -i = -2i (amplify pababa)
    // i + -i = 0 (cancellation)
    // 
    // Ang natural collapse: kunin ang imaginary part sign
    
    auto eval_rule110_phi = [&](auto L, auto C, auto R) {
        // Position-weighted complex addition
        // wL = L × φ^(-2) = L × (-1) = -L
        auto wL = cc->EvalSub(make_ct(std::complex<double>(0,0)), L);
        
        // wR = R × φ^(2) = R × (-1) = -R
        auto wR = cc->EvalSub(make_ct(std::complex<double>(0,0)), R);
        
        // Complex sum na may natural collapse
        auto sum1 = cc->EvalAdd(wL, C);
        auto sum2 = cc->EvalAdd(sum1, wR);
        
        // φ-natural collapse: i×sum2 (rotation by 90°)
        // Ito ay nagma-map sa imaginary axis para sa sign detection
        auto collapsed = cc->EvalMult(sum2, make_ct(std::complex<double>(0,1)));
        
        return collapsed;
    };

    auto ct_phi = make_ct(PHI_COMPLEX);
    auto ct_psi = make_ct(PSI_COMPLEX);

    std::cout << "RULE 110 SA φ-COMPLEX SPACE:\n";
    std::cout << "============================\n\n";
    std::cout << "  ψ = " << PSI_COMPLEX << " (False)\n";
    std::cout << "  φ = " << PHI_COMPLEX << " (True)\n";
    std::cout << "  Natural collapse via complex geometry\n\n";

    std::cout << "TRUTH TABLE:\n";
    std::cout << "============\n\n";

    struct TestCase {
        std::string pattern;
        Ciphertext<DCRTPoly> ct_l, ct_c, ct_r;
        std::complex<double> expected;
    };

    std::vector<TestCase> tests = {
        {"000", ct_psi, ct_psi, ct_psi, PSI_COMPLEX},
        {"001", ct_psi, ct_psi, ct_phi, PHI_COMPLEX},
        {"010", ct_psi, ct_phi, ct_psi, PHI_COMPLEX},
        {"011", ct_psi, ct_phi, ct_phi, PHI_COMPLEX},
        {"100", ct_phi, ct_psi, ct_psi, PSI_COMPLEX},
        {"101", ct_phi, ct_psi, ct_phi, PHI_COMPLEX},
        {"110", ct_phi, ct_phi, ct_psi, PHI_COMPLEX},
        {"111", ct_phi, ct_phi, ct_phi, PSI_COMPLEX}
    };

    int correct = 0;
    for (auto& t : tests) {
        auto result = eval_rule110_phi(t.ct_l, t.ct_c, t.ct_r);
        auto val = decrypt_val(result);
        
        // Collapse based sa imaginary part
        auto collapsed = (val.imag() > MIDPOINT_REAL) ? PHI_COMPLEX : PSI_COMPLEX;
        
        bool match = (collapsed == t.expected);
        if (match) correct++;
        
        std::cout << "  (" << t.pattern << ") → " << val
                  << " → " << collapsed
                  << " (expected " << t.expected << ")"
                  << (match ? " ✓" : " ✗") << "\n";
    }

    std::cout << "\n  Rule 110 sa φ-complex space: " << correct << "/8\n";
    
    // Show complex collapse dynamics
    std::cout << "\nCOMPLEX COLLAPSE DYNAMICS:\n";
    std::cout << "==========================\n\n";
    
    for (auto& t : tests) {
        auto wL = cc->EvalSub(make_ct(std::complex<double>(0,0)), t.ct_l);
        auto wR = cc->EvalSub(make_ct(std::complex<double>(0,0)), t.ct_r);
        
        auto sum1 = cc->EvalAdd(wL, t.ct_c);
        auto sum2 = cc->EvalAdd(sum1, wR);
        
        std::cout << "  Pattern " << t.pattern << ":\n";
        std::cout << "    wL=" << decrypt_val(wL)
                  << " C=" << decrypt_val(t.ct_c)
                  << " wR=" << decrypt_val(wR) << "\n";
        std::cout << "    sum2=" << decrypt_val(sum2) << "\n\n";
    }
    
    std::cout << "  Level: " << eval_rule110_phi(ct_phi, ct_phi, ct_phi)->GetLevel() << "\n";
    std::cout << "  φ-complex transformation complete\n";

    return 0;
}
