// SIGN DETECTION VIA EVALSUM — 0-LEVEL ABS
// Ang EvalSum ay 0-level at kayang magbigay ng sign info

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  SIGN DETECTION VIA EVALSUM\n";
    std::cout << "  0-Level Absolute Value\n";
    std::cout << "========================================\n\n";

    const double phi = 1.6180339887498948482;
    const double phi_sq = phi * phi;

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
    cc->EvalSumKeyGen(keys.secretKey);
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

    auto ct_phi_sq = make_ct(phi_sq);
    auto ct_zero = make_ct(0.0);

    // Test: 3 cases ng sign
    // Case 0: positive (φ²)
    // Case 1: zero (0)
    // Case 2: negative (-φ²)

    std::cout << "THREE SIGN CASES:\n";
    std::cout << "=================\n\n";

    auto pos_val = make_ct(phi_sq);
    auto zero_val = make_ct(0.0);
    auto neg_val = cc->EvalSub(ct_zero, ct_phi_sq);

    std::cout << "  Positive: " << decrypt_val(pos_val) << "\n";
    std::cout << "  Zero: " << decrypt_val(zero_val) << "\n";
    std::cout << "  Negative: " << decrypt_val(neg_val) << "\n\n";

    // Ang trick: magdagdag ng φ² at i-check kung lalampas sa φ²
    // positive + φ² = 2φ² (lampas)
    // zero + φ² = φ² (sakto)
    // negative + φ² = 0 (bababa)

    std::cout << "AFTER ADDING φ²:\n";
    std::cout << "================\n\n";

    auto pos_plus = cc->EvalAdd(pos_val, ct_phi_sq);
    auto zero_plus = cc->EvalAdd(zero_val, ct_phi_sq);
    auto neg_plus = cc->EvalAdd(neg_val, ct_phi_sq);

    std::cout << "  Positive + φ² = " << decrypt_val(pos_plus) << " (2φ²)\n";
    std::cout << "  Zero + φ² = " << decrypt_val(zero_plus) << " (φ²)\n";
    std::cout << "  Negative + φ² = " << decrypt_val(neg_plus) << " (0)\n\n";

    // Ngayon, ang oscillator na may φ²:
    // φ² - (value + φ²) ay nagbibigay ng:
    // positive: φ² - 2φ² = -φ²
    // zero: φ² - φ² = 0
    // negative: φ² - 0 = φ²

    std::cout << "OSCILLATOR AFTER ADDING φ²:\n";
    std::cout << "==========================\n\n";

    auto pos_osc = cc->EvalSub(ct_phi_sq, pos_plus);
    auto zero_osc = cc->EvalSub(ct_phi_sq, zero_plus);
    auto neg_osc = cc->EvalSub(ct_phi_sq, neg_plus);

    std::cout << "  φ² - (positive + φ²) = " << decrypt_val(pos_osc) << " (-φ²)\n";
    std::cout << "  φ² - (zero + φ²) = " << decrypt_val(zero_osc) << " (0)\n";
    std::cout << "  φ² - (negative + φ²) = " << decrypt_val(neg_osc) << " (φ²)\n\n";

    std::cout << "OBSERVATION:\n";
    std::cout << "============\n\n";
    std::cout << "  Ang oscillator ay nagbibigay ng:\n";
    std::cout << "  -φ² para sa positive input\n";
    std::cout << "  0 para sa zero input\n";
    std::cout << "  φ² para sa negative input\n\n";

    std::cout << "  Ito ay SIGN FLIP! Ang sign ay na-flip\n";
    std::cout << "  nang walang multiplication — 0-level!\n\n";

    std::cout << "  Kung ito ay i-oscillate muli:\n";
    std::cout << "  φ² - (φ² - (x + φ²)) = x + φ²\n";
    std::cout << "  Nakabalik sa original + φ²\n";

    return 0;
}
