// NAND DEPTH SCALING — LEVEL 0
// Sunod-sunod na NAND na walang multiplication
// Tignan kung hanggang ilang layers ang kaya

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  NAND DEPTH SCALING — LEVEL 0\n";
    std::cout << "  Walang Mult, Walang Bootstrapping\n";
    std::cout << "========================================\n\n";

    const double PI = 3.14159265358979323846;
    const double ENC_0 = -2 * PI / 5;
    const double ENC_1 = 2 * PI / 5;
    const double GOLDEN_ANGLE = 2 * PI * (1.0 - 1.0 / 1.618033988749895);
    const double AND_THRESHOLD = GOLDEN_ANGLE - PI/10;

    CCParams<CryptoContextCKKSRNS> params;
    params.SetMultiplicativeDepth(1);
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

    auto ct_0 = make_ct(ENC_0);
    auto ct_1 = make_ct(ENC_1);

    auto eval_nand = [&](auto a, auto b) {
        auto sum = cc->EvalAdd(a, b);
        return cc->EvalNegate(sum);
    };

    auto decrypt_bit = [&](auto ct) {
        double val = decrypt_val(ct);
        return (val < -AND_THRESHOLD) ? 0 : 1;
    };

    std::cout << "NAND CHAIN TEST:\n";
    std::cout << "================\n\n";

    // Initial: NAND(1,1) = 0
    auto current = eval_nand(ct_1, ct_1);
    int prev_bit = 0;

    std::cout << "  Layer 0: value=" << (decrypt_val(current) * 180.0 / PI)
              << "° bit=" << prev_bit
              << " level=" << current->GetLevel() << "\n";

    int steps = 20;
    int errors = 0;

    for (int step = 1; step <= steps; step++) {
        double val = decrypt_val(current);
        int bit = decrypt_bit(current);
        int level = current->GetLevel();

        if (bit != prev_bit) errors++;

        std::cout << "  Layer " << step << ": value="
                  << (val * 180.0 / PI)
                  << "° bit=" << bit
                  << " level=" << level
                  << (bit == prev_bit ? " ✓" : " ✗") << "\n";

        prev_bit = bit;
        current = eval_nand(current, current);
    }

    std::cout << "\n========================================\n";
    std::cout << "  Steps: " << steps << "\n";
    std::cout << "  Errors: " << errors << "\n";
    std::cout << "  Level: " << current->GetLevel() << "\n";
    std::cout << "  Status: " << (errors == 0 ? "✅ LEVEL 0 DEPTH SCALING!" : "⚠️ MAY ERRORS") << "\n";
    std::cout << "========================================\n";

    return 0;
}
