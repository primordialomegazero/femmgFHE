// φ²-DOMAIN NAND — 0-LEVEL ATTEMPT
// Bagong formula: NAND = φ² - (a+b) na may φ-correction

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  φ²-DOMAIN NAND — 0-LEVEL\n";
    std::cout << "  Bagong Formula\n";
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

    auto ct_phi_sq = make_ct(phi_sq);
    auto ct_zero = make_ct(0.0);

    // BAGONG NAND FORMULA (0-level):
    // NAND(a,b) = φ² - (a+b)
    // Na may period-2 correction para sa mga maling cases

    auto nand_0level = [&](auto a, auto b) {
        auto sum = cc->EvalAdd(a, b);       // 0-level
        return cc->EvalSub(ct_phi_sq, sum); // 0-level
    };

    std::cout << "BAGONG NAND (0-LEVEL):\n";
    std::cout << "=====================\n\n";

    // Truth table
    auto t00 = nand_0level(ct_zero, ct_zero);
    auto t0p = nand_0level(ct_zero, ct_phi_sq);
    auto tp0 = nand_0level(ct_phi_sq, ct_zero);
    auto tpp = nand_0level(ct_phi_sq, ct_phi_sq);

    std::cout << "  NAND(0,0) = " << decrypt_val(t00) << " (expected " << phi_sq << ")\n";
    std::cout << "  NAND(0,φ²) = " << decrypt_val(t0p) << " (expected " << phi_sq << ")\n";
    std::cout << "  NAND(φ²,0) = " << decrypt_val(tp0) << " (expected " << phi_sq << ")\n";
    std::cout << "  NAND(φ²,φ²) = " << decrypt_val(tpp) << " (expected 0)\n\n";

    // ANG CORRECTION:
    // NAND(0,0) = φ² ✓
    // NAND(0,φ²) = 0 ✗ → kailangan ng +φ² correction
    // NAND(φ²,0) = 0 ✗ → kailangan ng +φ² correction
    // NAND(φ²,φ²) = -φ² ✗ → kailangan ng +φ² correction

    std::cout << "CORRECTION ANALYSIS:\n";
    std::cout << "====================\n\n";
    std::cout << "  Kasalukuyan: φ² - (a+b)\n";
    std::cout << "  Kailangan: \n";
    std::cout << "    (0,0) → φ² - 0 = φ² ✓\n";
    std::cout << "    (0,φ²) → φ² - φ² = 0 ✗ (dapat φ²)\n";
    std::cout << "    (φ²,0) → φ² - φ² = 0 ✗ (dapat φ²)\n";
    std::cout << "    (φ²,φ²) → φ² - 2φ² = -φ² ✗ (dapat 0)\n\n";

    std::cout << "  ANG PATTERN NG MALI:\n";
    std::cout << "    Kapag may φ² sa inputs, may error.\n";
    std::cout << "    Kung isang φ²: error = φ²\n";
    std::cout << "    Kung dalawang φ²: error = φ²\n\n";

    std::cout << "  POSIBLENG FIX:\n";
    std::cout << "    NAND(a,b) = |φ² - (a+b)|\n";
    std::cout << "    (absolute value — nonlinear pero baka\n";
    std::cout << "     may φ-oscillator approximation)\n\n";

    std::cout << "  SUBUKAN ANG OSCILLATOR APPROXIMATION:\n";
    std::cout << "    |φ² - (a+b)| ≈ φ² - (a+b) + 2·(a·b)\n";
    std::cout << "    (pero may multiplication ulit...)\n";

    return 0;
}
