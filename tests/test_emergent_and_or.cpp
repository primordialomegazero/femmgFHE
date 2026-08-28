// EMERGENT AND/OR — Mula sa Sin Sign
// Pentagonal ±2π/5
// AND = positive sin
// OR  = positive cos o positive sin
// Walang bootstrapping, natural thresholds

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  EMERGENT AND/OR — Natural Sin Sign\n";
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

    auto eval_phase = [&](auto a, auto b) {
        auto sum = cc->EvalAdd(a, b);
        auto cos_val = cc->EvalCos(sum, -4.0, 4.0, 15);
        auto sin_val = cc->EvalSin(sum, -4.0, 4.0, 15);
        return std::make_pair(cos_val, sin_val);
    };

    auto decrypt_bit = [&](auto ct, double threshold) {
        double val = decrypt_val(ct);
        return (val > threshold) ? 1 : 0;
    };

    std::cout << "EMERGENT GATE RESULTS:\n";
    std::cout << "=======================\n\n";

    std::vector<std::pair<std::pair<int,int>, std::pair<Ciphertext<DCRTPoly>, Ciphertext<DCRTPoly>>>> tests = {
        {{0,0}, {ct_0, ct_0}},
        {{0,1}, {ct_0, ct_1}},
        {{1,0}, {ct_1, ct_0}},
        {{1,1}, {ct_1, ct_1}}
    };

    int and_correct = 0;
    int or_correct = 0;
    int xor_correct = 0;

    for (auto& [bits, cts] : tests) {
        auto [a_bit, b_bit] = bits;
        auto [ct_a, ct_b] = cts;

        auto [ct_cos, ct_sin] = eval_phase(ct_a, ct_b);
        double cos_val = decrypt_val(ct_cos);
        double sin_val = decrypt_val(ct_sin);

        // Emergent gates
        int xor_got = (cos_val < 0) ? 0 : 1;
        int and_got = (sin_val > 0.5) ? 1 : 0;
        int or_got = (sin_val > -0.5 && cos_val > -0.5) ? 1 : 0;

        int xor_exp = a_bit ^ b_bit;
        int and_exp = a_bit & b_bit;
        int or_exp = a_bit | b_bit;

        if (xor_got == xor_exp) xor_correct++;
        if (and_got == and_exp) and_correct++;
        if (or_got == or_exp) or_correct++;

        std::cout << "  (" << a_bit << "," << b_bit << ") -> "
                  << "cos=" << cos_val << " sin=" << sin_val << "\n";
        std::cout << "    XOR: " << xor_got << "/" << xor_exp
                  << "  AND: " << and_got << "/" << and_exp
                  << "  OR: " << or_got << "/" << or_exp << "\n\n";
    }

    std::cout << "========================================\n";
    std::cout << "  XOR: " << xor_correct << "/4\n";
    std::cout << "  AND: " << and_correct << "/4\n";
    std::cout << "  OR: " << or_correct << "/4\n";
    std::cout << "========================================\n";

    return 0;
}
