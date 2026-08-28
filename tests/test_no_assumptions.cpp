// NO ASSUMPTIONS — Pure φ Self-Reference
// Walang threshold, walang binary, walang NAND
// Ang φ equation lang ang nagko-compute

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  NO ASSUMPTIONS\n";
    std::cout << "  Pure φ Self-Reference\n";
    std::cout << "========================================\n\n";

    const double PHI = 1.6180339887498948482;
    const double PHI_SQ = PHI * PHI;

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

    auto ct_phi = make_ct(PHI);
    auto ct_phi_sq = make_ct(PHI_SQ);

    // Pure φ self-reference: f(x) = x - φ * (x² - x - 1)
    // Walang threshold, walang decision — ang equation lang
    auto eval_phi_self = [&](auto x) {
        auto x_sq = cc->EvalMult(x, x);
        auto x_sq_minus_x = cc->EvalSub(x_sq, x);
        auto poly = cc->EvalSub(x_sq_minus_x, make_ct(1.0));
        auto phi_times_poly = cc->EvalMult(ct_phi, poly);
        return cc->EvalSub(x, phi_times_poly);
    };

    std::cout << "PURE φ SELF-REFERENCE TEST:\n";
    std::cout << "===========================\n\n";

    // Subukan sa iba't ibang initial values
    for (double init : {-2.0, -1.0, -0.5, 0.0, 0.5, 1.0, 2.0}) {
        auto state = make_ct(init);
        std::cout << "  Initial: " << init << "\n";
        
        for (int i = 0; i < 5; i++) {
            state = eval_phi_self(state);
            double val = decrypt_val(state);
            std::cout << "    Step " << i << ": " << val << "\n";
        }
        std::cout << "\n";
    }

    return 0;
}
