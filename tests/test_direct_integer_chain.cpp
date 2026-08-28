// DIRECT INTEGER CHAIN — LEVEL 0
// NAND = 2 - (a+b)
// Walang multiplication, walang bootstrapping
// Subukan kung kaya ng malalim na chain

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  DIRECT INTEGER CHAIN — LEVEL 0\n";
    std::cout << "  NAND = 2 - (a+b)\n";
    std::cout << "========================================\n\n";

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

    auto ct_zero = make_ct(0.0);
    auto ct_one = make_ct(1.0);
    auto ct_two = make_ct(2.0);

    auto eval_nand = [&](auto a, auto b) {
        auto sum = cc->EvalAdd(a, b);
        return cc->EvalSub(ct_two, sum);
    };

    auto decrypt_bit = [&](auto ct) {
        double val = decrypt_val(ct);
        return (val > 0.5) ? 1 : 0;
    };

    std::cout << "CHAIN TEST (100 layers):\n";
    std::cout << "========================\n\n";

    // Initial: NAND(1,1) = 0
    auto state = eval_nand(ct_one, ct_one);
    int expected_bit = 0;
    int errors = 0;

    for (int layer = 1; layer <= 100; layer++) {
        double val = decrypt_val(state);
        int bit = decrypt_bit(state);
        int level = state->GetLevel();

        if (bit != expected_bit) {
            errors++;
            if (errors <= 3) {
                std::cout << "  Layer " << layer << ": bit=" << bit
                          << " expected=" << expected_bit
                          << " value=" << val << " ✗\n";
            }
        }

        if (layer <= 5 || layer >= 95) {
            std::cout << "  Layer " << layer << ": value=" << val
                      << " bit=" << bit
                      << " level=" << level
                      << (bit == expected_bit ? " ✓" : " ✗") << "\n";
        }

        // Map pabalik sa integer space
        auto next_input = (bit == 1) ? ct_one : ct_zero;
        state = eval_nand(next_input, next_input);
        expected_bit = 1 - expected_bit;
    }

    std::cout << "\n========================================\n";
    std::cout << "  Layers: 100\n";
    std::cout << "  Errors: " << errors << "\n";
    std::cout << "  Final Level: " << state->GetLevel() << "\n";
    std::cout << "  Status: " << (errors == 0 ? "✅ LEVEL 0 CHAIN!" : "⚠️ MAY ERRORS") << "\n";
    std::cout << "========================================\n";

    return 0;
}
