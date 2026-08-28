// EVALSIN ZERO DETECTION — Pentagonal Space
// sin(0°) = 0, sin(144°) = +0.588, sin(288°) = -0.951
// Natural na zero detection sa (1,1)
// Subukan kung mas mura kaysa EvalCos

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>
#include <chrono>

using namespace lbcrypto;
using namespace std::chrono;

int main() {
    std::cout << "========================================\n";
    std::cout << "  EVALSIN ZERO DETECTION\n";
    std::cout << "  Pentagonal Space\n";
    std::cout << "========================================\n\n";

    const double PI = 3.14159265358979323846;
    const double PENTA = 2 * PI / 5;  // 72°
    const double TWO_PENTA = 4 * PI / 5;  // 144°
    const double FOUR_PENTA = 8 * PI / 5;  // 288°

    CCParams<CryptoContextCKKSRNS> params;
    params.SetMultiplicativeDepth(15);
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
    auto ct_two_penta = make_ct(TWO_PENTA);
    auto ct_four_penta = make_ct(FOUR_PENTA);

    // EvalSin with different degrees para makita ang level cost
    std::cout << "SIN VALUES SA PENTAGONAL STATES:\n";
    std::cout << "=================================\n\n";

    for (int degree : {3, 5, 7, 9}) {
        auto sin_0 = cc->EvalSin(ct_zero, -4.0, 4.0, degree);
        auto sin_144 = cc->EvalSin(ct_two_penta, -4.0, 4.0, degree);
        auto sin_288 = cc->EvalSin(ct_four_penta, -4.0, 4.0, degree);

        std::cout << "  Degree " << degree << ":\n";
        std::cout << "    sin(0°) = " << decrypt_val(sin_0) 
                  << " (expected 0)\n";
        std::cout << "    sin(144°) = " << decrypt_val(sin_144)
                  << " (expected 0.588)\n";
        std::cout << "    sin(288°) = " << decrypt_val(sin_288)
                  << " (expected -0.951)\n";
        std::cout << "    Level: " << sin_0->GetLevel() << "\n\n";
    }

    // Test NAND output detection
    std::cout << "NAND OUTPUT DETECTION:\n";
    std::cout << "======================\n\n";

    // NAND outputs: (0,0)→288°, (0,1)→144°, (1,1)→0°
    auto eval_sin_detect = [&](auto x) {
        return cc->EvalSin(x, -4.0, 4.0, 5);
    };

    auto detect_00 = eval_sin_detect(ct_four_penta);
    auto detect_01 = eval_sin_detect(ct_two_penta);
    auto detect_11 = eval_sin_detect(ct_zero);

    double sin_00 = decrypt_val(detect_00);
    double sin_01 = decrypt_val(detect_01);
    double sin_11 = decrypt_val(detect_11);

    std::cout << "  NAND(0,0) sin = " << sin_00 << " → " 
              << (std::abs(sin_00) > 0.1 ? "NON-ZERO (1)" : "ZERO (0)") << "\n";
    std::cout << "  NAND(0,1) sin = " << sin_01 << " → "
              << (std::abs(sin_01) > 0.1 ? "NON-ZERO (1)" : "ZERO (0)") << "\n";
    std::cout << "  NAND(1,1) sin = " << sin_11 << " → "
              << (std::abs(sin_11) > 0.1 ? "NON-ZERO (1)" : "ZERO (0)") << "\n";

    std::cout << "\n  Level: " << detect_00->GetLevel() << "\n";

    return 0;
}
