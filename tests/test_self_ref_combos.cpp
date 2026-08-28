// SELF-REFERENTIAL COMBINATIONS
// Hanapin ang natural na zero crossing
// φ² - φ, φ² - 1, φ² - φ + 1/φ, atbp.

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  SELF-REFERENTIAL COMBINATIONS\n";
    std::cout << "  Natural Zero Crossing Hunt\n";
    std::cout << "========================================\n\n";

    const double PHI = 1.6180339887498948482;
    const double PHI_SQ = PHI * PHI;
    const double PHI_INV = 1.0 / PHI;

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

    auto ct_0 = make_ct(0.0);
    auto ct_1 = make_ct(PHI_INV);

    struct Formula {
        std::string name;
        double constant;
    };

    std::vector<Formula> formulas = {
        {"φ² - (a+b)", PHI_SQ},
        {"φ² - (a+b) - 1/φ", PHI_SQ - PHI_INV},
        {"φ² - (a+b) - 2/φ", PHI_SQ - 2 * PHI_INV},
        {"φ² - (a+b) - φ", PHI_SQ - PHI},
        {"φ² - (a+b) - 1", PHI_SQ - 1.0},
        {"φ² - (a+b) - φ²/2", PHI_SQ - PHI_SQ / 2},
        {"φ² - (a+b) - 1.5/φ", PHI_SQ - 1.5 * PHI_INV}
    };

    for (auto& formula : formulas) {
        std::cout << "FORMULA: " << formula.name << "\n";
        std::cout << std::string(40, '-') << "\n";

        auto ct_const = make_ct(formula.constant);

        auto eval_nand = [&](auto a, auto b) {
            auto sum = cc->EvalAdd(a, b);
            return cc->EvalSub(ct_const, sum);
        };

        struct TestCase {
            int a_bit;
            int b_bit;
            Ciphertext<DCRTPoly> ct_a;
            Ciphertext<DCRTPoly> ct_b;
        };

        std::vector<TestCase> tests = {
            {0, 0, ct_0, ct_0},
            {0, 1, ct_0, ct_1},
            {1, 0, ct_1, ct_0},
            {1, 1, ct_1, ct_1}
        };

        for (auto& t : tests) {
            auto result = eval_nand(t.ct_a, t.ct_b);
            double val = decrypt_val(result);
            int got = (val > 0) ? 1 : 0;
            int expected = !(t.a_bit & t.b_bit);

            std::cout << "  NAND(" << t.a_bit << "," << t.b_bit << ") = "
                      << val << " → " << got << "/" << expected
                      << (got == expected ? " ✓" : " ✗") << "\n";
        }
        std::cout << "\n";
    }

    return 0;
}
