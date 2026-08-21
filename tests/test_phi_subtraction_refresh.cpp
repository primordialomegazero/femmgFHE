// φ-SUBTRACTION REFRESH — Zero-Cost Level Reset
// Ang φ^(-1) = φ - 1 (subtraction, hindi multiplication!)
//
// ANG KEY:
// φ · φ^(-1) = 1
// φ · (φ - 1) = 1
// Kaya ang inverse ng φ ay SUBTRACTION lang!
//
// Kung ang refresh ay subtraction:
// - Level cost: 0 (subtraction ay libre)
// - Ito ay natural na unbounded mechanism!

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <chrono>

using namespace lbcrypto;
using namespace std::chrono;

int main() {
    std::cout << "========================================\n";
    std::cout << "  φ-SUBTRACTION REFRESH\n";
    std::cout << "  Zero-Cost Level Reset\n";
    std::cout << "========================================\n\n";

    std::cout << "THEORY:\n";
    std::cout << "  φ⁻¹ = φ - 1 (SUBTRACTION!)\n";
    std::cout << "  φ · (φ - 1) = φ² - φ = 1\n\n";

    std::cout << "  Sa CKKS:\n";
    std::cout << "  Subtraction ay may 0 level cost!\n";
    std::cout << "  Kung ang refresh ay subtraction lang,\n";
    std::cout << "  WALANG level consumption!\n\n";

    // Standard CKKS (walang patch)
    CCParams<CryptoContextCKKSRNS> params;
    params.SetMultiplicativeDepth(30);
    params.SetScalingModSize(40);
    params.SetBatchSize(256);

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

    // ============================================
    // φ-ENCODING NA MAY SUBTRACTION REFRESH
    // ============================================
    // Sa φ-encoding:
    // "1" = φ, "0" = 0
    // NAND(a,b) = φ - a·b·φ⁻¹ = φ - a·b·(φ-1)
    // 
    // Ang NAND ay may 2 multiplications pa rin...
    // PERO ang refresh ay maaaring SUBTRACTION lang!

    const double phi = 1.6180339887498948482;
    const double phi_minus_1 = phi - 1.0;  // = ψ

    auto ct_phi = make_ct(phi);
    auto ct_phi_minus_1 = make_ct(phi_minus_1);
    auto ct_one = make_ct(1.0);

    std::cout << "φ = " << phi << "\n";
    std::cout << "φ - 1 = " << phi_minus_1 << " (= ψ)\n\n";

    // TEST: Subtraction-based refresh
    // refresh(ct) = ct - (φ-1)·something
    // O mas simple: refresh(ct) = ct - ct + φ = φ
    
    auto nand_phi = [&](auto a, auto b) {
        auto prod = cc->EvalMult(a, b);           // 1 mult: a·b
        auto scaled = cc->EvalMult(prod, ct_phi_minus_1);  // 1 mult: a·b·(φ-1)
        return cc->EvalSub(ct_phi, scaled);        // 0 mult: subtraction
    };

    std::cout << "NAND_φ truth table:\n";
    std::cout << "===================\n\n";

    auto ct_zero = make_ct(0.0);
    auto nand_00 = nand_phi(ct_zero, ct_zero);
    auto nand_01 = nand_phi(ct_zero, ct_phi);
    auto nand_11 = nand_phi(ct_phi, ct_phi);

    std::cout << "  NAND(0,0) = " << decrypt_val(nand_00) << " (expected " << phi << ")\n";
    std::cout << "  NAND(0,1) = " << decrypt_val(nand_01) << " (expected " << phi << ")\n";
    std::cout << "  NAND(1,1) = " << decrypt_val(nand_11) << " (expected 0)\n\n";

    // Period-2 test
    std::cout << "PERIOD-2 TEST:\n";
    auto not_phi = nand_phi(ct_phi, ct_phi);
    auto not_not_phi = nand_phi(not_phi, not_phi);
    std::cout << "  NOT(φ) = " << decrypt_val(not_phi) << " (expected 0)\n";
    std::cout << "  NOT(NOT(φ)) = " << decrypt_val(not_not_phi) << " (expected " << phi << ")\n\n";

    std::cout << "========================================\n";
    std::cout << "  KEY INSIGHT:\n";
    std::cout << "  Ang NAND_φ ay may 2 multiplications.\n";
    std::cout << "  Pero ang REFRESH ay subtraction — 0 mult!\n";
    std::cout << "  Kung maaari nating gawing 1 mult ang NAND,\n";
    std::cout << "  ang refresh ay FREE.\n";
    std::cout << "========================================\n";

    return 0;
}
