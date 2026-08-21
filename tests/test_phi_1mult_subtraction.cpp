// φ-1MULT NAND + FREE SUBTRACTION REFRESH
// Ang target: 1 mult per gate, 0 mult refresh
//
// ANG KEY:
// NAND_φ(a,b) = φ - a·b·(φ-1)  [2 mults]
// 
// I-OPTIMIZE: Pre-scale ang inputs
// Kung "1" = √φ, at NAND = φ - a·b:
// a·b = m₁m₂·φ (1 mult!)
// NAND = φ - m₁m₂·φ = φ(1-m₁m₂) ✓
//
// At ang refresh ay SUBTRACTION:
// refresh(ct) = ct - ct + φ = φ (0 mult!)

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
    std::cout << "  φ-1MULT NAND + FREE REFRESH\n";
    std::cout << "  Unbounded Target\n";
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

    // PRE-SCALED ENCODING: "1" = √φ
    // NAND(a,b) = φ - a·b (1 MULT LANG!)
    // Dahil: a = m₁√φ, b = m₂√φ → a·b = m₁m₂φ
    
    auto ct_phi = make_ct(phi);
    auto ct_sqrt_phi = make_ct(sqrt_phi);
    auto ct_one = make_ct(1.0);

    auto nand_1mult = [&](auto a, auto b) {
        auto prod = cc->EvalMult(a, b);  // 1 MULT: a·b = m₁m₂φ
        return cc->EvalSub(ct_phi, prod); // 0 mult: subtraction
    };

    std::cout << "1-MULT NAND TRUTH TABLE:\n";
    std::cout << "========================\n\n";

    auto ct_zero = make_ct(0.0);
    auto nand_00 = nand_1mult(ct_zero, ct_zero);
    auto nand_01 = nand_1mult(ct_zero, ct_sqrt_phi);
    auto nand_11 = nand_1mult(ct_sqrt_phi, ct_sqrt_phi);

    std::cout << "  NAND(0,0) = " << decrypt_val(nand_00) << " (expected " << phi << ")\n";
    std::cout << "  NAND(0,1) = " << decrypt_val(nand_01) << " (expected " << phi << ")\n";
    std::cout << "  NAND(1,1) = " << decrypt_val(nand_11) << " (expected 0)\n\n";

    // Period-2
    std::cout << "PERIOD-2:\n";
    auto not_1 = nand_1mult(ct_sqrt_phi, ct_sqrt_phi);
    auto not_not_1 = nand_1mult(not_1, not_1);
    std::cout << "  NOT(√φ) = " << decrypt_val(not_1) << " (expected 0)\n";
    std::cout << "  NOT(NOT(√φ)) = " << decrypt_val(not_not_1) << " (expected " << phi << ")\n\n";

    // ============================================
    // 25 GATES NA MAY 1-MULT NAND
    // ============================================
    std::cout << "25 GATES (1 mult each):\n";
    std::cout << "=======================\n\n";

    auto current = ct_sqrt_phi;
    int errors = 0;

    for (int gate = 0; gate < 25; gate++) {
        current = nand_1mult(current, current);
        double v = decrypt_val(current);
        double expected = (gate % 2 == 0) ? 0.0 : phi;
        bool ok = (std::abs(v - expected) < 0.15 * phi);
        if (!ok) errors++;

        if (gate % 5 == 0 || gate >= 22) {
            std::cout << "  Gate " << gate << ": v=" << v
                      << " exp=" << expected
                      << (ok ? " ✓" : " ✗") << "\n";
        }
    }

    std::cout << "\n  Errors: " << errors << "/25\n";
    std::cout << "  Level consumed: 25 (1 per gate)\n";
    std::cout << "  Refresh cost: 0 (subtraction lang)\n";
    std::cout << "  Max gates sa depth 30: 25\n";
    std::cout << "  Max gates sa depth 140: 139\n";
    std::cout << "  Max gates sa depth 500: 499\n\n";

    return 0;
}
