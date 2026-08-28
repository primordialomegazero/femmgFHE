// EVALSIN CHAIN — Zero Detection sa Bawat Gate
// NAND = EvalSin(2*72° - (a+b))
// Zero → 0, Non-zero → 1
// Subukan kung ilang gates ang kaya

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
    std::cout << "  EVALSIN CHAIN\n";
    std::cout << "  Natural Zero Detection\n";
    std::cout << "========================================\n\n";

    const double PI = 3.14159265358979323846;
    const double PENTA = 2 * PI / 5;  // 72°
    const double ENC_0 = -2 * PI / 5;  // -72°
    const double ENC_1 = 2 * PI / 5;   // +72°

    CCParams<CryptoContextCKKSRNS> params;
    params.SetMultiplicativeDepth(60);
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
    auto ct_two_penta = make_ct(4 * PI / 5);  // 144°

    // NAND: EvalSin(144° - (a+b))
    auto eval_nand = [&](auto a, auto b) {
        auto sum = cc->EvalAdd(a, b);
        auto nand_raw = cc->EvalSub(ct_two_penta, sum);
        return cc->EvalSin(nand_raw, -4.0, 4.0, 5);
    };

    auto decrypt_bit = [&](auto ct) {
        double val = decrypt_val(ct);
        return (std::abs(val) > 0.1) ? 1 : 0;
    };

    std::cout << "CHAIN TEST (20 gates):\n";
    std::cout << "======================\n\n";

    auto state = eval_nand(ct_enc1, ct_enc1);
    auto start = high_resolution_clock::now();

    int errors = 0;
    int expected_bit = 0;

    for (int i = 1; i <= 20; i++) {
        double val = decrypt_val(state);
        int bit = decrypt_bit(state);
        int level = state->GetLevel();

        if (bit != expected_bit) {
            errors++;
            if (errors <= 3) {
                std::cout << "  Gate " << i << ": bit=" << bit
                          << " expected=" << expected_bit
                          << " value=" << val << " ✗\n";
            }
        }

        if (i <= 5 || i >= 18) {
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

    std::cout << "\n  Gates: 20\n";
    std::cout << "  Errors: " << errors << "\n";
    std::cout << "  Time: " << duration << " seconds\n";
    std::cout << "  Final Level: " << state->GetLevel() << "\n";
    std::cout << "  Status: " << (errors == 0 ? "✅ EVALSIN CHAIN!" : "⚠️ MAY ERRORS") << "\n";

    return 0;
}
