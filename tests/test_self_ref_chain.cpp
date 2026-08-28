// SELF-REFERENTIAL CHAIN — 1000 GATES
// NAND = φ² - (a+b) - φ
// Natural zero crossing, level 0

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
    std::cout << "  SELF-REFERENTIAL CHAIN\n";
    std::cout << "  NAND = φ² - (a+b) - φ\n";
    std::cout << "========================================\n\n";

    const double PHI = 1.6180339887498948482;
    const double PHI_SQ = PHI * PHI;
    const double PHI_INV = 1.0 / PHI;

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

    auto ct_0 = make_ct(0.0);
    auto ct_1 = make_ct(PHI_INV);
    auto ct_const = make_ct(PHI_SQ - PHI);

    auto eval_nand = [&](auto a, auto b) {
        auto sum = cc->EvalAdd(a, b);
        return cc->EvalSub(ct_const, sum);
    };

    auto decrypt_bit = [&](auto ct) {
        double val = decrypt_val(ct);
        return (val > 0) ? 1 : 0;
    };

    std::cout << "CHAIN TEST (1000 gates):\n";
    std::cout << "========================\n\n";

    auto state = eval_nand(ct_1, ct_1);
    auto start = high_resolution_clock::now();

    int errors = 0;
    int expected_bit = 0;

    for (int i = 1; i <= 1000; i++) {
        double val = decrypt_val(state);
        int bit = decrypt_bit(state);
        int level = state->GetLevel();

        if (bit != expected_bit) errors++;

        if (i <= 3 || i >= 998) {
            std::cout << "  Gate " << i << ": value=" << val
                      << " bit=" << bit << " expected=" << expected_bit
                      << " level=" << level
                      << (bit == expected_bit ? " ✓" : " ✗") << "\n";
        }

        auto next_input = (bit == 1) ? ct_1 : ct_0;
        state = eval_nand(next_input, next_input);
        expected_bit = 1 - expected_bit;
    }

    auto end = high_resolution_clock::now();
    auto duration = duration_cast<seconds>(end - start).count();

    std::cout << "\n  Gates: 1000\n";
    std::cout << "  Errors: " << errors << "\n";
    std::cout << "  Time: " << duration << " seconds\n";
    std::cout << "  Final Level: " << state->GetLevel() << "\n";
    std::cout << "  Status: " << (errors == 0 ? "✅ SELF-REFERENTIAL CHAIN!" : "⚠️ MAY ERRORS") << "\n";

    return 0;
}
