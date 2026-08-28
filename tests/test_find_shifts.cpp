// BRUTE FORCE SHIFT FINDER — Pentagonal ±2π/5
// Hanapin ang tamang shift per gate para mag-4/4
// Output: shifts para sa XOR, NAND, NOR, AND, OR

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  SHIFT FINDER — Pentagonal Encoding\n";
    std::cout << "  Brute Force 0 to 2π, step 0.01\n";
    std::cout << "========================================\n\n";

    const double PI = 3.14159265358979323846;
    const double ENC_0 = -2 * PI / 5;
    const double ENC_1 = 2 * PI / 5;
    const double THRESHOLD = -0.3;

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

    auto test_shift = [&](double shift, const std::vector<int>& expected) {
        int correct = 0;
        std::vector<std::pair<Ciphertext<DCRTPoly>, Ciphertext<DCRTPoly>>> inputs = {
            {ct_0, ct_0}, {ct_0, ct_1}, {ct_1, ct_0}, {ct_1, ct_1}
        };

        for (int i = 0; i < 4; i++) {
            auto ct_result = eval_gate(inputs[i].first, inputs[i].second, shift);
            int got = decrypt_bit(ct_result);
            if (got == expected[i]) correct++;
        }
        return correct == 4;
    };

    struct GateInfo {
        std::string name;
        std::vector<int> expected;
    };

    std::vector<GateInfo> gates = {
        {"XOR",  {0, 1, 1, 0}},
        {"NAND", {1, 1, 1, 0}},
        {"NOR",  {1, 0, 0, 0}},
        {"AND",  {0, 0, 0, 1}},
        {"OR",   {0, 1, 1, 1}}
    };

    for (const auto& gate : gates) {
        std::cout << gate.name << " valid shifts:\n";
        bool found = false;
        for (double shift = 0; shift < 2 * PI; shift += 0.01) {
            if (test_shift(shift, gate.expected)) {
                std::cout << "  shift=" << shift 
                          << " (" << (shift * 180.0 / PI) << "°)\n";
                found = true;
            }
        }
        if (!found) {
            std::cout << "  (walang nahanap sa threshold " << THRESHOLD << ")\n";
        }
        std::cout << "\n";
    }

    return 0;
}
