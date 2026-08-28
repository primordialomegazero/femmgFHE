// UNIVERSAL SHIFT FINDER — Lahat ng Gates sa Isang Shift
// Pentagonal ±2π/5, threshold 0.0
// Hanapin ang isang shift na nagbibigay 20/20

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  UNIVERSAL SHIFT FINDER\n";
    std::cout << "  Pentagonal Encoding ±2π/5\n";
    std::cout << "========================================\n\n";

    const double PI = 3.14159265358979323846;
    const double ENC_0 = -2 * PI / 5;
    const double ENC_1 = 2 * PI / 5;
    const double THRESHOLD = 0.0;  // Mas mahigpit

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

    auto eval_gate = [&](auto a, auto b, double shift) {
        auto sum = cc->EvalAdd(a, b);
        auto shifted = cc->EvalAdd(sum, make_ct(shift));
        return cc->EvalCos(shifted, -4.0, 4.0, 15);
    };

    auto decrypt_bit = [&](auto ct) {
        double cos_val = decrypt_val(ct);
        return (cos_val > THRESHOLD) ? 1 : 0;
    };

    auto ct_0 = make_ct(ENC_0);
    auto ct_1 = make_ct(ENC_1);

    // Lahat ng gates at expected outputs
    struct Gate {
        std::string name;
        std::vector<int> expected;
    };

    std::vector<Gate> gates = {
        {"XOR",  {0, 1, 1, 0}},
        {"NAND", {1, 1, 1, 0}},
        {"NOR",  {1, 0, 0, 0}},
        {"AND",  {0, 0, 0, 1}},
        {"OR",   {0, 1, 1, 1}}
    };

    // Subukan ang shifts na multiple ng π/5
    std::cout << "Naghahanap ng universal shift...\n\n";
    int found_count = 0;

    for (int k = 0; k < 10; k++) {
        double shift = k * PI / 5;  // 0°, 36°, 72°, 108°, 144°, 180°, ...

        int total_correct = 0;
        for (const auto& gate : gates) {
            std::vector<std::pair<Ciphertext<DCRTPoly>, Ciphertext<DCRTPoly>>> inputs = {
                {ct_0, ct_0}, {ct_0, ct_1}, {ct_1, ct_0}, {ct_1, ct_1}
            };

            for (int i = 0; i < 4; i++) {
                auto ct_result = eval_gate(inputs[i].first, inputs[i].second, shift);
                int got = decrypt_bit(ct_result);
                if (got == gate.expected[i]) {
                    total_correct++;
                }
            }
        }

        if (total_correct == 20) {
            std::cout << "  ✓ Universal shift: " << shift 
                      << " (" << (shift * 180.0 / PI) << "°) — 20/20!\n";
            found_count++;
        } else {
            std::cout << "  shift=" << shift 
                      << " (" << (shift * 180.0 / PI) << "°) — " 
                      << total_correct << "/20\n";
        }
    }

    if (found_count == 0) {
        std::cout << "\n  Walang universal shift sa threshold " << THRESHOLD << "\n";
        std::cout << "  Kailangan i-adjust ang threshold o encoding\n";
    }

    return 0;
}
