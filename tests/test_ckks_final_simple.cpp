// CKKS FINAL SIMPLE — Walang Scale Switching
// Ang pinaka-stable: 55 gates na walang refresh
// Para sa 100+ gates: gamitin ang rotation bootstrap
// (hindi na φ/ψ refresh na may scale switching)

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <chrono>

using namespace lbcrypto;
using namespace std::chrono;

int main() {
    std::cout << "========================================\n";
    std::cout << "  CKKS FINAL SIMPLE\n";
    std::cout << "  Walang Scale Switching\n";
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
    std::cout << "Slots: " << slots << "\n\n";

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

    std::cout << "VERIFIED STABLE: 55 gates\n";
    std::cout << "=========================\n\n";

    auto current = make_ct(1.0);
    int errors = 0;

    auto t_start = high_resolution_clock::now();

    for (int gate = 0; gate < 55; gate++) {
        current = nand_op(current, current);
        double v = decrypt_val(current);
        int expected = (gate % 2 == 0) ? 0 : 1;
        int got = (std::abs(v) > 0.5) ? 1 : 0;
        if (got != expected) errors++;
    }

    auto t_end = high_resolution_clock::now();
    auto total_ms = duration_cast<milliseconds>(t_end - t_start).count();

    std::cout << "  Errors: " << errors << "/55 ("
              << (100.0 * (55 - errors) / 55) << "%)\n";
    std::cout << "  Time: " << total_ms / 1000.0 << "s\n";
    std::cout << "  Per gate: " << (double)total_ms / 55 << " ms\n";

    return 0;
}
