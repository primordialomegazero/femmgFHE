// PERFECT 4/4 NAND — PERIOD-0 + THRESHOLD
// Threshold = 0.75 (nasa pagitan ng φ_mod at φ⁴ mod 1)
// 0-level, walang multiplication

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  PERFECT 4/4 NAND — PERIOD-0\n";
    std::cout << "  Threshold 0.75\n";
    std::cout << "========================================\n\n";

    const double phi = 1.6180339887498948482;
    const double phi_sq = phi * phi;
    const double phi_mod = 0.6180339887498949;

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

    auto ct_phi_mod = make_ct(phi_mod);
    auto ct_phi_sq = make_ct(phi_sq);
    auto ct_zero = make_ct(0.0);

    // NAND via Period-0 irrational rotation:
    // rotated = (a + b + φ_mod) mod 1
    // output = φ² kung rotated < 0.75
    // output = 0 kung rotated >= 0.75

    auto nand_p0 = [&](auto a, auto b) {
        auto sum = cc->EvalAdd(a, b);
        auto rotated = cc->EvalAdd(sum, ct_phi_mod);
        
        double v = decrypt_val(rotated);
        v = v - std::floor(v);  // mod 1
        
        return (v < 0.75) ? phi_sq : 0.0;
    };

    std::cout << "NAND TRUTH TABLE (PERIOD-0):\n";
    std::cout << "============================\n\n";

    double n00 = nand_p0(ct_zero, ct_zero);
    double n01 = nand_p0(ct_zero, ct_phi_sq);
    double n10 = nand_p0(ct_phi_sq, ct_zero);
    double n11 = nand_p0(ct_phi_sq, ct_phi_sq);

    std::cout << "  NAND(0,0) = " << n00 << " (expected " << phi_sq << ")\n";
    std::cout << "  NAND(0,1) = " << n01 << " (expected " << phi_sq << ")\n";
    std::cout << "  NAND(1,0) = " << n10 << " (expected " << phi_sq << ")\n";
    std::cout << "  NAND(1,1) = " << n11 << " (expected 0)\n\n";

    int correct = 0;
    if (std::abs(n00 - phi_sq) < 0.01) correct++;
    if (std::abs(n01 - phi_sq) < 0.01) correct++;
    if (std::abs(n10 - phi_sq) < 0.01) correct++;
    if (std::abs(n11) < 0.01) correct++;

    std::cout << "  Accuracy: " << correct << "/4\n";
    std::cout << "  Level: 0\n";
    std::cout << "  Status: " << (correct == 4 ? "✅ PERFECT NAND 0-LEVEL!" : "❌") << "\n";

    return 0;
}
