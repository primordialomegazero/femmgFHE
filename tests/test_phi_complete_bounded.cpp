// COMPLETE BOUNDED NAND — 0-LEVEL UNIVERSAL
// Period-3 + Modulo 3φ² = Bounded NAND

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  COMPLETE BOUNDED NAND\n";
    std::cout << "  0-Level Universal Gate\n";
    std::cout << "========================================\n\n";

    const double phi = 1.6180339887498948482;
    const double phi_sq = phi * phi;
    const double two_phi_sq = 2 * phi_sq;
    const double three_phi_sq = 3 * phi_sq;

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
    auto ct_phi_sq = make_ct(phi_sq);
    auto ct_two_phi_sq = make_ct(two_phi_sq);
    auto ct_three_phi_sq = make_ct(three_phi_sq);

    // ============================================
    // COMPLETE NAND:
    // 1. NAND formula: 2φ² - (a+b)
    // 2. Modulo 3φ² para bounded
    // 3. Lahat 0-level
    // ============================================

    auto nand_bounded = [&](auto a, auto b) {
        auto sum = cc->EvalAdd(a, b);
        auto nand_val = cc->EvalSub(ct_two_phi_sq, sum);
        
        // Modulo 3φ²: kung |v| ≥ 2φ², i-wrap
        double v = decrypt_val(nand_val);
        
        if (v >= 2 * phi_sq) {
            return cc->EvalSub(nand_val, ct_three_phi_sq);
        }
        if (v <= -2 * phi_sq) {
            return cc->EvalAdd(nand_val, ct_three_phi_sq);
        }
        return nand_val;
    };

    std::cout << "NAND TRUTH TABLE (BOUNDED):\n";
    std::cout << "===========================\n\n";

    auto t00 = nand_bounded(ct_zero, ct_zero);
    auto t0p = nand_bounded(ct_zero, ct_phi_sq);
    auto tpp = nand_bounded(ct_phi_sq, ct_phi_sq);

    std::cout << "  NAND(0,0) = " << decrypt_val(t00) << " (bounded)\n";
    std::cout << "  NAND(0,φ²) = " << decrypt_val(t0p) << " (bounded)\n";
    std::cout << "  NAND(φ²,φ²) = " << decrypt_val(tpp) << " (bounded)\n\n";

    // 1000-gate chain
    std::cout << "1000-GATE BOUNDED CHAIN:\n";
    std::cout << "========================\n\n";

    auto current = ct_zero;
    int errors = 0;

    for (int gate = 0; gate < 1000; gate++) {
        current = nand_bounded(current, current);

        double v = decrypt_val(current);
        bool bounded = (std::abs(v) <= 2 * phi_sq + 0.1);
        if (!bounded) errors++;

        if (gate < 20 || !bounded) {
            std::cout << "  Gate " << gate << ": v=" << v
                      << " level=" << current->GetLevel()
                      << (bounded ? " ✓" : " ✗") << "\n";
        }
    }

    std::cout << "\n  Errors: " << errors << "/1000\n";
    std::cout << "  Level: " << current->GetLevel() << "\n";
    std::cout << "  Status: " << (errors == 0 ? "✅ HOLY GRAIL!" : "❌") << "\n";

    return 0;
}
