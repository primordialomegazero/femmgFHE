// EMERGENT RAW — Walang EvalSin/Cos
// Kunin ang gate logic mula sa raw angle sum
// Natural φ thresholds lang

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  EMERGENT RAW — Walang Trig Functions\n";
    std::cout << "  Raw Angle + Natural φ Thresholds\n";
    std::cout << "========================================\n\n";

    const double PI = 3.14159265358979323846;
    const double ENC_0 = -2 * PI / 5;
    const double ENC_1 = 2 * PI / 5;

    // Natural φ-based thresholds
    const double GOLDEN_ANGLE = 2 * PI * (1.0 - 1.0 / 1.618033988749895);
    const double HALF_PENTA = PI / 5;  // 36°

    CCParams<CryptoContextCKKSRNS> params;
    params.SetMultiplicativeDepth(10);
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

    auto eval_sum = [&](auto a, auto b) {
        return cc->EvalAdd(a, b);  // Raw angle sum lang
    };

    auto decrypt_bit = [&](auto ct, double threshold) {
        double val = decrypt_val(ct);
        return (val > threshold) ? 1 : 0;
    };

    std::cout << "RAW ANGLE ANALYSIS:\n";
    std::cout << "===================\n\n";

    std::vector<std::pair<std::pair<int,int>, std::pair<Ciphertext<DCRTPoly>, Ciphertext<DCRTPoly>>>> tests = {
        {{0,0}, {ct_0, ct_0}},
        {{0,1}, {ct_0, ct_1}},
        {{1,0}, {ct_1, ct_0}},
        {{1,1}, {ct_1, ct_1}}
    };

    for (auto& [bits, cts] : tests) {
        auto [a_bit, b_bit] = bits;
        auto [ct_a, ct_b] = cts;

        auto ct_sum = eval_sum(ct_a, ct_b);
        double sum_val = decrypt_val(ct_sum);

        int xor_got = (sum_val > -HALF_PENTA && sum_val < HALF_PENTA) ? 1 : 0;
        int and_got = (sum_val > GOLDEN_ANGLE - PI/10) ? 1 : 0;
        int or_got = (sum_val > -GOLDEN_ANGLE + PI/10) ? 1 : 0;

        int xor_exp = a_bit ^ b_bit;
        int and_exp = a_bit & b_bit;
        int or_exp = a_bit | b_bit;

        std::cout << "  (" << a_bit << "," << b_bit << ") -> sum="
                  << (sum_val * 180.0 / PI) << "°\n";
        std::cout << "    XOR: " << xor_got << "/" << xor_exp
                  << "  AND: " << and_got << "/" << and_exp
                  << "  OR: " << or_got << "/" << or_exp << "\n\n";
    }

    return 0;
}
