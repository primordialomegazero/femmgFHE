// CKKS + φ — 1 MULTIPLICATION NAND
// Hanapin ang formula na 1 mult lang per gate
//
// CURRENT: NAND(a,b) = φ - a·b·φ⁻¹ (2 mults)
// TARGET: NAND(a,b) = ??? (1 mult)
//
// IDEA 1: Pre-scale ang inputs
// Kung i-encode natin ang "1" bilang √φ sa halip na φ:
//   a = m₁·√φ, b = m₂·√φ
//   a·b = m₁m₂·φ
//   NAND = φ - a·b = φ - m₁m₂·φ = φ(1 - m₁m₂)
//   = φ kung m₁m₂=0, = 0 kung m₁m₂=1
//   → 1 MULTIPLICATION LANG!
//
// IDEA 2: Pre-scale ang output
// Kung ang "1" ay naka-encode bilang 1 (standard):
//   NAND = 1 - a·b (1 mult)
//   Tapos i-scale sa φ para sa susunod na gate
//
// Pinaka-simple: STANDARD NAND na 1 - a·b (1 mult)
// Pero may φ-scaling sa encoding

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  CKKS + φ — 1 MULT NAND\n";
    std::cout << "========================================\n\n";

    // Depth 60 para sa mas maraming gates
    CCParams<CryptoContextCKKSRNS> params;
    params.SetMultiplicativeDepth(60);
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

    std::cout << "CKKS: ring=" << ring << ", depth=60\n\n";

    // ============================================
    // METHOD 1: STANDARD NAND (1 mult)
    // NAND(a,b) = 1 - a·b
    // Encoding: 0 → 0, 1 → 1
    // ============================================
    std::cout << "METHOD 1: STANDARD NAND (1 mult)\n";
    std::cout << "==================================\n\n";

    std::vector<std::complex<double>> v1(slots, {0.0, 0.0});
    v1[0] = {1.0, 0.0};
    std::vector<std::complex<double>> v0(slots, {0.0, 0.0});

    auto ct1 = cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(v1));
    auto ct0 = cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(v0));

    auto nand_1mult = [&](auto a, auto b) {
        auto prod = cc->EvalMult(a, b);  // 1 mult
        return cc->EvalSub(ct1, prod);   // 0 mult (sub is free)
    };

    auto decrypt_val = [&](auto ct) {
        Plaintext pt;
        cc->Decrypt(keys.secretKey, ct, &pt);
        return pt->GetCKKSPackedValue()[0].real();
    };

    std::cout << "  55 gates (55 mults) — True FHE:\n\n";

    auto current = ct1;
    int errors = 0;

    for (int gate = 0; gate < 55; gate++) {
        current = nand_1mult(current, current);

        double v = decrypt_val(current);
        double expected = (gate % 2 == 0) ? 0.0 : 1.0;
        bool ok = (std::abs(v - expected) < 0.1);

        if (!ok) errors++;

        if (gate < 3 || gate >= 52) {
            std::cout << "  Gate " << gate << ": val=" << v
                      << " expected=" << expected
                      << (ok ? " YES" : " NO") << "\n";
        }
    }

    std::cout << "\n  Result: " << errors << "/55 errors ("
              << (100.0 * (55 - errors) / 55) << "%)\n\n";

    // ============================================
    // METHOD 2: φ-SCALED STANDARD NAND
    // NAND(a,b) = φ - a·b
    // Encoding: 0 → 0, 1 → √φ
    // a·b = m₁m₂·φ
    // NAND = φ - m₁m₂·φ = φ(1-m₁m₂)
    // ============================================
    std::cout << "METHOD 2: φ-SCALED NAND (1 mult)\n";
    std::cout << "===================================\n\n";

    const double sqrt_phi = std::sqrt(1.6180339887498948482);
    const double phi_val = 1.6180339887498948482;

    std::vector<std::complex<double>> vec_sqrt_phi(slots, {0.0, 0.0});
    vec_sqrt_phi[0] = {sqrt_phi, 0.0};

    std::vector<std::complex<double>> vec_phi(slots, {0.0, 0.0});
    vec_phi[0] = {phi_val, 0.0};

    auto ct_sqrt_phi = cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(vec_sqrt_phi));
    auto ct_phi_const = cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(vec_phi));

    // NAND(a,b) = φ - a·b (1 mult!)
    auto phi_scaled_nand = [&](auto a, auto b) {
        auto prod = cc->EvalMult(a, b);        // 1 mult
        return cc->EvalSub(ct_phi_const, prod); // 0 mult
    };

    std::cout << "  Encoding: 1 → √φ = " << sqrt_phi << "\n";
    std::cout << "  NAND formula: φ - a·b\n\n";

    std::cout << "  Truth table:\n";
    auto nand_00 = phi_scaled_nand(ct0, ct0);
    auto nand_01 = phi_scaled_nand(ct0, ct_sqrt_phi);
    auto nand_10 = phi_scaled_nand(ct_sqrt_phi, ct0);
    auto nand_11 = phi_scaled_nand(ct_sqrt_phi, ct_sqrt_phi);

    std::cout << "    NAND(0,0) = " << decrypt_val(nand_00) << " (expected " << phi_val << ")\n";
    std::cout << "    NAND(0,1) = " << decrypt_val(nand_01) << " (expected " << phi_val << ")\n";
    std::cout << "    NAND(1,0) = " << decrypt_val(nand_10) << " (expected " << phi_val << ")\n";
    std::cout << "    NAND(1,1) = " << decrypt_val(nand_11) << " (expected 0)\n\n";

    // Deep chain with φ-scaled
    std::cout << "  55 gates (55 mults) — True FHE:\n\n";

    auto current_phi = ct_sqrt_phi;
    int errors_phi = 0;

    for (int gate = 0; gate < 55; gate++) {
        current_phi = phi_scaled_nand(current_phi, current_phi);

        double v = decrypt_val(current_phi);
        double expected = (gate % 2 == 0) ? 0.0 : phi_val;
        bool ok = (std::abs(v - expected) < 0.15);

        if (!ok) errors_phi++;

        if (gate < 3 || gate >= 52) {
            std::cout << "  Gate " << gate << ": val=" << v
                      << " expected=" << expected
                      << (ok ? " YES" : " NO") << "\n";
        }
    }

    std::cout << "\n  Result: " << errors_phi << "/55 errors ("
              << (100.0 * (55 - errors_phi) / 55) << "%)\n";

    return 0;
}
