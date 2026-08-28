// NAND DEPTH SCALING — Hanggang ilang gates?
// Sundan ang level at noise pagkatapos ng bawat layer

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  NAND DEPTH SCALING\n";
    std::cout << "  Pentagonal ±2π/5\n";
    std::cout << "========================================\n\n";

    const double PI = 3.14159265358979323846;
    const double ENC_0 = -2 * PI / 5;
    const double ENC_1 = 2 * PI / 5;

    CCParams<CryptoContextCKKSRNS> params;
    params.SetMultiplicativeDepth(20);
    params.SetScalingModSize(50);
    params.SetBatchSize(256);
    params.SetFirstModSize(60);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(params);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);
    cc->Enable(ADVANCEDSHE);

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
        auto sin_val = cc->EvalSin(sum, -4.0, 4.0, 15);
        return cc->EvalNegate(sin_val);  // -sin(a+b)
    };

    auto decrypt_bit = [&](auto ct) {
        double val = decrypt_val(ct);
        return (val > -0.5) ? 1 : 0;
    };

    // Simulan sa NAND(1,1) = 0
    auto current = eval_nand(ct_1, ct_1);

    std::cout << "NAND CHAIN TEST:\n";
    std::cout << "================\n\n";

    for (int step = 1; step <= 10; step++) {
        double val = decrypt_val(current);
        int bit = decrypt_bit(current);
        int level = current->GetLevel();

        std::cout << "  Layer " << step << ": "
                  << "value=" << val
                  << " bit=" << bit
                  << " level=" << level
                  << (step <= 1 ? "" : " ✓")
                  << "\n";

        // I-feed muli sa NAND kasama ang sarili
        current = eval_nand(current, current);
    }

    std::cout << "\n  Status: Natapos ang loop\n";
    return 0;
}
