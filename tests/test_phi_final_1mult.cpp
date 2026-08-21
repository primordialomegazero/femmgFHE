// φ-FINAL 1-MULT NAND — Stable Scale
// Ang solusyon: ang "1" ay 1 (hindi φ o √φ)
// NAND(a,b) = 1 - a·b (1 mult, 0 scale issues)
//
// ITO ANG PINAKA-SIMPLE AT PINAKA-STABLE:
// Binary encoding: 0 at 1
// NAND: 1 - a·b (1 multiplication)
// Period-2: NOT(NOT(1)) = 1 ✓
// Walang scale tracking kailangan!

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <chrono>

using namespace lbcrypto;
using namespace std::chrono;

int main() {
    std::cout << "========================================\n";
    std::cout << "  φ-FINAL 1-MULT NAND\n";
    std::cout << "  Stable Binary Encoding\n";
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

    auto ct_one = make_ct(1.0);
    auto nand_op = [&](auto a, auto b) {
        auto prod = cc->EvalMult(a, b);  // 1 MULT
        return cc->EvalSub(ct_one, prod); // 0 mult (subtraction)
    };

    std::cout << "25 GATES (1 mult each, binary 0/1):\n";
    std::cout << "=====================================\n\n";

    auto current = make_ct(1.0);
    int errors = 0;

    auto t_start = high_resolution_clock::now();

    for (int gate = 0; gate < 25; gate++) {
        current = nand_op(current, current);
        double v = decrypt_val(current);
        int expected = (gate % 2 == 0) ? 0 : 1;
        int got = (std::abs(v) > 0.5) ? 1 : 0;
        if (got != expected) errors++;

        std::cout << "  Gate " << gate << ": v=" << v
                  << " exp=" << expected
                  << (got == expected ? " ✓" : " ✗") << "\n";
    }

    auto t_end = high_resolution_clock::now();
    auto total_ms = duration_cast<milliseconds>(t_end - t_start).count();

    std::cout << "\n  Errors: " << errors << "/25\n";
    std::cout << "  Time: " << total_ms / 1000.0 << "s\n";
    std::cout << "  Per gate: " << (double)total_ms / 25 << " ms\n";
    std::cout << "  Level: 1 per gate (walang overhead)\n";

    return 0;
}
