// φ SIGMOID — Natural Normalization
// f(x) = 1/φ / (1 + φ^(-kx))
// Natural na sigmoid na φ-based

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  φ SIGMOID — Natural Normalization\n";
    std::cout << "========================================\n\n";

    const double PHI = 1.6180339887498948482;
    const double PHI_INV = 1.0 / PHI;

    CCParams<CryptoContextCKKSRNS> params;
    params.SetMultiplicativeDepth(10);
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

    // Natural na normalization: x → x / (φ + |x|)
    // Ito ay bounded sa [-1/φ, 1/φ]
    auto eval_normalize = [&](auto x) {
        // Para sa FHE, gamitin ang polynomial approximation
        // f(x) = x / (1 + x²/φ²) — bounded
        auto x_sq = cc->EvalMult(x, x);
        auto phi_sq = make_ct(PHI * PHI);
        auto x_sq_over_phi_sq = cc->EvalMult(x_sq, make_ct(1.0 / (PHI * PHI)));
        auto one_plus = cc->EvalAdd(make_ct(1.0), x_sq_over_phi_sq);
        
        // Hindi natin kaya ang division sa FHE
        // Kaya gamitin ang polynomial approximation
        return x;
    };

    // Simpleng normalization: x → x - φ * floor(x/φ)
    // Para sa bounded range, gamitin ang EvalSin bilang natural na modulo
    auto eval_sin_normalize = [&](auto x) {
        return cc->EvalSin(x, -4.0, 4.0, 5);
    };

    std::cout << "NATURAL NORMALIZATION TEST:\n";
    std::cout << "===========================\n\n";

    // Subukan ang EvalSin bilang natural na modulo
    for (double test_val : {-5.0, -2.0, -0.5, 0.0, 0.5, 2.0, 5.0}) {
        auto ct_val = make_ct(test_val);
        auto normalized = eval_sin_normalize(ct_val);
        std::cout << "  sin(" << test_val << ") = " << decrypt_val(normalized) << "\n";
    }

    std::cout << "\nOBSERVATION:\n";
    std::cout << "============\n\n";
    std::cout << "  Ang EvalSin ay bounded sa [-1, 1]\n";
    std::cout << "  Maaaring ito ang natural na normalization\n";
    std::cout << "  Pero may level cost\n\n";

    return 0;
}
