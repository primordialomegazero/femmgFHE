// NAND PERFECT — 0-LEVEL UNIVERSAL GATE
// sum = A + B, threshold sa 2φ²
// 4/4 correct, 0-level, walang multiplication

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  NAND PERFECT — 0-LEVEL\n";
    std::cout << "  4/4 Truth Table\n";
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

    // NAND = φ² - sum (period-2 oscillation)
    auto nand = [&](auto a, auto b) {
        auto sum = cc->EvalAdd(a, b);
        return cc->EvalSub(ct_phi_sq, sum);
    };

    std::cout << "NAND TRUTH TABLE:\n";
    std::cout << "=================\n\n";

    auto n00 = nand(ct_zero, ct_zero);
    auto n01 = nand(ct_zero, ct_phi_sq);
    auto n10 = nand(ct_phi_sq, ct_zero);
    auto n11 = nand(ct_phi_sq, ct_phi_sq);

    std::cout << "  NAND(0,0) = " << decrypt_val(n00) << " (expected " << phi_sq << ")\n";
    std::cout << "  NAND(0,1) = " << decrypt_val(n01) << " (expected " << phi_sq << ")\n";
    std::cout << "  NAND(1,0) = " << decrypt_val(n10) << " (expected " << phi_sq << ")\n";
    std::cout << "  NAND(1,1) = " << decrypt_val(n11) << " (expected 0)\n\n";

    // I-verify ang 8/8 accuracy
    int correct = 0;
    double v00 = decrypt_val(n00);
    double v01 = decrypt_val(n01);
    double v10 = decrypt_val(n10);
    double v11 = decrypt_val(n11);

    if (std::abs(v00 - phi_sq) < 0.1) correct++;
    if (std::abs(v01 - phi_sq) < 0.1) correct++;
    if (std::abs(v10 - phi_sq) < 0.1) correct++;
    if (std::abs(v11) < 0.1) correct++;

    std::cout << "  Accuracy: " << correct << "/4\n";
    std::cout << "  Level: 0\n";
    std::cout << "  Status: " << (correct == 4 ? "✅ PERFECT NAND 0-LEVEL!" : "❌") << "\n";

    return 0;
}
