// φ-SCALE TRACKED NAND — Tamang Scale Management
// Ang kulang: scale normalization pagkatapos ng bawat NAND
//
// ANG CYCLE:
// Input: √φ-scale ("1" = √φ, "0" = 0)
// NAND: φ - a·b (1 mult, output sa φ-scale)
// Normalize: output → √φ-scale para sa susunod na gate
//
// Ang normalization ay SUBTRACTION (0 mult!)

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <chrono>
#include <cmath>

using namespace lbcrypto;
using namespace std::chrono;

int main() {
    std::cout << "========================================\n";
    std::cout << "  φ-SCALE TRACKED NAND\n";
    std::cout << "  Tamang Scale Management\n";
    std::cout << "========================================\n\n";

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

    const double phi = 1.6180339887498948482;
    const double sqrt_phi = std::sqrt(phi);
    const double inv_sqrt_phi = 1.0 / sqrt_phi;

    // ENCODING: "1" = √φ
    auto ct_phi = make_ct(phi);
    auto ct_sqrt_phi = make_ct(sqrt_phi);
    auto ct_inv_sqrt_phi = make_ct(inv_sqrt_phi);

    // NAND sa √φ-scale: NAND(a,b) = φ - a·b (1 mult)
    // Output ay φ-scale: "1" = φ, "0" = 0
    auto nand_op = [&](auto a, auto b) {
        auto prod = cc->EvalMult(a, b);  // a·b = m₁m₂·φ
        return cc->EvalSub(ct_phi, prod);  // φ - m₁m₂·φ
    };

    // NORMALIZE: φ-scale → √φ-scale
    // "1" sa φ-scale = φ, "1" sa √φ-scale = √φ
    // normalize(φ) = √φ, normalize(0) = 0
    // Ito ay SUBTRACTION-based... pero paano?
    //
    // ANG KEY: Ang φ at √φ ay related:
    // √φ · √φ = φ
    // normalize(x) = x · inv_sqrt_phi (1 mult — hindi 0!)
    //
    // PERO: May mas magandang paraan — 
    // Gumamit na lang ng φ-scale throughout!
    // "1" = φ, "0" = 0
    // NAND = φ - a·b·(1/φ) (2 mults pa rin)
    //
    // ANG TOTOONG SOLUTION: 
    // Huwag nang mag-switch ng scale. Panatilihin ang φ-scale.

    std::cout << "FIXED SCALE: φ-encoding throughout\n";
    std::cout << "====================================\n\n";

    // Sa φ-scale: "1" = φ, "0" = 0
    // NAND(a,b) = φ - a·b·ψ (2 mults)
    // Pero ang period-2 ay stable sa φ-scale

    auto ct_psi = make_ct(1.0 / phi);
    auto nand_phi_scale = [&](auto a, auto b) {
        auto prod = cc->EvalMult(a, b);        // a·b
        auto scaled = cc->EvalMult(prod, ct_psi); // a·b·ψ
        return cc->EvalSub(ct_phi, scaled);     // φ - a·b·ψ
    };

    std::cout << "TRUTH TABLE (φ-scale):\n";
    auto ct_zero = make_ct(0.0);
    auto nand_00 = nand_phi_scale(ct_zero, ct_zero);
    auto nand_01 = nand_phi_scale(ct_zero, ct_phi);
    auto nand_11 = nand_phi_scale(ct_phi, ct_phi);

    std::cout << "  NAND(0,0) = " << decrypt_val(nand_00) << " (expected " << phi << ")\n";
    std::cout << "  NAND(0,1) = " << decrypt_val(nand_01) << " (expected " << phi << ")\n";
    std::cout << "  NAND(1,1) = " << decrypt_val(nand_11) << " (expected 0)\n\n";

    // PERIOD-2 sa φ-scale
    std::cout << "PERIOD-2 (φ-scale):\n";
    auto not_phi = nand_phi_scale(ct_phi, ct_phi);
    auto not_not_phi = nand_phi_scale(not_phi, not_phi);
    std::cout << "  NOT(φ) = " << decrypt_val(not_phi) << " (expected 0)\n";
    std::cout << "  NOT(NOT(φ)) = " << decrypt_val(not_not_phi) << " (expected " << phi << ")\n\n";

    // DEEP CHAIN: 12 gates (depth 30, 2 mults per gate)
    std::cout << "DEEP CHAIN (12 gates, 2 mults each):\n";
    std::cout << "=====================================\n\n";

    auto current = ct_phi;
    int errors = 0;

    for (int gate = 0; gate < 12; gate++) {
        current = nand_phi_scale(current, current);
        double v = decrypt_val(current);
        double expected = (gate % 2 == 0) ? 0.0 : phi;
        bool ok = (std::abs(v - expected) < 0.15 * phi);
        if (!ok) errors++;

        std::cout << "  Gate " << gate << ": v=" << v
                  << " exp=" << expected
                  << (ok ? " ✓" : " ✗") << "\n";
    }

    std::cout << "\n  Errors: " << errors << "/12\n";
    std::cout << "  (2 mults per gate, 12 gates = 24 mults sa depth 30)\n";

    return 0;
}
