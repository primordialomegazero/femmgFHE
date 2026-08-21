// 0-LEVEL NAND — COMPLETE FORMULA
// NAND(a,b) = φ² - (φ² - ((a+b) + φ²))
// Puro addition/subtraction lang!

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  0-LEVEL NAND — COMPLETE\n";
    std::cout << "  Pure Add/Sub\n";
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

    auto ct_phi_sq = make_ct(phi_sq);
    auto ct_zero = make_ct(0.0);

    // 0-LEVEL NAND:
    // NAND(a,b) = φ² - (φ² - ((a+b) + φ²))
    // Puro addition at subtraction — 0 levels!
    auto nand_0level = [&](auto a, auto b) {
        auto sum = cc->EvalAdd(a, b);               // a+b
        auto plus_phi = cc->EvalAdd(sum, ct_phi_sq); // (a+b)+φ²
        auto osc = cc->EvalSub(ct_phi_sq, plus_phi); // φ² - ((a+b)+φ²)
        return cc->EvalSub(ct_phi_sq, osc);           // φ² - osc
    };

    std::cout << "NAND TRUTH TABLE (0-LEVEL):\n";
    std::cout << "==========================\n\n";

    auto t00 = nand_0level(ct_zero, ct_zero);
    auto t0p = nand_0level(ct_zero, ct_phi_sq);
    auto tp0 = nand_0level(ct_phi_sq, ct_zero);
    auto tpp = nand_0level(ct_phi_sq, ct_phi_sq);

    std::cout << "  NAND(0,0) = " << decrypt_val(t00) << " (expected " << phi_sq << ")\n";
    std::cout << "  NAND(0,φ²) = " << decrypt_val(t0p) << " (expected " << phi_sq << ")\n";
    std::cout << "  NAND(φ²,0) = " << decrypt_val(tp0) << " (expected " << phi_sq << ")\n";
    std::cout << "  NAND(φ²,φ²) = " << decrypt_val(tpp) << " (expected 0)\n\n";

    std::cout << "LEVEL CHECK:\n";
    std::cout << "============\n";
    std::cout << "  t00 level: " << t00->GetLevel() << "\n";
    std::cout << "  t0p level: " << t0p->GetLevel() << "\n";
    std::cout << "  tpp level: " << tpp->GetLevel() << "\n\n";

    // 1000-gate NOT chain test
    std::cout << "1000-GATE NOT CHAIN (0-LEVEL NAND):\n";
    std::cout << "===================================\n\n";

    auto current = ct_phi_sq;
    int errors = 0;

    for (int gate = 0; gate < 1000; gate++) {
        current = nand_0level(current, current);
        double v = decrypt_val(current);
        double expected = (gate % 2 == 0) ? 0.0 : phi_sq;
        bool ok = (std::abs(v - expected) < 0.15 * phi_sq);
        if (!ok) errors++;

        if (gate % 100 == 0) {
            std::cout << "  Gate " << gate << ": v=" << v
                      << " exp=" << expected
                      << " level=" << current->GetLevel()
                      << (ok ? " ✓" : " ✗") << "\n";
        }
    }

    std::cout << "\n  Errors: " << errors << "/1000\n";
    std::cout << "  Level: " << current->GetLevel() << "\n";
    std::cout << "  Status: " << (errors == 0 ? "✅ HOLY GRAIL!" : "❌ FAILED") << "\n";

    return 0;
}
