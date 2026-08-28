// SIGN VIA φ — Natural Zero Crossing
// sign(x) = (φ^x - ψ^x) / (φ^x + ψ^x)
// Para sa malaking x, lumalapit sa ±1
// Sa encrypted domain: gumamit ng φ-powers

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  SIGN VIA φ — Natural Zero Crossing\n";
    std::cout << "  Walang Polynomial Approximation\n";
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
    auto ct_phi = make_ct(PHI);
    auto ct_2phi = make_ct(2 * PHI);

    // NAND = 2φ - (a+b)
    auto eval_nand = [&](auto a, auto b) {
        auto sum = cc->EvalAdd(a, b);
        return cc->EvalSub(ct_2phi, sum);
    };

    // Subukan ang iba't ibang paraan para makuha ang sign
    std::cout << "SIGN EXTRACTION METHODS:\n";
    std::cout << "========================\n\n";

    auto nand_00 = eval_nand(ct_zero, ct_zero);
    auto nand_01 = eval_nand(ct_zero, ct_phi);
    auto nand_11 = eval_nand(ct_phi, ct_phi);

    std::cout << "Raw NAND values:\n";
    std::cout << "  NAND(0,0) = " << decrypt_val(nand_00) << "\n";
    std::cout << "  NAND(0,1) = " << decrypt_val(nand_01) << "\n";
    std::cout << "  NAND(1,1) = " << decrypt_val(nand_11) << "\n\n";

    // Method 1: x / |x| approximation
    std::cout << "Method 1: x / |x| (approximation)\n";
    std::cout << "  Hindi homomorphic — kailangan ng |x|\n\n";

    // Method 2: φ-based sign approximation
    // sign(x) ≈ tanh(kx) para sa malaking k
    // tanh(kx) = (e^{kx} - e^{-kx}) / (e^{kx} + e^{-kx})
    // Sa φ: e^{kx} ≈ φ^{kx/ln(φ)}
    std::cout << "Method 2: tanh approximation via φ\n";
    std::cout << "  tanh(kx) = (φ^{kx/ln(φ)} - φ^{-kx/ln(φ)}) / (φ^{kx/ln(φ)} + φ^{-kx/ln(φ)})\n";
    std::cout << "  Kailangan ng exponential — hindi level 0\n\n";

    // Method 3: Natural φ alternation
    // φ^n alternates sa pagitan ng paglaki at pagliit
    std::cout << "Method 3: Natural φ alternation\n";
    std::cout << "  φ^1 = 1.618 (positive)\n";
    std::cout << "  φ^2 = 2.618 (positive, mas malaki)\n";
    std::cout << "  φ^3 = 4.236 (positive, mas malaki pa)\n";
    std::cout << "  Ang φ^n ay laging positive — hindi makukuha ang sign\n\n";

    // Method 4: Polynomial approximation ng sign
    // sign(x) ≈ x * (3 - x²) / 2 para sa |x| ≤ 1
    std::cout << "Method 4: Polynomial approximation\n";
    std::cout << "  sign(x) ≈ x * (3 - x²) / 2\n";
    std::cout << "  May multiplication — level cost\n\n";

    // Subukan natin: x * (3 - x²) / 2
    auto eval_sign_poly = [&](auto x) {
        // x²
        auto x_sq = cc->EvalMult(x, x);
        // 3 - x²
        auto three_minus_x_sq = cc->EvalSub(make_ct(3.0), x_sq);
        // x * (3 - x²)
        auto x_times = cc->EvalMult(x, three_minus_x_sq);
        // / 2
        return cc->EvalMult(x_times, make_ct(0.5));
    };

    std::cout << "Polynomial sign test:\n";
    auto sign_00 = eval_sign_poly(nand_00);
    auto sign_01 = eval_sign_poly(nand_01);
    auto sign_11 = eval_sign_poly(nand_11);

    std::cout << "  sign(NAND(0,0)) = " << decrypt_val(sign_00) << "\n";
    std::cout << "  sign(NAND(0,1)) = " << decrypt_val(sign_01) << "\n";
    std::cout << "  sign(NAND(1,1)) = " << decrypt_val(sign_11) << "\n";
    std::cout << "  Level: " << sign_00->GetLevel() << "\n\n";

    // Method 5: Natural zero crossing na walang sign extraction
    // Kung ang zero crossing mismo ang boundary, baka hindi natin kailangan ng sign
    std::cout << "Method 5: Zero crossing as natural boundary\n";
    std::cout << "  Kung ang NAND(1,1) = 0, at iba ay positive,\n";
    std::cout << "  ang zero mismo ang nagsisilbing separator.\n";
    std::cout << "  Hindi natin kailangan ng sign — kailangan lang natin\n";
    std::cout << "  ng paraan para malaman kung zero o positive.\n\n";

    return 0;
}
