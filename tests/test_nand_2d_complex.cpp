// NAND 2D — COMPLETE COMPLEX OUTPUT
// Ipakita ang real at imaginary parts

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  NAND 2D — COMPLETE COMPLEX\n";
    std::cout << "  Real + Imaginary Output\n";
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

    // 2D states:
    // A=0 → (0,0), A=1 → (φ², 0)
    // B=0 → (0,0), B=1 → (0, φ²)
    
    auto a0 = make_ct({0.0, 0.0});
    auto a1 = make_ct({phi_sq, 0.0});
    auto b0 = make_ct({0.0, 0.0});
    auto b1 = make_ct({0.0, phi_sq});

    // NAND = φ² - (a + b) sa complex domain
    auto ct_phi_sq = make_ct({phi_sq, 0.0});

    auto nand_00 = cc->EvalSub(ct_phi_sq, cc->EvalAdd(a0, b0));
    auto nand_01 = cc->EvalSub(ct_phi_sq, cc->EvalAdd(a0, b1));
    auto nand_10 = cc->EvalSub(ct_phi_sq, cc->EvalAdd(a1, b0));
    auto nand_11 = cc->EvalSub(ct_phi_sq, cc->EvalAdd(a1, b1));

    std::cout << "COMPLEX NAND OUTPUTS:\n";
    std::cout << "=====================\n\n";

    auto out_00 = decrypt_complex(nand_00);
    auto out_01 = decrypt_complex(nand_01);
    auto out_10 = decrypt_complex(nand_10);
    auto out_11 = decrypt_complex(nand_11);

    std::cout << "  NAND(0,0) = (" << out_00.real() << ", " << out_00.imag() << ")\n";
    std::cout << "  NAND(0,1) = (" << out_01.real() << ", " << out_01.imag() << ")\n";
    std::cout << "  NAND(1,0) = (" << out_10.real() << ", " << out_10.imag() << ")\n";
    std::cout << "  NAND(1,1) = (" << out_11.real() << ", " << out_11.imag() << ")\n\n";

    std::cout << "MAGNITUDES:\n";
    std::cout << "============\n\n";
    std::cout << "  |NAND(0,0)| = " << std::abs(out_00) << "\n";
    std::cout << "  |NAND(0,1)| = " << std::abs(out_01) << "\n";
    std::cout << "  |NAND(1,0)| = " << std::abs(out_10) << "\n";
    std::cout << "  |NAND(1,1)| = " << std::abs(out_11) << "\n\n";

    std::cout << "  Level: 0\n";
    std::cout << "  Status: Complex NAND complete\n";

    return 0;
}
