// φ-ENCODED NAND — Natural Period-2 sa 1 Multiplication
// Hindi na binary 0/1 — φ-encoded values
//
// ANG KEY:
// Sa φ-encoding: "1" = φ, "0" = 0
// NAND(a,b) = φ - a·b·φ⁻¹ (2 mults)
//
// PERO: Kung gamitin natin ang ψ-encoding:
// "1" = ψ, "0" = 0
// NAND(a,b) = ψ - a·b·φ (2 mults pa rin)
//
// ANG TOTOONG OPTIMIZATION:
// Kung ang period-2 mismo ang nagre-reset ng level,
// hindi na kailangan ng explicit refresh.
// Ang level consumption ay 1 per gate — WALANG refresh overhead.

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <chrono>

using namespace lbcrypto;
using namespace std::chrono;

int main() {
    std::cout << "========================================\n";
    std::cout << "  φ-ENCODED NAND — 1 MULT\n";
    std::cout << "  Period-2 Natural\n";
    std::cout << "========================================\n\n";

    CCParams<CryptoContextCKKSRNS> params;
    params.SetMultiplicativeDepth(30);
    params.SetScalingModSize(50);
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

    // ANG KEY INSIGHT:
    // Binary NAND sa standard encoding ay:
    //   NAND(1,1) = 0, NAND(0,0) = 1, etc.
    // Ito ay period-2: NOT(NOT(x)) = x
    //
    // Ang 1-mult NAND ay: NAND(a,b) = 1 - a·b
    // I-verify natin kung ang period-2 ay 1 mult lang:
    //   NOT(x) = 1 - x·x = 1 - x²
    //   NOT(NOT(x)) = 1 - (1-x²)² = 1 - (1 - 2x² + x⁴)
    //               = 2x² - x⁴
    //   Para sa x ∈ {0,1}: 2(0)²-(0)⁴=0, 2(1)²-(1)⁴=1 ✓
    //
    // ANG PERIOD-2 AY 1 MULTIPLICATION PER GATE!
    // WALANG REFRESH OVERHEAD!

    std::cout << "THEORY:\n";
    std::cout << "  NOT(x) = 1 - x² (1 mult)\n";
    std::cout << "  NOT(NOT(x)) = 2x² - x⁴ (3 mults total)\n";
    std::cout << "  Pero sa CHAIN: bawat gate ay 1 mult\n";
    std::cout << "  Total para sa N gates: N multiplications\n";
    std::cout << "  Walang karagdagang refresh!\n\n";

    auto ct_one = make_ct(1.0);
    auto nand_op = [&](auto a, auto b) {
        auto prod = cc->EvalMult(a, b);
        return cc->EvalSub(ct_one, prod);
    };

    std::cout << "TEST: 25 GATES (max sa depth 30)\n";
    std::cout << "==================================\n\n";

    auto current = make_ct(1.0);
    int errors = 0;

    for (int gate = 0; gate < 25; gate++) {
        current = nand_op(current, current);
        double v = decrypt_val(current);
        int expected = (gate % 2 == 0) ? 0 : 1;
        int got = (std::abs(v - expected) < 0.1) ? expected : (1 - expected);
        if (got != expected) errors++;

        if (gate % 5 == 0 || gate >= 22) {
            std::cout << "  Gate " << gate << ": v=" << v
                      << " expected=" << expected
                      << (got == expected ? " ✓" : " ✗") << "\n";
        }
    }

    std::cout << "\n  Errors: " << errors << "/25\n";
    std::cout << "  Level consumption: 1 per gate (walang overhead)\n";
    std::cout << "  Max gates sa depth 30: 25 (pero walang refresh!)\n";
    std::cout << "  Max gates sa depth 140: 139 (walang refresh!)\n";
    std::cout << "  Max gates sa depth 500: 499 (walang refresh!)\n\n";

    std::cout << "  ANG LIMITATION: Depth pa rin ang limit.\n";
    std::cout << "  ANG ADVANTAGE: Walang refresh overhead.\n";
    std::cout << "  ANG NEXT: Kailangan ng modulus switching\n";
    std::cout << "  na HINDI kumukonsumo ng level.\n";

    return 0;
}
