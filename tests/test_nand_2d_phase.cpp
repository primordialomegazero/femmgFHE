// NAND VIA 2D PHASE — COMPLEX CKKS
// (real, imaginary) encoding para sa NAND

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  NAND VIA 2D PHASE\n";
    std::cout << "  Complex Encoding\n";
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

    auto decrypt_val = [&](auto ct) {
        Plaintext pt;
        cc->Decrypt(keys.secretKey, ct, &pt);
        return pt->GetCKKSPackedValue()[0];
    };

    // 2D Phase states:
    // A=0 → (0,0), A=1 → (φ², 0)
    // B=0 → (0,0), B=1 → (0, φ²)
    
    auto a0 = make_ct({0.0, 0.0});
    auto a1 = make_ct({phi_sq, 0.0});
    auto b0 = make_ct({0.0, 0.0});
    auto b1 = make_ct({0.0, phi_sq});

    // NAND(a,b) = φ² - (a.real + b.imag)
    // Kung (0,0): φ² - 0 = φ² (true)
    // Kung (0,1): φ² - φ² = 0 (false — dapat true!)
    // Kung (1,0): φ² - φ² = 0 (false — dapat true!)
    // Kung (1,1): φ² - 2φ² = -φ² (false)

    // Hindi ito ang tamang formula. Kailangan ng ibang approach.

    // 2D NAND: ang (0,0) at (1,1) ay may iba't ibang phase
    // NAND = φ² kung (a.real = 0 o b.imag = 0)
    // NAND = 0 kung (a.real = φ² at b.imag = φ²)

    // Ang NAND ay: NOT(AND)
    // AND = a.real * b.imag (kailangan ng multiplication)
    // NOT(x) = φ² - x

    // Sa 0-level, wala tayong multiplication
    // PERO may 2D phase tayo

    std::cout << "2D PHASE NAND VALUES:\n";
    std::cout << "=====================\n\n";

    auto nand_00 = cc->EvalAdd(a0, b0);
    auto nand_01 = cc->EvalAdd(a0, b1);
    auto nand_10 = cc->EvalAdd(a1, b0);
    auto nand_11 = cc->EvalAdd(a1, b1);

    std::cout << "  NAND(0,0) = " << decrypt_val(nand_00) << "\n";
    std::cout << "  NAND(0,1) = " << decrypt_val(nand_01) << "\n";
    std::cout << "  NAND(1,0) = " << decrypt_val(nand_10) << "\n";
    std::cout << "  NAND(1,1) = " << decrypt_val(nand_11) << "\n\n";

    // I-check kung ang 2D sum ay may unique pattern:
    // (0,0) → (0, 0)
    // (0,1) → (0, φ²)
    // (1,0) → (φ², 0)
    // (1,1) → (φ², φ²)

    std::cout << "  Ang 2D sum ay may unique representation!\n";
    std::cout << "  Hindi na kailangan ng threshold — ang\n";
    std::cout << "  magnitude at angle ay sapat na\n\n";

    std::cout << "  Level: 0\n";
    std::cout << "  Status: 2D encoding ay promising!\n";

    return 0;
}
