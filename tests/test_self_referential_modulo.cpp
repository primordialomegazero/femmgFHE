// SELF-REFERENTIAL MODULO
// Hanapin kung ang φ ay may natural na modulo
// φ² = φ + 1 → φ² - φ - 1 = 0
// May natural na reduction ba?

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  SELF-REFERENTIAL MODULO\n";
    std::cout << "  Natural φ Reduction\n";
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

    std::cout << "φ SELF-REFERENTIAL PROPERTIES:\n";
    std::cout << "==============================\n\n";
    std::cout << "  φ² = φ + 1 = " << PHI_SQ << "\n";
    std::cout << "  φ² - φ = 1\n";
    std::cout << "  φ² - φ - 1 = 0\n\n";

    std::cout << "NATURAL REDUCTION CANDIDATES:\n";
    std::cout << "=============================\n\n";

    // Test: x - φ² * floor(x/φ²)
    // Sa φ-space: x mod φ²
    std::cout << "1. Modulo φ²:\n";
    std::cout << "   x mod φ² = x - φ² * floor(x/φ²)\n\n";

    // Test: x - φ * floor(x/φ)
    std::cout << "2. Modulo φ:\n";
    std::cout << "   x mod φ = x - φ * floor(x/φ)\n\n";

    // Test: x - floor(x) — modulo 1
    std::cout << "3. Modulo 1:\n";
    std::cout << "   x mod 1 = x - floor(x)\n\n";

    // Ang susi: may polynomial approximation ba para sa floor?
    // floor(x) ≈ x - 0.5 - sin(2πx)/(2π) para sa non-integer x
    std::cout << "FLOOR APPROXIMATION:\n";
    std::cout << "====================\n\n";
    std::cout << "  floor(x) ≈ x - 0.5 - sin(2πx)/(2π)\n";
    std::cout << "  Kailangan ng EvalSin — may level cost\n\n";

    // Subukan: ang φ self-reference ay polynomial
    // x² - x - 1 = 0 → x² = x + 1 → x = 1 + 1/x
    std::cout << "POLYNOMIAL SELF-REFERENCE:\n";
    std::cout << "==========================\n\n";
    std::cout << "  x² - x - 1 = 0\n";
    std::cout << "  Kung x = φ, ito ay zero\n";
    std::cout << "  Kung x ≠ φ, ito ay non-zero\n\n";

    // Subukan: gamitin ang x² - x - 1 bilang natural na modulo
    auto eval_poly_mod = [&](auto x) {
        // Hindi natin kaya ang x² nang walang multiplication
        // Pero subukan natin sa plaintext muna
        return x;
    };

    std::cout << "OBSERVATION:\n";
    std::cout << "============\n\n";
    std::cout << "  Ang x² - x - 1 = 0 ay polynomial equation\n";
    std::cout << "  Sa FHE, ang x² ay multiplication — level cost\n";
    std::cout << "  Pero kung level 1 lang, baka manageable\n\n";

    // Subukan: polynomial modulo na level 1
    // f(x) = x - φ² * (x² - x - 1)
    // Ito ay parang modulo φ²
    auto ct_phi_sq = make_ct(PHI_SQ);
    auto ct_phi = make_ct(PHI);
    
    auto eval_self_mod = [&](auto x) {
        // x² - x - 1
        auto x_sq = cc->EvalMult(x, x);
        auto x_sq_minus_x = cc->EvalSub(x_sq, x);
        auto poly = cc->EvalSub(x_sq_minus_x, make_ct(1.0));
        
        // x - φ² * poly
        auto phi_sq_times_poly = cc->EvalMult(ct_phi_sq, poly);
        return cc->EvalSub(x, phi_sq_times_poly);
    };

    std::cout << "SELF-REFERENTIAL MODULO TEST:\n";
    std::cout << "=============================\n\n";

    for (double test_val : {-5.0, -2.0, -1.0, 0.0, 1.0, 2.0, 5.0, 10.0}) {
        auto ct_val = make_ct(test_val);
        auto result = eval_self_mod(ct_val);
        double modded = decrypt_val(result);
        
        std::cout << "  f(" << test_val << ") = " << modded << "\n";
    }

    return 0;
}
