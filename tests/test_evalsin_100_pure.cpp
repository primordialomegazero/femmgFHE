// EVALSIN 100 GATES — Pure Evaluation
// Walang decrypt sa bawat gate
// Check lang sa dulo kung stable ang level

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
    std::cout << "  EVALSIN 100 GATES — PURE\n";
    std::cout << "  Walang Decrypt sa Bawat Gate\n";
    std::cout << "========================================\n\n";

    const double PI = 3.14159265358979323846;
    const double ENC_0 = -2 * PI / 5;
    const double ENC_1 = 2 * PI / 5;

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
    auto ct_two_penta = make_ct(4 * PI / 5);

    // NAND: EvalSin(144° - (a+b))
    auto eval_nand = [&](auto a, auto b) {
        auto sum = cc->EvalAdd(a, b);
        auto nand_raw = cc->EvalSub(ct_two_penta, sum);
        return cc->EvalSin(nand_raw, -4.0, 4.0, 5);
    };

    std::cout << "PURE EVALUATION (100 gates):\n";
    std::cout << "============================\n\n";

    auto state = eval_nand(ct_enc1, ct_enc1);
    auto start = high_resolution_clock::now();

    std::cout << "  Initial Level: " << state->GetLevel() << "\n";
    std::cout << "  Nag-e-evaluate...\n\n";

    // Pure evaluation — walang decrypt, walang bit check
    for (int i = 1; i <= 100; i++) {
        state = eval_nand(state, state);
    }

    auto end = high_resolution_clock::now();
    auto duration = duration_cast<seconds>(end - start).count();

    double final_val = decrypt_val(state);
    int final_level = state->GetLevel();

    std::cout << "RESULT:\n";
    std::cout << "=======\n\n";
    std::cout << "  Gates: 100\n";
    std::cout << "  Time: " << duration << " seconds\n";
    std::cout << "  Final Value: " << final_val << "\n";
    std::cout << "  Final Level: " << final_level << "\n";
    std::cout << "  Bounded: " << (std::abs(final_val) < 2.0 ? "✅ YES" : "⚠️ NO") << "\n";
    std::cout << "  Status: " << (final_level > 0 ? "✅ LEVEL STABLE!" : "⚠️ LEVEL EXHAUSTED") << "\n";

    return 0;
}
