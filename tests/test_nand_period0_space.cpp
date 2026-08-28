// NAND SA PERIOD-0 SPACE
// 0 → 0, 1 → φ_mod
// Natural modulo sa [0,1)
// Walang decrypt, 0-level

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  NAND SA PERIOD-0 SPACE\n";
    std::cout << "  Natural Modulo sa [0,1)\n";
    std::cout << "========================================\n\n";

    const double phi_mod = 0.6180339887498949;  // 1 → φ_mod
    const double ZERO = 0.0;                     // 0 → 0

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

    auto ct_zero = make_ct(0.0);
    auto ct_one = make_ct(phi_mod);

    // NAND sa Period-0: 3φ - (a+b) mod 1
    auto nand_p0 = [&](auto a, auto b) {
        auto sum = cc->EvalAdd(a, b);
        auto ct_three_phi = make_ct(3 * phi_mod);
        auto raw = cc->EvalSub(ct_three_phi, sum);
        return raw;
    };

    std::cout << "NAND SA PERIOD-0 (walang decrypt):\n";
    std::cout << "================================\n\n";

    auto r00 = nand_p0(ct_zero, ct_zero);
    auto r01 = nand_p0(ct_zero, ct_one);
    auto r10 = nand_p0(ct_one, ct_zero);
    auto r11 = nand_p0(ct_one, ct_one);

    double v00 = decrypt_val(r00);
    double v01 = decrypt_val(r01);
    double v10 = decrypt_val(r10);
    double v11 = decrypt_val(r11);

    v00 = v00 - std::floor(v00);
    v01 = v01 - std::floor(v01);
    v10 = v10 - std::floor(v10);
    v11 = v11 - std::floor(v11);

    std::cout << "  NAND(0,0) = " << v00 << " → " << (v00 > 0.3 ? "1" : "0") << "\n";
    std::cout << "  NAND(0,1) = " << v01 << " → " << (v01 > 0.3 ? "1" : "0") << "\n";
    std::cout << "  NAND(1,0) = " << v10 << " → " << (v10 > 0.3 ? "1" : "0") << "\n";
    std::cout << "  NAND(1,1) = " << v11 << " → " << (v11 > 0.3 ? "1" : "0") << "\n\n";

    std::cout << "  Expected: 1, 1, 1, 0\n";
    std::cout << "  Level: " << r00->GetLevel() << "\n";

    return 0;
}
