// RULE 110 SA φ-SPACE — Hindi Binary
// I-encode ang states bilang φ-powers at fractions
// Ang multiplication ay natural na modulo sa φ

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  RULE 110 SA φ-SPACE\n";
    std::cout << "  Hindi Binary, Natural na φ\n";
    std::cout << "========================================\n\n";

    const double PHI = 1.6180339887498948482;
    const double PSI = -0.6180339887498948482;

    CCParams<CryptoContextCKKSRNS> params;
    params.SetMultiplicativeDepth(20);
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

    auto ct_phi = make_ct(PHI);
    auto ct_psi = make_ct(PSI);

    auto eval_rule110_phi = [&](auto L, auto C, auto R) {
        auto CR = cc->EvalMult(C, R);
        auto LCR = cc->EvalMult(L, CR);
        auto sum1 = cc->EvalAdd(C, R);
        auto sum2 = cc->EvalSub(sum1, CR);
        return cc->EvalSub(sum2, LCR);
    };

    std::cout << "RULE 110 SA φ-SPACE:\n";
    std::cout << "====================\n\n";
    std::cout << "  ψ = " << PSI << " (False)\n";
    std::cout << "  φ = " << PHI << " (True)\n\n";

    std::cout << "TRUTH TABLE:\n";
    std::cout << "============\n\n";

    struct TestCase {
        int l, c, r;
        Ciphertext<DCRTPoly> ct_l, ct_c, ct_r;
    };

    std::vector<TestCase> tests = {
        {0, 0, 0, ct_psi, ct_psi, ct_psi},
        {0, 0, 1, ct_psi, ct_psi, ct_phi},
        {0, 1, 0, ct_psi, ct_phi, ct_psi},
        {0, 1, 1, ct_psi, ct_phi, ct_phi},
        {1, 0, 0, ct_phi, ct_psi, ct_psi},
        {1, 0, 1, ct_phi, ct_psi, ct_phi},
        {1, 1, 0, ct_phi, ct_phi, ct_psi},
        {1, 1, 1, ct_phi, ct_phi, ct_phi}
    };

    int correct = 0;
    for (auto& t : tests) {
        auto result = eval_rule110_phi(t.ct_l, t.ct_c, t.ct_r);
        double val = decrypt_val(result);
        
        int expected;
        switch (t.l * 4 + t.c * 2 + t.r) {
            case 0: expected = 0; break;
            case 1: expected = 1; break;
            case 2: expected = 1; break;
            case 3: expected = 1; break;
            case 4: expected = 0; break;
            case 5: expected = 1; break;
            case 6: expected = 1; break;
            case 7: expected = 0; break;
        }
        
        double expected_val = (expected == 0) ? PSI : PHI;
        bool match = std::abs(val - expected_val) < 0.5;
        if (match) correct++;
        
        std::cout << "  (" << t.l << t.c << t.r << ") → " << val
                  << " (expected " << expected_val << ")"
                  << (match ? " ✓" : " ✗") << "\n";
    }

    std::cout << "\n  Rule 110 sa φ-space: " << correct << "/8\n";
    std::cout << "  Level: " << eval_rule110_phi(ct_phi, ct_phi, ct_phi)->GetLevel() << "\n";

    return 0;
}
