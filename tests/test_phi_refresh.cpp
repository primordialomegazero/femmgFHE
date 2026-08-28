// φ NATURAL REFRESH
// Hanapin ang property ng φ na natural na nagre-refresh
// φ^0 = 1, φ^1 = φ, φ^2 = φ+1, φ^3 = 2φ+1
// Cassini: F(n-1)F(n+1) - F(n)² = (-1)^n

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  φ NATURAL REFRESH\n";
    std::cout << "  Hidden Properties Hunt\n";
    std::cout << "========================================\n\n";

    const double PHI = 1.6180339887498948482;
    const double PSI = -0.6180339887498948482;  // conjugate

    CCParams<CryptoContextCKKSRNS> params;
    params.SetMultiplicativeDepth(2);
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

    std::cout << "φ HIDDEN PROPERTIES:\n";
    std::cout << "====================\n\n";

    // 1. φ + ψ = 1 (natural complement)
    std::cout << "1. φ + ψ = 1\n";
    std::cout << "   " << PHI << " + " << PSI << " = " << (PHI + PSI) << "\n\n";

    // 2. φ^n + ψ^n = Lucas numbers
    std::cout << "2. Lucas numbers (L(n) = φ^n + ψ^n):\n";
    for (int n = 0; n <= 6; n++) {
        double lucas = std::pow(PHI, n) + std::pow(PSI, n);
        std::cout << "   L(" << n << ") = " << lucas << "\n";
    }
    std::cout << "\n";

    // 3. Cassini identity: F(n-1)F(n+1) - F(n)² = (-1)^n
    std::cout << "3. Cassini Identity — Natural ±1 oscillation:\n";
    long long F[10];
    F[0] = 0; F[1] = 1;
    for (int i = 2; i < 10; i++) F[i] = F[i-1] + F[i-2];
    for (int n = 1; n < 8; n++) {
        long long cassini = F[n-1] * F[n+1] - F[n] * F[n];
        std::cout << "   F(" << (n-1) << ")F(" << (n+1) << ") - F(" << n << ")² = "
                  << cassini << " = " << ((n % 2 == 0) ? 1 : -1) << "\n";
    }
    std::cout << "\n";

    // 4. Natural refresh: φ² - φ = 1
    std::cout << "4. Natural refresh candidates:\n";
    std::cout << "   φ² - φ = " << (PHI * PHI - PHI) << " (refresh to 1)\n";
    std::cout << "   φ - 1/φ = " << (PHI - 1.0/PHI) << " (refresh to 1)\n";
    std::cout << "   φ² - φ - 1 = " << (PHI * PHI - PHI - 1) << " (refresh to 0)\n\n";

    // 5. Test: f(x) = x - (x² - x - 1) = -x² + 2x + 1
    auto eval_refresh = [&](auto x) {
        auto x_sq = cc->EvalMult(x, x);
        auto neg_x_sq = cc->EvalNegate(x_sq);
        auto two_x = cc->EvalAdd(x, x);
        auto result = cc->EvalAdd(neg_x_sq, two_x);
        return cc->EvalAdd(result, make_ct(1.0));
    };

    std::cout << "5. REFRESH TEST: f(x) = -x² + 2x + 1\n";
    std::cout << "   (Ito ay 1 - (x² - x - 1) = 1 - polynomial)\n\n";

    for (double test_val : {-5.0, -3.0, -1.0, 0.0, 0.5, 1.0, 1.5, 2.0, 3.0, 5.0}) {
        auto ct_val = make_ct(test_val);
        auto result = eval_refresh(ct_val);
        double refreshed = decrypt_val(result);
        std::cout << "   f(" << test_val << ") = " << refreshed << "\n";
    }

    return 0;
}
