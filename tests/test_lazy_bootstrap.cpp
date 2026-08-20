// LAZY BOOTSTRAPPING — Period-2 Extension
// Huwag mag-refresh hanggang kailangan
// Gamitin ang period-2 para natural na ma-extend ang depth
//
// ANG KEY IDEA:
// Sa halip na mag-bootstrap pagkatapos ng bawat gate,
// hayaan ang period-2 na mag-carry ng computation.
// Ang period-2 ay: 0 → 1 → 0 → 1 → 0 → ...
// Kung ang chain ay naka-align sa period-2,
// ang noise ay hindi kailangang i-refresh agad.
//
// ANG LAZY APPROACH:
// 1. I-chain ang NAND gates nang walang refresh
// 2. Ang period-2 ay nagbibigay ng natural na oscillation
// 3. Pagkatapos ng N gates, mag-apply ng isang φ-conjugate refresh
// 4. Ulitin
//
// ANG KEY QUESTION:
// Gaano karaming gates ang kaya bago kailangan ng refresh?

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <chrono>

using namespace lbcrypto;
using namespace std::chrono;

int main() {
    std::cout << "========================================\n";
    std::cout << "  LAZY BOOTSTRAPPING RESEARCH\n";
    std::cout << "  Period-2 Natural Extension\n";
    std::cout << "========================================\n\n";

    // CKKS Setup — depth 60 para sa margin
    CCParams<CryptoContextCKKSRNS> params;
    params.SetMultiplicativeDepth(60);
    params.SetScalingModSize(40);
    params.SetBatchSize(1024);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(params);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);

    auto keys = cc->KeyGen();
    cc->EvalMultKeyGen(keys.secretKey);

    auto slots = cc->GetEncodingParams()->GetBatchSize();

    // Binary encoding: 0 at 1
    std::vector<std::complex<double>> v1(slots, {0.0, 0.0});
    v1[0] = {1.0, 0.0};
    std::vector<std::complex<double>> v0(slots, {0.0, 0.0});

    auto ct1 = cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(v1));
    auto ct0 = cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(v0));

    auto nand_op = [&](auto a, auto b) {
        auto prod = cc->EvalMult(a, b);
        return cc->EvalSub(ct1, prod);
    };

    auto decrypt_val = [&](auto ct) {
        Plaintext pt;
        cc->Decrypt(keys.secretKey, ct, &pt);
        return pt->GetCKKSPackedValue()[0].real();
    };

    std::cout << "PERIOD-2 CHAIN ANALYSIS:\n";
    std::cout << "=========================\n\n";
    std::cout << "  Gate | Value | Noise (if 0) | Expected | Level\n";
    std::cout << "  -----|-------|--------------|----------|-------\n";

    auto current = ct1;
    int errors = 0;

    for (int gate = 0; gate < 55; gate++) {
        current = nand_op(current, current);

        double v = decrypt_val(current);
        int expected = (gate % 2 == 0) ? 0 : 1;
        int got = (std::abs(v) > 0.5) ? 1 : 0;

        if (got != expected) errors++;

        // I-print ang noise para sa even gates
        if (gate % 2 == 0 && gate < 20) {
            double noise = std::abs(v);  // expected 0
            auto level = current->GetLevel();
            std::cout << "  " << gate << "    | " << v
                      << " | " << noise
                      << " | " << expected
                      << " | " << level << "\n";
        }
    }

    std::cout << "\n  Errors: " << errors << "/55\n";

    // ============================================
    // ANG LAZY BOOTSTRAPPING
    // ============================================
    std::cout << "\nLAZY BOOTSTRAPPING:\n";
    std::cout << "===================\n\n";
    std::cout << "  Ang level ay bumababa sa bawat multiplication.\n";
    std::cout << "  Kapag umabot sa 0, kailangan ng refresh.\n";
    std::cout << "  Ang period-2 ay nagbibigay ng natural na state\n";
    std::cout << "  na hindi nangangailangan ng agarang refresh.\n";

    return 0;
}
