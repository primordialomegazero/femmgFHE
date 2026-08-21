// φ-DOMAIN 1-MULT NAND
// Para sa 30 gates sa depth 30
//
// IDEA:
// NAND(a,b) = φ - a·b
// Kung ang "1" ay naka-scale bilang sqrt(φ),
// ang multiply ay magbibigay ng φ-scale output.
//
// Subukan natin ang stable fixed point:
// "1" = sqrt(φ), "0" = 0
// NAND(a,b) = φ - a·b
// Period: NOT(x) = φ - x²
// NOT(sqrt(φ)) = φ - φ = 0
// NOT(0) = φ (hindi sqrt(φ)) → kailangan ng scale fix

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
    std::cout << "  φ-DOMAIN 1-MULT NAND\n";
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
    auto ct_phi = make_ct(phi);

    // SUBUKAN NATIN: 1-mult NAND
    // NAND(a,b) = φ - a·b
    // Kung "1" = 1 at "0" = 0:
    // NAND(0,0) = φ - 0 = φ (hindi 1, pero φ ang "true")
    // NAND(0,1) = φ - 0 = φ
    // NAND(1,1) = φ - 1 = 0.618 (hindi 0)
    //
    // Kaya kailangan ng scale compensation...
    // SUBUKAN NATIN ANG PURE φ-VALUES:
    // "1" = φ, "0" = 0
    // NAND(a,b) = φ - a·b·(1/φ) — 2 mults pa rin
    //
    // ALTERNATIVE: Pre-scale bago multiply
    // "1" = 1 (hindi φ)
    // NAND(a,b) = 1 - a·b — ito ang standard NAND!
    // Pero ang standard NAND ay may scale drift...

    std::cout << "STANDARD NAND (1 = 1, 0 = 0):\n";
    std::cout << "==============================\n\n";

    auto ct_one = make_ct(1.0);
    auto ct_zero = make_ct(0.0);

    auto nand_std = [&](auto a, auto b) {
        auto prod = cc->EvalMult(a, b);
        return cc->EvalSub(ct_one, prod);
    };

    // Truth table
    std::cout << "Truth table:\n";
    auto t00 = nand_std(ct_zero, ct_zero);
    auto t01 = nand_std(ct_zero, ct_one);
    auto t11 = nand_std(ct_one, ct_one);
    std::cout << "  NAND(0,0) = " << decrypt_val(t00) << " (expected 1)\n";
    std::cout << "  NAND(0,1) = " << decrypt_val(t01) << " (expected 1)\n";
    std::cout << "  NAND(1,1) = " << decrypt_val(t11) << " (expected 0)\n\n";

    // 30 gates, 1 mult each
    std::cout << "30 GATES (1 mult each):\n";
    std::cout << "========================\n\n";

    auto current = ct_one;
    int errors = 0;

    for (int gate = 0; gate < 30; gate++) {
        current = nand_std(current, current);

        double v = decrypt_val(current);
        double expected = (gate % 2 == 0) ? 0.0 : 1.0;
        bool ok = (std::abs(v - expected) < 0.15);
        if (!ok) errors++;

        if (gate % 5 == 0 || !ok) {
            std::cout << "  Gate " << gate << ": v=" << v
                      << " exp=" << expected
                      << (ok ? " ✓" : " ✗") << "\n";
        }
    }

    std::cout << "\n  Result: " << errors << "/30 errors\n";
    std::cout << "  (1 mult per gate, 30 gates = 30 mults sa depth 30)\n";

    return 0;
}
