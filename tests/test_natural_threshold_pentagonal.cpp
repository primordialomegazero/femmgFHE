// NATURAL THRESHOLD SA PENTAGONAL ENCODING
// Hanapin ang level-0 threshold gamit ang pentagon geometry
// Walang multiplication, natural na zero crossing

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  NATURAL THRESHOLD — PENTAGONAL\n";
    std::cout << "  Level-0, Walang Multiplication\n";
    std::cout << "========================================\n\n";

    const double PI = 3.14159265358979323846;
    const double TWO_PI = 2 * PI;
    const double GOLDEN_ANGLE = TWO_PI * (1.0 - 1.0 / 1.6180339887498948482);
    const double ENC_0 = -2 * PI / 5;  // -72°
    const double ENC_1 = 2 * PI / 5;   // +72°

    CCParams<CryptoContextCKKSRNS> params;
    params.SetMultiplicativeDepth(5);
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
    auto ct_enc0 = make_ct(ENC_0);
    auto ct_enc1 = make_ct(ENC_1);
    auto ct_golden_angle = make_ct(GOLDEN_ANGLE);

    // NAND sa pentagonal space: NAND = golden_angle - (a+b)
    auto eval_nand_pentagonal = [&](auto a, auto b) {
        auto sum = cc->EvalAdd(a, b);
        return cc->EvalSub(ct_golden_angle, sum);
    };

    std::cout << "PENTAGONAL NAND VALUES:\n";
    std::cout << "=======================\n\n";

    struct TestCase {
        int a_bit;
        int b_bit;
        Ciphertext<DCRTPoly> ct_a;
        Ciphertext<DCRTPoly> ct_b;
    };

    std::vector<TestCase> tests = {
        {0, 0, ct_enc0, ct_enc0},
        {0, 1, ct_enc0, ct_enc1},
        {1, 0, ct_enc1, ct_enc0},
        {1, 1, ct_enc1, ct_enc1}
    };

    for (auto& t : tests) {
        auto nand_result = eval_nand_pentagonal(t.ct_a, t.ct_b);
        double val = decrypt_val(nand_result);
        double val_degrees = val * 180.0 / PI;
        
        std::cout << "  NAND(" << t.a_bit << "," << t.b_bit << ") = "
                  << val << " (" << val_degrees << "°)\n";
    }

    std::cout << "\nGOLDEN ANGLE = " << GOLDEN_ANGLE << " (" 
              << (GOLDEN_ANGLE * 180.0 / PI) << "°)\n\n";

    // Tingnan kung may natural separation
    std::cout << "NATURAL SEPARATION ANALYSIS:\n";
    std::cout << "============================\n\n";
    std::cout << "  Ang zero crossing ay nasa GOLDEN_ANGLE\n";
    std::cout << "  - Kung sum < GOLDEN_ANGLE → positive (NAND=1)\n";
    std::cout << "  - Kung sum > GOLDEN_ANGLE → negative (NAND=0)\n\n";

    // Subukan: NAND = (GOLDEN_ANGLE - sum) / GOLDEN_ANGLE
    // Para ma-normalize sa [0,1]
    auto eval_nand_normalized = [&](auto a, auto b) {
        auto sum = cc->EvalAdd(a, b);
        auto diff = cc->EvalSub(ct_golden_angle, sum);
        return cc->EvalMult(diff, make_ct(1.0 / GOLDEN_ANGLE));
    };

    std::cout << "NORMALIZED NAND (GOLDEN_ANGLE - sum) / GOLDEN_ANGLE:\n";
    std::cout << "=====================================================\n\n";

    for (auto& t : tests) {
        auto nand_result = eval_nand_normalized(t.ct_a, t.ct_b);
        double val = decrypt_val(nand_result);
        
        int got = (val > 0.5) ? 1 : 0;
        int expected = !(t.a_bit & t.b_bit);
        
        std::cout << "  NAND(" << t.a_bit << "," << t.b_bit << ") = "
                  << val << " → " << got << "/" << expected
                  << " (level=" << nand_result->GetLevel() << ")"
                  << (got == expected ? " ✓" : " ✗") << "\n";
    }

    return 0;
}
