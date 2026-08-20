// CKKS SIMPLE BEST — Ang Totoong Working Configuration
// Walang kumplikadong refresh — pure NAND chain
// Ito ang pinaka-stable na nakita natin
//
// ANG KEY:
// Hindi natin kailangan ng refresh para sa 20-55 gates
// Ang period-2 ay natural na naglilimita ng noise
// Refresh ay para lang sa >55 gates (kailangan ng mas malalim na depth)

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <chrono>

using namespace lbcrypto;
using namespace std::chrono;

int main() {
    std::cout << "========================================\n";
    std::cout << "  CKKS SIMPLE BEST\n";
    std::cout << "  Pure NAND Chain — No Refresh\n";
    std::cout << "========================================\n\n";

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
    auto ring = cc->GetRingDimension();
    std::cout << "Ring: " << ring << ", Slots: " << slots << "\n\n";

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

    auto nand_op = [&](auto a, auto b) {
        auto prod = cc->EvalMult(a, b);
        auto ct_one = make_ct(1.0);
        return cc->EvalSub(ct_one, prod);
    };

    std::cout << "55 GATES — NO REFRESH:\n";
    std::cout << "========================\n\n";

    auto current = make_ct(1.0);
    int errors = 0;
    int total_gates = 55;

    auto t_start = high_resolution_clock::now();

    for (int gate = 0; gate < total_gates; gate++) {
        current = nand_op(current, current);

        double v = decrypt_val(current);
        int expected = (gate % 2 == 0) ? 0 : 1;
        int got = (std::abs(v) > 0.5) ? 1 : 0;

        if (got != expected) errors++;

        if (gate < 3 || gate >= total_gates - 3 || gate % 10 == 0) {
            std::cout << "  Gate " << gate << ": v=" << v
                      << " expected=" << expected
                      << (got == expected ? " ✓" : " ✗") << "\n";
        }
    }

    auto t_end = high_resolution_clock::now();
    auto total_ms = duration_cast<milliseconds>(t_end - t_start).count();

    std::cout << "\n  Result: " << errors << "/55 errors ("
              << (100.0 * (55 - errors) / 55) << "%)\n";
    std::cout << "  Time: " << total_ms / 1000.0 << "s\n";

    return 0;
}
