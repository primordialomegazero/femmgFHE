// CKKS 190 GATES — Depth 200
// Pure NAND chain, walang refresh
// TARGET: 190 gates, 0 errors

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <chrono>

using namespace lbcrypto;
using namespace std::chrono;

int main() {
    std::cout << "========================================\n";
    std::cout << "  CKKS 190 GATES — DEPTH 200\n";
    std::cout << "========================================\n\n";

    CCParams<CryptoContextCKKSRNS> params;
    params.SetMultiplicativeDepth(200);
    params.SetScalingModSize(40);
    params.SetBatchSize(512);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(params);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);

    std::cout << "Generating keys...\n";
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

    auto ct_one = make_ct(1.0);
    auto nand_op = [&](auto a, auto b) {
        auto prod = cc->EvalMult(a, b);
        return cc->EvalSub(ct_one, prod);
    };

    std::cout << "190 GATES — PURE NAND:\n";
    std::cout << "=======================\n\n";

    auto current = make_ct(1.0);
    int errors = 0;
    int total_gates = 190;

    auto t_start = high_resolution_clock::now();

    for (int gate = 0; gate < total_gates; gate++) {
        current = nand_op(current, current);

        // Verify every 20 gates para sa speed
        if (gate % 20 == 0 || gate >= total_gates - 3) {
            double v = decrypt_val(current);
            int expected = (gate % 2 == 0) ? 0 : 1;
            int got = (std::abs(v) > 0.5) ? 1 : 0;

            if (got != expected) errors++;

            std::cout << "  Gate " << gate << ": v=" << v
                      << " expected=" << expected
                      << (got == expected ? " ✓" : " ✗") << "\n";
        }
    }

    auto t_end = high_resolution_clock::now();
    auto total_ms = duration_cast<milliseconds>(t_end - t_start).count();

    std::cout << "\n  Result: " << errors << " errors sa " 
              << (total_gates / 20 + 3) << " verified gates\n";
    std::cout << "  Time: " << total_ms / 1000.0 << "s\n";
    std::cout << "  Per gate: " << (double)total_ms / total_gates << " ms\n";

    return 0;
}
