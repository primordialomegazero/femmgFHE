// PHASE STATES — Basahin ang buong angle, hindi lang cosine
// Subukan kung kaya ng EvalSin na makuha ang sign ng angle

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  PHASE STATES — Beyond Cosine\n";
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

    // Compute both cos and sin
    auto eval_cos_sin = [&](auto a, auto b) {
        auto sum = cc->EvalAdd(a, b);
        auto cos_val = cc->EvalCos(sum, -4.0, 4.0, 15);
        auto sin_val = cc->EvalSin(sum, -4.0, 4.0, 15);
        return std::make_pair(cos_val, sin_val);
    };

    std::cout << "FULL PHASE ANALYSIS:\n";
    std::cout << "====================\n\n";

    std::vector<std::pair<Ciphertext<DCRTPoly>, Ciphertext<DCRTPoly>>> inputs = {
        {ct_0, ct_0}, {ct_0, ct_1}, {ct_1, ct_0}, {ct_1, ct_1}
    };

    for (auto& [a, b] : inputs) {
        auto [cos_ct, sin_ct] = eval_cos_sin(a, b);
        double cos_val = decrypt_val(cos_ct);
        double sin_val = decrypt_val(sin_ct);
        double angle = std::atan2(sin_val, cos_val);
        
        std::cout << "  a=" << (decrypt_val(a) * 180.0 / PI) << "° "
                  << "b=" << (decrypt_val(b) * 180.0 / PI) << "°\n";
        std::cout << "    sum=" << (decrypt_val(a) + decrypt_val(b)) * 180.0 / PI << "°\n";
        std::cout << "    cos=" << cos_val << " sin=" << sin_val << "\n";
        std::cout << "    recovered angle=" << angle * 180.0 / PI << "°\n\n";
    }

    return 0;
}
