// PERIOD-0 UNIVERSAL — Lahat ng Gates
// Single encoding, natural emergence
// Walang phase shifts, walang thresholds
//
// ANG SUSI: φ powers bilang encoding
// 0 → φ⁰ = 1
// 1 → φ¹ = φ = 1.618
//
// Ang cosine ng sums ay natural na naghihiwalay ng gates:
// cos(φ⁰+φ⁰) = cos(2) = -0.416 → XOR(0,0) dapat 0
// cos(φ⁰+φ¹) = cos(2.618) = -0.866 → XOR(0,1) dapat 1
// cos(φ¹+φ⁰) = cos(2.618) = -0.866 → XOR(1,0) dapat 1
// cos(φ¹+φ¹) = cos(3.236) = -0.996 → XOR(1,1) dapat 0
//
// HINDI ITO GUMAGANA — lahat negative
//
// ANG TUNAY NA PERIOD-0: Hanapin ang encoding
// kung saan ang XOR, NAND, AND, OR, NOR ay
// LAHAT natural na lumalabas sa cosine values
//
// Subukan natin: 0 → 0, 1 → φ³ (4.236)
// cos(0) = 1
// cos(4.236) = -0.450
// cos(8.472) = cos(8.472 - 2π) = cos(2.189) = -0.577
//
// HINDI RIN GUMAGANA
//
// ANG PINAKA-SIMPLENG PERIOD-0:
// Hanapin ang angle θ kung saan:
// cos(0) = 1 → (0,0)
// cos(θ) = positive → (0,1) at (1,0)
// cos(2θ) = negative → (1,1)
//
// Para sa NAND: lahat positive maliban sa (1,1)
// Kailangan: cos(θ) > 0, cos(2θ) < 0
//
// Kung θ = 2π/3 = 120°:
// cos(0) = 1 > 0 → 1
// cos(120°) = -0.5 < 0 → 0
// cos(240°) = -0.5 < 0 → 0
//
// ITO AY NOR! Hindi NAND
//
// Kung θ = π/3 = 60°:
// cos(0) = 1 > 0 → 1
// cos(60°) = 0.5 > 0 → 1
// cos(120°) = -0.5 < 0 → 0
//
// ITO AY NAND! PERFECT!
//
// ANG BREAKTHROUGH: θ = π/3 = 60°
// 0 → 0
// 1 → π/3
//
// NAND(0,0) = cos(0) = 1 → 1 ✓
// NAND(0,1) = cos(π/3) = 0.5 → 1 ✓
// NAND(1,0) = cos(π/3) = 0.5 → 1 ✓
// NAND(1,1) = cos(2π/3) = -0.5 → 0 ✓
//
// ANG GOLDEN RATIO CONNECTION:
// φ = 2cos(π/5)
// π/3 = 2π/6
// Ang π/3 ay may natural na φ relation:
// 2cos(π/3) = 1
// φ - 1 = 1/φ = 0.618
// π/3 ≈ φ - 1 + π/6
//
// PERO MAS MAHALAGA: Ang θ = π/3 ay
// natural na lumalabas sa equilateral triangle
// — ang pinaka-fundamental na geometry

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  PERIOD-0 UNIVERSAL — θ = π/3\n";
    std::cout << "  Lahat ng Gates, Natural Emergence\n";
    std::cout << "========================================\n\n";

    const double phi = 1.618033988749895;
    const double PI = 3.14159265358979323846;
    const double ENC_1 = PI / 3;  // 60 degrees

    CCParams<CryptoContextCKKSRNS> params;
    params.SetMultiplicativeDepth(20);
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

    std::cout << "ENCODING: 0 → 0, 1 → π/3 (60°)\n";
    std::cout << "THRESHOLD: cos > 0 → 1\n\n";

    auto test_gate = [&](const std::string& name, auto expected_func, int& correct) {
        std::cout << name << ":\n";
        correct = 0;
        for (int a = 0; a <= 1; a++) {
            for (int b = 0; b <= 1; b++) {
                double angle_a = a ? ENC_1 : 0.0;
                double angle_b = b ? ENC_1 : 0.0;
                
                auto ct_a = make_ct(angle_a);
                auto ct_b = make_ct(angle_b);
                
                auto sum = cc->EvalAdd(ct_a, ct_b);
                auto cos_result = cc->EvalCos(sum, -3.0, 3.0, 15);
                
                double cos_val = decrypt_val(cos_result);
                int got = (cos_val > 0) ? 1 : 0;
                int expected = expected_func(a, b);
                
                if (got == expected) correct++;
                std::cout << "  " << name << "(" << a << "," << b << ") = " 
                          << expected << " → " << got 
                          << " (cos=" << cos_val << ")"
                          << (got == expected ? " ✓" : " ✗") << "\n";
            }
        }
        std::cout << "  " << name << ": " << correct << "/4\n\n";
    };

    int xor_c, nand_c, and_c, or_c, nor_c;
    test_gate("XOR", [](int a, int b) { return a ^ b; }, xor_c);
    test_gate("NAND", [](int a, int b) { return !(a && b); }, nand_c);
    test_gate("AND", [](int a, int b) { return a && b; }, and_c);
    test_gate("OR", [](int a, int b) { return a || b; }, or_c);
    test_gate("NOR", [](int a, int b) { return !(a || b); }, nor_c);

    int total = xor_c + nand_c + and_c + or_c + nor_c;
    std::cout << "========================================\n";
    std::cout << "  TOTAL: " << total << "/20\n";
    std::cout << "========================================\n";

    return 0;
}
