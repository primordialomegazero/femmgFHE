// PENTAGONAL 1K LIGHT — Pure Homomorphic Evaluation
// Walang decrypt sa bawat gate, check lang sa dulo
// Mas mabilis, mas magaan

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
    std::cout << "  PENTAGONAL 1K LIGHT\n";
    std::cout << "  Pure Homomorphic, Check sa Dulo\n";
    std::cout << "========================================\n\n";

    const double PI = 3.14159265358979323846;
    const double TWO_PI = 2 * PI;
    const double GOLDEN_ANGLE = TWO_PI * (1.0 - 1.0 / 1.6180339887498948482);
    const double ENC_0 = -2 * PI / 5;
    const double ENC_1 = 2 * PI / 5;

    CCParams<CryptoContextCKKSRNS> params;
    params.SetMultiplicativeDepth(50);
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

    // NAND sa pentagonal space
    auto eval_nand = [&](auto a, auto b) {
        auto sum = cc->EvalAdd(a, b);
        auto diff = cc->EvalSub(ct_golden_angle, sum);
        return cc->EvalMult(diff, ct_inv_golden);
    };

    std::cout << "PURE HOMOMORPHIC CHAIN (1000 gates):\n";
    std::cout << "====================================\n\n";

    // Initial: NAND(1,1)
    auto state = eval_nand(ct_enc1, ct_enc1);
    
    auto start = high_resolution_clock::now();

    // Pure homomorphic evaluation — walang decrypt, walang check
    for (int i = 1; i <= 1000; i++) {
        state = eval_nand(state, state);
    }

    auto end = high_resolution_clock::now();
    auto duration = duration_cast<seconds>(end - start).count();

    // I-check lang sa dulo
    double final_val = decrypt_val(state);
    int final_bit = (final_val > 0.5) ? 1 : 0;
    int final_level = state->GetLevel();

    std::cout << "  Gates: 1000\n";
    std::cout << "  Time: " << duration << " seconds\n";
    std::cout << "  Final Value: " << final_val << "\n";
    std::cout << "  Final Bit: " << final_bit << "\n";
    std::cout << "  Final Level: " << final_level << "\n";
    std::cout << "  Status: " << (final_val > -1 && final_val < 3 ? "✅ BOUNDED!" : "⚠️ MAAYING EXPLODED") << "\n";

    return 0;
}
