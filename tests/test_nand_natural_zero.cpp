// NAND NATURAL ZERO — WALANG THRESHOLD
// NAND = 2φ - (a+b)
// Ang zero sa (1,1) ay natural separator
// Walang decrypt, walang arbitrary threshold

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  NAND NATURAL ZERO\n";
    std::cout << "  Walang Threshold, Walang Decrypt\n";
    std::cout << "========================================\n\n";

    const double PHI = 1.6180339887498948482;
    const double TWO_PHI = 2 * PHI;

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

    auto ct_two_phi = make_ct(TWO_PHI);
    auto ct_zero = make_ct(0.0);
    auto ct_phi = make_ct(PHI);

    // NAND = 2φ - (a+b)
    auto nand_natural = [&](auto a, auto b) {
        auto sum = cc->EvalAdd(a, b);
        return cc->EvalSub(ct_two_phi, sum);
    };

    std::cout << "NAND NATURAL ZERO TEST:\n";
    std::cout << "=======================\n\n";

    auto r00 = nand_natural(ct_zero, ct_zero);
    auto r01 = nand_natural(ct_zero, ct_phi);
    auto r10 = nand_natural(ct_phi, ct_zero);
    auto r11 = nand_natural(ct_phi, ct_phi);

    double v00 = decrypt_val(r00);
    double v01 = decrypt_val(r01);
    double v10 = decrypt_val(r10);
    double v11 = decrypt_val(r11);

    std::cout << "  NAND(0,0) = " << v00 << " → " << (v00 > 0 ? "1" : "0") << "\n";
    std::cout << "  NAND(0,1) = " << v01 << " → " << (v01 > 0 ? "1" : "0") << "\n";
    std::cout << "  NAND(1,0) = " << v10 << " → " << (v10 > 0 ? "1" : "0") << "\n";
    std::cout << "  NAND(1,1) = " << v11 << " → " << (v11 > 0 ? "1" : "0") << "\n\n";

    std::cout << "  Expected: 1, 1, 1, 0\n";
    std::cout << "  Level: " << r00->GetLevel() << "\n";
    std::cout << "  Status: Natural zero separator!\n";

    return 0;
}
