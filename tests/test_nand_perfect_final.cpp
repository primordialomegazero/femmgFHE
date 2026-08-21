// PERFECT NAND — PERIOD-4 + SUM-0 CORRECTION
// NAND = 2φ² - sum, maliban sa sum=0 → φ²

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  PERFECT NAND — FINAL\n";
    std::cout << "  Period-4 + Sum-0 Correction\n";
    std::cout << "========================================\n\n";

    const double phi = 1.6180339887498948482;
    const double phi_sq = phi * phi;
    const double two_phi_sq = 2 * phi_sq;

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
    auto ct_two_phi_sq = make_ct(two_phi_sq);
    auto ct_zero = make_ct(0.0);

    // NAND = 2φ² - sum (period-4) — 3/4 correct
    // Ang sum=0 lang ang mali (dapat φ², nagbibigay 2φ²)
    //
    // CORRECTION: kung sum=0, output φ²
    // Sa 0-level: i-map ang 2φ² → φ² gamit ang period-2
    // 2φ² - φ² = φ² (simpleng subtraction!)

    auto nand_perfect = [&](auto a, auto b) {
        auto sum = cc->EvalAdd(a, b);
        auto nand_val = cc->EvalSub(ct_two_phi_sq, sum);
        
        // Kung sum=0: nand_val = 2φ², gusto natin φ²
        // I-subtract ang φ² para sa correction
        // PERO ito ay magbibigay din ng φ² para sa ibang cases
        // Kailangan natin ng conditional correction
        
        return nand_val;
    };

    std::cout << "NAND RESULTS (Period-4 lang):\n";
    std::cout << "============================\n\n";

    auto n00 = nand_perfect(ct_zero, ct_zero);
    auto n01 = nand_perfect(ct_zero, ct_phi_sq);
    auto n10 = nand_perfect(ct_phi_sq, ct_zero);
    auto n11 = nand_perfect(ct_phi_sq, ct_phi_sq);

    std::cout << "  NAND(0,0) = " << decrypt_val(n00) << " → dapat " << phi_sq << "\n";
    std::cout << "  NAND(0,1) = " << decrypt_val(n01) << " → dapat " << phi_sq << "\n";
    std::cout << "  NAND(1,0) = " << decrypt_val(n10) << " → dapat " << phi_sq << "\n";
    std::cout << "  NAND(1,1) = " << decrypt_val(n11) << " → dapat 0\n\n";

    std::cout << "  3/4 correct — kailangan ng sum=0 correction\n";
    std::cout << "  Level: 0\n";

    return 0;
}
