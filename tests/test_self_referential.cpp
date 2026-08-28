// SELF-REFERENTIAL φ NAND
// NAND(a,b) = φ² - (a+b)
// Self-referential: φ² = φ + 1

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  SELF-REFERENTIAL φ NAND\n";
    std::cout << "  φ² = φ + 1\n";
    std::cout << "========================================\n\n";

    const double PHI = 1.6180339887498948482;
    const double PHI_SQ = PHI * PHI;  // 2.618034
    const double PHI_INV = 1.0 / PHI; // 0.618034

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

    auto ct_phi_sq = make_ct(PHI_SQ);
    auto ct_bit0 = make_ct(0.0);
    auto ct_bit1 = make_ct(PHI_INV);

    // Self-referential NAND: φ² - (a+b)
    auto eval_nand = [&](auto a, auto b) {
        auto sum = cc->EvalAdd(a, b);
        return cc->EvalSub(ct_phi_sq, sum);
    };

    std::cout << "NAND = φ² - (a+b)\n";
    std::cout << "==================\n\n";

    struct TestCase {
        int a_bit;
        int b_bit;
        Ciphertext<DCRTPoly> ct_a;
        Ciphertext<DCRTPoly> ct_b;
    };

    std::vector<TestCase> tests = {
        {0, 0, ct_bit0, ct_bit0},
        {0, 1, ct_bit0, ct_bit1},
        {1, 0, ct_bit1, ct_bit0},
        {1, 1, ct_bit1, ct_bit1}
    };

    for (auto& t : tests) {
        auto result = eval_nand(t.ct_a, t.ct_b);
        double val = decrypt_val(result);
        std::cout << "  NAND(" << t.a_bit << "," << t.b_bit << ") = "
                  << val << "\n";
    }

    std::cout << "\nSELF-REFERENTIAL OBSERVATION:\n";
    std::cout << "==============================\n\n";
    std::cout << "  φ² = " << PHI_SQ << "\n";
    std::cout << "  φ² - 2(1/φ) = " << (PHI_SQ - 2 * PHI_INV) << "\n";
    std::cout << "  φ² - 1/φ = " << (PHI_SQ - PHI_INV) << "\n";
    std::cout << "  φ² - 0 = " << PHI_SQ << "\n\n";

    return 0;
}
