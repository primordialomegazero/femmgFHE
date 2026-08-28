// φ SCALING — Natural Scale para sa NAND Output
// I-encode pabalik ang outputs gamit ang φ scale
// 2φ → φ, φ → φ, 0 → 0

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  φ SCALING — Natural Scale\n";
    std::cout << "  2φ → φ, φ → φ, 0 → 0\n";
    std::cout << "========================================\n\n";

    const double PHI = 1.6180339887498948482;
    const double TWO_PHI = 2 * PHI;
    const double PHI_INV = 1.0 / PHI;  // 0.618034

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

    auto ct_zero = make_ct(0.0);
    auto ct_phi = make_ct(PHI);
    auto ct_2phi = make_ct(TWO_PHI);

    // NAND = 2φ - (a+b)
    auto eval_nand = [&](auto a, auto b) {
        auto sum = cc->EvalAdd(a, b);
        return cc->EvalSub(ct_2phi, sum);
    };

    std::cout << "NAND OUTPUTS:\n";
    std::cout << "=============\n\n";

    auto nand_00 = eval_nand(ct_zero, ct_zero);
    auto nand_01 = eval_nand(ct_zero, ct_phi);
    auto nand_11 = eval_nand(ct_phi, ct_phi);

    std::cout << "  NAND(0,0) = " << decrypt_val(nand_00) << "\n";
    std::cout << "  NAND(0,1) = " << decrypt_val(nand_01) << "\n";
    std::cout << "  NAND(1,1) = " << decrypt_val(nand_11) << "\n\n";

    // Subukan ang iba't ibang scaling
    std::cout << "SCALING METHODS:\n";
    std::cout << "================\n\n";

    // Method 1: Multiply by 1/φ (scaling down)
    auto scale_down_00 = cc->EvalMult(nand_00, make_ct(PHI_INV));
    auto scale_down_01 = cc->EvalMult(nand_01, make_ct(PHI_INV));
    auto scale_down_11 = cc->EvalMult(nand_11, make_ct(PHI_INV));

    std::cout << "Method 1: Multiply by 1/φ\n";
    std::cout << "  NAND(0,0) * 1/φ = " << decrypt_val(scale_down_00) << "\n";
    std::cout << "  NAND(0,1) * 1/φ = " << decrypt_val(scale_down_01) << "\n";
    std::cout << "  NAND(1,1) * 1/φ = " << decrypt_val(scale_down_11) << "\n";
    std::cout << "  Level: " << scale_down_00->GetLevel() << "\n\n";

    // Method 2: Subtract φ (normalization)
    auto norm_00 = cc->EvalSub(nand_00, ct_phi);
    auto norm_01 = cc->EvalSub(nand_01, ct_phi);
    auto norm_11 = cc->EvalSub(nand_11, ct_phi);

    std::cout << "Method 2: Subtract φ\n";
    std::cout << "  NAND(0,0) - φ = " << decrypt_val(norm_00) << "\n";
    std::cout << "  NAND(0,1) - φ = " << decrypt_val(norm_01) << "\n";
    std::cout << "  NAND(1,1) - φ = " << decrypt_val(norm_11) << "\n";
    std::cout << "  Level: " << norm_00->GetLevel() << "\n\n";

    // Method 3: Natural φ power scaling
    // φ² = φ + 1 → φ² - 1 = φ
    // 2φ = φ + φ = φ² - 1 + φ² - 1 = 2φ² - 2
    auto ct_phi_sq = make_ct(PHI * PHI);
    auto ct_two = make_ct(2.0);
    auto scale3_00 = cc->EvalSub(cc->EvalMult(nand_00, ct_phi_sq), ct_two);
    auto scale3_01 = cc->EvalSub(cc->EvalMult(nand_01, ct_phi_sq), ct_two);
    auto scale3_11 = cc->EvalSub(cc->EvalMult(nand_11, ct_phi_sq), ct_two);

    std::cout << "Method 3: φ²*x - 2\n";
    std::cout << "  NAND(0,0) map = " << decrypt_val(scale3_00) << "\n";
    std::cout << "  NAND(0,1) map = " << decrypt_val(scale3_01) << "\n";
    std::cout << "  NAND(1,1) map = " << decrypt_val(scale3_11) << "\n";
    std::cout << "  Level: " << scale3_00->GetLevel() << "\n\n";

    // Pattern hunt
    std::cout << "PATTERN HUNT:\n";
    std::cout << "=============\n\n";
    std::cout << "  Target: map outputs para maging self-sustaining\n";
    std::cout << "  (0,0) → 2φ → dapat map to φ (1 state)\n";
    std::cout << "  (0,1) → φ → dapat map to φ (1 state)\n";
    std::cout << "  (1,1) → 0 → dapat map to 0 (0 state)\n\n";

    return 0;
}
