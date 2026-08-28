// PHI-PSI TOGGLE — Natural Self-Normalizing
// f(x) = 1 - x, state = φ o ψ
// Level 0, bounded, natural sa golden ratio

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <chrono>

using namespace lbcrypto;
using namespace std::chrono;

int main() {
    std::cout << "========================================\n";
    std::cout << "  PHI-PSI TOGGLE\n";
    std::cout << "  Natural Self-Normalizing\n";
    std::cout << "========================================\n\n";

    const double PHI = 1.6180339887498948482;
    const double PSI = -0.6180339887498948482;

    CCParams<CryptoContextCKKSRNS> params;
    params.SetMultiplicativeDepth(1);
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
    auto ct_psi = make_ct(PSI);
    auto ct_one = make_ct(1.0);

    // NAND/Not: f(x) = 1 - x
    auto eval_not = [&](auto x) {
        return cc->EvalSub(ct_one, x);
    };

    std::cout << "PHI-PSI TOGGLE TEST:\n";
    std::cout << "====================\n\n";

    auto state = ct_phi;
    
    for (int i = 0; i < 100; i++) {
        state = eval_not(state);
        double val = decrypt_val(state);
        
        if (i < 5 || i >= 95) {
            std::cout << "  Gate " << i << ": " << val
                      << " level=" << state->GetLevel() << "\n";
        }
    }

    double final_val = decrypt_val(state);
    std::cout << "\n  Final: " << final_val << "\n";
    std::cout << "  Bounded: " << (std::abs(final_val - PHI) < 0.01 || std::abs(final_val - PSI) < 0.01 ? "✅" : "⚠️") << "\n";

    return 0;
}
