// EMERGENT THRESHOLD — Pentagonal Natural Thresholds
// Subukan ang natural thresholds: cos(36°), cos(72°), cos(108°), cos(144°)
// Tingnan kung anong gate ang lumalabas sa bawat threshold

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  EMERGENT THRESHOLD FINDER\n";
    std::cout << "  Pentagonal ±2π/5, Walang Shift\n";
    std::cout << "========================================\n\n";

    const double PI = 3.14159265358979323846;
    const double ENC_0 = -2 * PI / 5;
    const double ENC_1 = 2 * PI / 5;

    // Natural thresholds mula sa pentagon
    std::vector<double> thresholds = {
        std::cos(PI / 5),     // cos(36°) = 0.809
        std::cos(2 * PI / 5), // cos(72°) = 0.309
        std::cos(3 * PI / 5), // cos(108°) = -0.309
        std::cos(4 * PI / 5), // cos(144°) = -0.809
        0.0,                  // Zero
        -0.5                  // Midpoint
    };

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

    auto eval_gate = [&](auto a, auto b) {
        auto sum = cc->EvalAdd(a, b);
        return cc->EvalCos(sum, -4.0, 4.0, 15);
    };

    auto decrypt_bit = [&](auto ct, double threshold) {
        double cos_val = decrypt_val(ct);
        return (cos_val > threshold) ? 1 : 0;
    };

    // Raw cosine values
    std::cout << "RAW COSINE VALUES (walang shift):\n";
    std::cout << "=================================\n";
    std::vector<std::pair<Ciphertext<DCRTPoly>, Ciphertext<DCRTPoly>>> inputs = {
        {ct_0, ct_0}, {ct_0, ct_1}, {ct_1, ct_0}, {ct_1, ct_1}
    };
    std::vector<double> raw_cos;
    for (auto& [a, b] : inputs) {
        auto result = eval_gate(a, b);
        double cos_val = decrypt_val(result);
        raw_cos.push_back(cos_val);
        std::cout << "  cos(" << (decrypt_val(a) * 180.0 / PI) << "° + "
                  << (decrypt_val(b) * 180.0 / PI) << "°) = " << cos_val << "\n";
    }

    std::cout << "\nGATE PATTERNS SA BAWAT THRESHOLD:\n";
    std::cout << "==================================\n\n";

    for (double threshold : thresholds) {
        std::vector<int> pattern;
        for (double cos_val : raw_cos) {
            pattern.push_back(cos_val > threshold ? 1 : 0);
        }

        std::cout << "Threshold " << threshold << ":\n";
        std::cout << "  Pattern: (" << pattern[0] << "," << pattern[1] << ","
                  << pattern[2] << "," << pattern[3] << ") → ";

        if (pattern == std::vector<int>{0,1,1,0}) {
            std::cout << "XOR\n";
        } else if (pattern == std::vector<int>{1,1,1,0}) {
            std::cout << "NAND\n";
        } else if (pattern == std::vector<int>{1,0,0,0}) {
            std::cout << "NOR\n";
        } else if (pattern == std::vector<int>{0,0,0,1}) {
            std::cout << "AND\n";
        } else if (pattern == std::vector<int>{0,1,1,1}) {
            std::cout << "OR\n";
        } else {
            std::cout << "Iba pa\n";
        }
    }

    return 0;
}
