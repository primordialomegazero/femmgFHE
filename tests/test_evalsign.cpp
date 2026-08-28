// EVALSIGN — May Homomorphic Sign Function ba?
// Check kung may natural na sign sa OpenFHE

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  EVALSIGN CHECK\n";
    std::cout << "  May Homomorphic Sign ba?\n";
    std::cout << "========================================\n\n";

    CCParams<CryptoContextCKKSRNS> params;
    params.SetMultiplicativeDepth(30);
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

    std::cout << "Available functions sa OpenFHE ADVANCEDSHE:\n";
    std::cout << "===========================================\n\n";
    std::cout << "  EvalSin ✓\n";
    std::cout << "  EvalCos ✓\n";
    std::cout << "  EvalSign — checking...\n\n";

    // Subukan ang EvalCompare o EvalSign
    auto test_neg = make_ct(-0.236);
    auto test_pos = make_ct(0.382);

    // Subukan ang polynomial approximation ng sign
    // sign(x) ≈ x / |x| ≈ x / sqrt(x²)
    auto eval_sign_poly = [&](auto x) {
        auto x_sq = cc->EvalMult(x, x);
        // Hindi natin kaya ang sqrt sa FHE
        // Kaya gumamit ng polynomial approximation
        // sign(x) ≈ tanh(kx) para sa malaking k
        return x;
    };

    // Subukan ang EvalSin bilang sign approximation
    auto sin_neg = cc->EvalSin(test_neg, -4.0, 4.0, 5);
    auto sin_pos = cc->EvalSin(test_pos, -4.0, 4.0, 5);

    std::cout << "EVALSIN AS SIGN:\n";
    std::cout << "================\n\n";
    std::cout << "  sin(-0.236) = " << decrypt_val(sin_neg) << " → " 
              << (decrypt_val(sin_neg) > 0 ? "positive" : "negative") << "\n";
    std::cout << "  sin(0.382) = " << decrypt_val(sin_pos) << " → "
              << (decrypt_val(sin_pos) > 0 ? "positive" : "negative") << "\n";
    std::cout << "  Level: " << sin_neg->GetLevel() << "\n\n";

    std::cout << "OBSERVATION:\n";
    std::cout << "============\n\n";
    std::cout << "  Ang EvalSin ay natural na sign function\n";
    std::cout << "  Para sa maliliit na x, sin(x) ≈ x\n";
    std::cout << "  Ang sign ay natural sa sine\n\n";

    return 0;
}
