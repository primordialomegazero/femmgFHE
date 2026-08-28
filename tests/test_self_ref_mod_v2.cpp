// SELF-REFERENTIAL MODULO V2
// May sapat na multiplicative depth
// x - φ²(x² - x - 1) bilang natural reduction

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  SELF-REFERENTIAL MODULO V2\n";
    std::cout << "  Polynomial Reduction\n";
    std::cout << "========================================\n\n";

    const double PHI = 1.6180339887498948482;
    const double PHI_SQ = PHI * PHI;

    CCParams<CryptoContextCKKSRNS> params;
    params.SetMultiplicativeDepth(5);
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

    auto ct_phi_sq = make_ct(PHI_SQ);

    // Self-referential modulo: f(x) = x - φ²(x² - x - 1)
    auto eval_self_mod = [&](auto x) {
        auto x_sq = cc->EvalMult(x, x);           // x²
        auto x_sq_minus_x = cc->EvalSub(x_sq, x);  // x² - x
        auto poly = cc->EvalSub(x_sq_minus_x, make_ct(1.0));  // x² - x - 1
        
        auto phi_sq_times_poly = cc->EvalMult(ct_phi_sq, poly);  // φ² * poly
        return cc->EvalSub(x, phi_sq_times_poly);  // x - φ² * poly
    };

    std::cout << "SELF-REFERENTIAL MODULO TEST:\n";
    std::cout << "=============================\n\n";
    std::cout << "  f(x) = x - φ²(x² - x - 1)\n\n";

    for (double test_val : {-10.0, -5.0, -2.0, -1.0, 0.0, 1.0, 2.0, 5.0, 10.0}) {
        auto ct_val = make_ct(test_val);
        auto result = eval_self_mod(ct_val);
        double modded = decrypt_val(result);
        
        std::cout << "  f(" << test_val << ") = " << modded << "\n";
    }

    std::cout << "\nOBSERVATION:\n";
    std::cout << "============\n\n";
    std::cout << "  Kung f(x) ay bounded sa [-2, 3], ito ay promising\n";
    std::cout << "  Kung hindi, kailangan ng ibang approach\n";

    return 0;
}
