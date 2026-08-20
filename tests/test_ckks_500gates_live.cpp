// CKKS 500 GATES — LIVE FOREGROUND
// Depth 500, pure NAND chain
// Estimated: ~2.8 hours, makikita mo bawat 50 gates

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <chrono>

using namespace lbcrypto;
using namespace std::chrono;

int main() {
    std::cout << "========================================\n";
    std::cout << "  CKKS 500 GATES — LIVE\n";
    std::cout << "========================================\n\n";

    CCParams<CryptoContextCKKSRNS> params;
    params.SetMultiplicativeDepth(500);
    params.SetScalingModSize(40);
    params.SetBatchSize(512);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(params);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);

    std::cout << "Generating keys (matagal 'to, ~5 min)...\n";
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

    std::cout << "500 GATES — PURE NAND:\n";
    std::cout << "========================\n\n";

    auto current = make_ct(1.0);
    int errors = 0;
    int total_gates = 500;

    auto t_start = high_resolution_clock::now();

    for (int gate = 0; gate < total_gates; gate++) {
        current = nand_op(current, current);

        double v = decrypt_val(current);
        int expected = (gate % 2 == 0) ? 0 : 1;
        int got = (std::abs(v) > 0.5) ? 1 : 0;

        if (got != expected) errors++;

        // Live progress every 50 gates
        if (gate % 50 == 0) {
            auto t_now = high_resolution_clock::now();
            auto elapsed = duration_cast<seconds>(t_now - t_start).count();
            std::cout << "  Gate " << gate << "/" << total_gates
                      << ": v=" << v
                      << " expected=" << expected
                      << (got == expected ? " ✓" : " ✗")
                      << " (elapsed: " << elapsed / 60 << "m " << elapsed % 60 << "s)\n";
        }

        // Mas madalas sa dulo
        if (gate >= total_gates - 10) {
            std::cout << "  Gate " << gate << ": v=" << v
                      << " expected=" << expected
                      << (got == expected ? " ✓" : " ✗") << "\n";
        }
    }

    auto t_end = high_resolution_clock::now();
    auto total_ms = duration_cast<milliseconds>(t_end - t_start).count();

    std::cout << "\n========================================\n";
    std::cout << "  RESULT:\n";
    std::cout << "  Gates: " << total_gates << "\n";
    std::cout << "  Errors: " << errors << "\n";
    std::cout << "  Accuracy: " << (100.0 * (total_gates - errors) / total_gates) << "%\n";
    std::cout << "  Total time: " << total_ms / 1000.0 / 60.0 << " minutes\n";
    std::cout << "  Per gate: " << (double)total_ms / total_gates << " ms\n";
    std::cout << "========================================\n";

    return 0;
}
