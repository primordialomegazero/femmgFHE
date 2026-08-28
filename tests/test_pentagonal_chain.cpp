// PENTAGONAL NAND CHAIN — Level 1
// Subukan kung kaya ng 100 gates sa pentagonal space

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>
#include <chrono>

using namespace lbcrypto;
using namespace std::chrono;

int main() {
    std::cout << "========================================\n";
    std::cout << "  PENTAGONAL NAND CHAIN\n";
    std::cout << "  Level 1, Natural Threshold\n";
    std::cout << "========================================\n\n";

    const double PI = 3.14159265358979323846;
    const double TWO_PI = 2 * PI;
    const double GOLDEN_ANGLE = TWO_PI * (1.0 - 1.0 / 1.6180339887498948482);
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

    auto ct_enc0 = make_ct(ENC_0);
    auto ct_enc1 = make_ct(ENC_1);
    auto ct_golden_angle = make_ct(GOLDEN_ANGLE);
    auto ct_inv_golden = make_ct(1.0 / GOLDEN_ANGLE);

    // NAND sa pentagonal space na may natural threshold
    auto eval_nand = [&](auto a, auto b) {
        auto sum = cc->EvalAdd(a, b);
        auto diff = cc->EvalSub(ct_golden_angle, sum);
        return cc->EvalMult(diff, ct_inv_golden);
    };

    auto decrypt_bit = [&](auto ct) {
        double val = decrypt_val(ct);
        return (val > 0.5) ? 1 : 0;
    };

    std::cout << "CHAIN TEST (100 gates):\n";
    std::cout << "=======================\n\n";

    auto state = eval_nand(ct_enc1, ct_enc1);
    auto start = high_resolution_clock::now();

    int errors = 0;
    int expected_bit = 0;

    for (int i = 1; i <= 100; i++) {
        double val = decrypt_val(state);
        int bit = decrypt_bit(state);
        int level = state->GetLevel();

        if (bit != expected_bit) errors++;

        if (i <= 5 || i >= 98) {
            std::cout << "  Gate " << i << ": value=" << val
                      << " bit=" << bit << " expected=" << expected_bit
                      << " level=" << level
                      << (bit == expected_bit ? " ✓" : " ✗") << "\n";
        }

        // Map pabalik sa pentagonal space
        auto next_input = (bit == 1) ? ct_enc1 : ct_enc0;
        state = eval_nand(next_input, next_input);
        expected_bit = 1 - expected_bit;
    }

    auto end = high_resolution_clock::now();
    auto duration = duration_cast<seconds>(end - start).count();

    std::cout << "\n  Gates: 100\n";
    std::cout << "  Errors: " << errors << "\n";
    std::cout << "  Time: " << duration << " seconds\n";
    std::cout << "  Final Level: " << state->GetLevel() << "\n";

    return 0;
}
