// CKKS + φ SA PLAINTEXT DOMAIN
// Path B: I-embed ang φ-structure sa CKKS plaintext
// Hindi na kailangan i-modify ang CKKS primes
//
// ANG KEY IDEA:
// Sa CKKS, ang plaintext ay complex numbers
// Ang φ = 1.618033... ay real number
// Ang NAND sa φ-domain ay:
//   NAND(a,b) = φ - a·b·φ⁻¹
//
// Para sa binary encoding:
//   0 → 0
//   1 → φ (≈ 1.618)
//
// Ang φ-NAND ay may period-2 property:
//   NOT(φ) = 0
//   NOT(0) = φ
//   NOT(NOT(φ)) = φ

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  CKKS + φ SA PLAINTEXT\n";
    std::cout << "  Binary sa φ-domain\n";
    std::cout << "========================================\n\n";

    // CKKS setup — maliit para mabilis
    CCParams<CryptoContextCKKSRNS> params;
    params.SetMultiplicativeDepth(30);
    params.SetScalingModSize(40);
    params.SetBatchSize(2048);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(params);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);

    auto keys = cc->KeyGen();
    cc->EvalMultKeyGen(keys.secretKey);

    auto slots = cc->GetEncodingParams()->GetBatchSize();
    auto ring = cc->GetRingDimension();

    std::cout << "CKKS: ring=" << ring << ", slots=" << slots << "\n\n";

    // φ-Structure sa plaintext
    const double phi_val = 1.6180339887498948482;
    const double inv_phi = 1.0 / phi_val;

    std::cout << "φ = " << phi_val << "\n";
    std::cout << "φ⁻¹ = " << inv_phi << "\n\n";

    // ENCODING:
    // "1" → φ (≈1.618)
    // "0" → 0
    std::vector<std::complex<double>> vec_phi(slots, {0.0, 0.0});
    vec_phi[0] = {phi_val, 0.0};

    std::vector<std::complex<double>> vec_inv_phi(slots, {0.0, 0.0});
    vec_inv_phi[0] = {inv_phi, 0.0};

    std::vector<std::complex<double>> vec_zero(slots, {0.0, 0.0});

    auto ct_phi = cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(vec_phi));
    auto ct_inv_phi = cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(vec_inv_phi));
    auto ct_zero = cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(vec_zero));
    auto ct_one = ct_phi;  // "1" = φ

    // φ-NAND: φ - a·b·φ⁻¹
    auto phi_nand = [&](auto a, auto b) {
        auto prod = cc->EvalMult(a, b);
        auto scaled = cc->EvalMult(prod, ct_inv_phi);
        return cc->EvalSub(ct_phi, scaled);
    };

    auto decrypt_val = [&](auto ct) {
        Plaintext pt;
        cc->Decrypt(keys.secretKey, ct, &pt);
        return pt->GetCKKSPackedValue()[0].real();
    };

    // ============================================
    // TRUTH TABLE
    // ============================================
    std::cout << "φ-NAND TRUTH TABLE:\n";
    std::cout << "=====================\n\n";

    auto nand_00 = phi_nand(ct_zero, ct_zero);
    auto nand_01 = phi_nand(ct_zero, ct_one);
    auto nand_10 = phi_nand(ct_one, ct_zero);
    auto nand_11 = phi_nand(ct_one, ct_one);

    std::cout << "  NAND(0,0) = " << decrypt_val(nand_00) << " (expected " << phi_val << ")\n";
    std::cout << "  NAND(0,1) = " << decrypt_val(nand_01) << " (expected " << phi_val << ")\n";
    std::cout << "  NAND(1,0) = " << decrypt_val(nand_10) << " (expected " << phi_val << ")\n";
    std::cout << "  NAND(1,1) = " << decrypt_val(nand_11) << " (expected 0)\n\n";

    // ============================================
    // PERIOD-2 TEST SA CKKS
    // ============================================
    std::cout << "PERIOD-2 TEST SA CKKS:\n";
    std::cout << "========================\n\n";

    // NOT(φ) = φ - φ²·φ⁻¹ = φ - φ = 0
    auto not_phi = phi_nand(ct_one, ct_one);
    double val_not_phi = decrypt_val(not_phi);
    std::cout << "  NOT(φ) = " << val_not_phi << " (expected 0)\n";

    // NOT(0) = φ - 0·0·φ⁻¹ = φ
    auto not_zero = phi_nand(ct_zero, ct_zero);
    double val_not_zero = decrypt_val(not_zero);
    std::cout << "  NOT(0) = " << val_not_zero << " (expected " << phi_val << ")\n";

    // NOT(NOT(φ)) = NOT(0) = φ
    auto not_not_phi = phi_nand(not_phi, not_phi);
    double val_not_not_phi = decrypt_val(not_not_phi);
    std::cout << "  NOT(NOT(φ)) = " << val_not_not_phi << " (expected " << phi_val << ")\n\n";

    // ============================================
    // DEEP CHAIN — TRUE FHE (walang decrypt sa gitna)
    // ============================================
    std::cout << "DEEP CHAIN (TRUE FHE, 14 gates):\n";
    std::cout << "===================================\n\n";

    auto current = ct_one;
    std::cout << "  Gate | Value | Expected | OK?\n";
    std::cout << "  -----|-------|----------|-----\n";

    for (int gate = 0; gate < 14; gate++) {
        current = phi_nand(current, current);

        double v = decrypt_val(current);
        double expected = (gate % 2 == 0) ? 0.0 : phi_val;
        bool ok = (std::abs(v - expected) < 0.1);

        std::cout << "  " << gate << "    | " << v
                  << " | " << expected
                  << " | " << (ok ? "YES" : "NO") << "\n";
    }

    return 0;
}
