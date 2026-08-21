// NAND 2D — TAMANG FORMULA
// NAND = φ² kung (real=0 o imaginary=0)
// NAND = 0 kung (real=φ² at imaginary=φ²)

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  NAND 2D — TAMANG FORMULA\n";
    std::cout << "  Complete Truth Table\n";
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

    auto make_ct = [&](std::complex<double> val) {
        std::vector<std::complex<double>> vec(slots, {0.0, 0.0});
        vec[0] = val;
        return cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(vec));
    };

    auto decrypt_complex = [&](auto ct) {
        Plaintext pt;
        cc->Decrypt(keys.secretKey, ct, &pt);
        return pt->GetCKKSPackedValue()[0];
    };

    auto a0 = make_ct({0.0, 0.0});
    auto a1 = make_ct({phi_sq, 0.0});
    auto b0 = make_ct({0.0, 0.0});
    auto b1 = make_ct({0.0, phi_sq});
    auto ct_phi_sq = make_ct({phi_sq, 0.0});

    // Tamang NAND para sa 2D:
    // Kung real=0 at imaginary=0 → φ² (true)
    // Kung real=0 at imaginary=φ² → φ² (true)
    // Kung real=φ² at imaginary=0 → φ² (true)
    // Kung real=φ² at imaginary=φ² → 0 (false)
    //
    // Formula: NAND = φ² - (real × imaginary) / φ²
    // May multiplication — pero sa 2D, ito ay complex multiply
    //
    // Sa 0-level: gumamit ng period-4 oscillation

    // Subukan ang alternative na walang mult:
    // NAND = φ² kung (real + imaginary) < 2φ²
    // NAND = 0 kung (real + imaginary) >= 2φ²

    std::cout << "2D NAND VALUES:\n";
    std::cout << "==============\n\n";

    auto sum_00 = cc->EvalAdd(a0, b0);
    auto sum_01 = cc->EvalAdd(a0, b1);
    auto sum_10 = cc->EvalAdd(a1, b0);
    auto sum_11 = cc->EvalAdd(a1, b1);

    auto out_00 = decrypt_complex(sum_00);
    auto out_01 = decrypt_complex(sum_01);
    auto out_10 = decrypt_complex(sum_10);
    auto out_11 = decrypt_complex(sum_11);

    std::cout << "  Sum(0,0) = (" << out_00.real() << ", " << out_00.imag() << ")\n";
    std::cout << "  Sum(0,1) = (" << out_01.real() << ", " << out_01.imag() << ")\n";
    std::cout << "  Sum(1,0) = (" << out_10.real() << ", " << out_10.imag() << ")\n";
    std::cout << "  Sum(1,1) = (" << out_11.real() << ", " << out_11.imag() << ")\n\n";

    // Real + Imaginary magnitudes
    std::cout << "  |Sum| values:\n";
    std::cout << "  (0,0): " << std::abs(out_00) << "\n";
    std::cout << "  (0,1): " << std::abs(out_01) << "\n";
    std::cout << "  (1,0): " << std::abs(out_10) << "\n";
    std::cout << "  (1,1): " << std::abs(out_11) << "\n\n";

    std::cout << "  Level: 0\n";
    std::cout << "  Status: 2D sum complete\n";

    return 0;
}
