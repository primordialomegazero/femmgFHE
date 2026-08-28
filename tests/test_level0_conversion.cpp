// LEVEL 0 CONVERSION — Walang Multiplication
// Hanapin ang additive na paraan para sa φ ↔ integer conversion
// φ² = φ + 1, 1/φ = φ - 1

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  LEVEL 0 CONVERSION\n";
    std::cout << "  Additive φ ↔ Integer\n";
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

    auto ct_zero = make_ct(0.0);
    auto ct_one = make_ct(1.0);
    auto ct_phi = make_ct(PHI);
    auto ct_phi_sq = make_ct(PHI_SQ);
    auto ct_phi_inv = make_ct(PHI_INV);

    // Key observation:
    // 1/φ = φ - 1  (additive!)
    // φ = 1 + 1/φ  (additive!)
    // φ² = φ + 1  (additive!)

    std::cout << "ADDITIVE IDENTITIES:\n";
    std::cout << "====================\n\n";
    std::cout << "  1/φ = φ - 1 = " << (PHI - 1.0) << " (actual " << PHI_INV << ")\n";
    std::cout << "  φ = 1 + 1/φ = " << (1.0 + PHI_INV) << " (actual " << PHI << ")\n";
    std::cout << "  φ² = φ + 1 = " << (PHI + 1.0) << " (actual " << PHI_SQ << ")\n\n";

    // Subukan: Convert φ to integer gamit ang subtraction
    auto ct_phi_minus_one = cc->EvalSub(ct_phi, ct_one);
    std::cout << "φ - 1 = " << decrypt_val(ct_phi_minus_one) << "\n";
    std::cout << "Level: " << ct_phi_minus_one->GetLevel() << "\n\n";

    // Subukan: Convert integer to φ gamit ang addition
    auto ct_one_plus_phi_inv = cc->EvalAdd(ct_one, ct_phi_inv);
    std::cout << "1 + 1/φ = " << decrypt_val(ct_one_plus_phi_inv) << "\n";
    std::cout << "Level: " << ct_one_plus_phi_inv->GetLevel() << "\n\n";

    // NAND sa integer space gamit ang additive conversion
    // Input: φ-space (0 o φ)
    // Step 1: Convert φ → 1 gamit ang subtraction (φ - 1)
    //   Kung input ay φ: φ - 1 = 0.618 = 1/φ (mali, dapat 1)
    //   Kung input ay 0: 0 - 1 = -1 (mali, dapat 0)
    // 
    // HINDI GUMAGANA ANG SIMPLENG SUBTRACTION
    // Kailangan ng conditional

    std::cout << "PROBLEMA:\n";
    std::cout << "=========\n\n";
    std::cout << "  Ang φ → integer conversion ay conditional:\n";
    std::cout << "  - Kung input = 0, output = 0\n";
    std::cout << "  - Kung input = φ, output = 1\n\n";
    std::cout << "  Ang subtraction φ - 1 = 0.618 (hindi 1)\n";
    std::cout << "  Ang subtraction 0 - 1 = -1 (hindi 0)\n\n";

    // Subukan: Direct encoding sa integer space
    std::cout << "ALTERNATIVE: DIRECT INTEGER ENCODING\n";
    std::cout << "====================================\n\n";
    std::cout << "  Sa halip na φ-space, i-encode natin nang direkta\n";
    std::cout << "  sa integer space mula simula:\n";
    std::cout << "  0 → 0\n";
    std::cout << "  1 → 1\n\n";
    std::cout << "  NAND = 2 - (a+b) ay addition/subtraction lang!\n";
    std::cout << "  Walang conversion, walang multiplication!\n\n";

    // Test: Direct integer encoding
    auto ct_0_int = make_ct(0.0);
    auto ct_1_int = make_ct(1.0);
    auto ct_2_int = make_ct(2.0);

    auto eval_nand_direct = [&](auto a, auto b) {
        auto sum = cc->EvalAdd(a, b);
        return cc->EvalSub(ct_2_int, sum);
    };

    std::cout << "DIRECT INTEGER NAND:\n";
    std::cout << "===================\n\n";

    struct TestCase {
        int a_bit;
        int b_bit;
        Ciphertext<DCRTPoly> ct_a;
        Ciphertext<DCRTPoly> ct_b;
    };

    std::vector<TestCase> tests = {
        {0, 0, ct_0_int, ct_0_int},
        {0, 1, ct_0_int, ct_1_int},
        {1, 0, ct_1_int, ct_0_int},
        {1, 1, ct_1_int, ct_1_int}
    };

    for (auto& t : tests) {
        auto result = eval_nand_direct(t.ct_a, t.ct_b);
        double val = decrypt_val(result);
        int got = (val > 0.5) ? 1 : 0;
        int expected = !(t.a_bit & t.b_bit);

        std::cout << "  NAND(" << t.a_bit << "," << t.b_bit << ") = "
                  << expected << " → " << got
                  << " (value=" << val
                  << ", level=" << result->GetLevel() << ")"
                  << (got == expected ? " ✓" : " ✗") << "\n";
    }

    return 0;
}
