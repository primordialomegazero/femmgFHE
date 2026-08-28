// SIN NORMALIZED CHAIN — EvalSin Bilang Natural Modulo
// NAND = φ² - (a+b) - φ
// Normalize = EvalSin(x)
// Subukan kung bounded ang chain

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>
#include <chrono>

using namespace lbcrypto;
using namespace std::chrono;

int main() {
    std::cout << "========================================\n";
    std::cout << "  SIN NORMALIZED CHAIN\n";
    std::cout << "  EvalSin Bilang Natural Modulo\n";
    std::cout << "========================================\n\n";

    const double PHI = 1.6180339887498948482;
    const double PHI_SQ = PHI * PHI;
    const double PHI_INV = 1.0 / PHI;

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

    auto ct_0 = make_ct(0.0);
    auto ct_1 = make_ct(PHI_INV);
    auto ct_one = make_ct(1.0);

    // NAND raw: 1 - (a+b)
    auto eval_nand_raw = [&](auto a, auto b) {
        auto sum = cc->EvalAdd(a, b);
        return cc->EvalSub(ct_one, sum);
    };

    // Normalize: EvalSin para sa bounded [-1,1]
    auto eval_normalize = [&](auto x) {
        return cc->EvalSin(x, -4.0, 4.0, 5);
    };

    std::cout << "SIN NORMALIZED CHAIN (100 gates):\n";
    std::cout << "=================================\n\n";

    auto state = eval_nand_raw(ct_1, ct_1);
    auto start = high_resolution_clock::now();

    for (int i = 0; i < 100; i++) {
        state = eval_normalize(state);
        state = eval_nand_raw(state, state);
        
        if (i < 5 || i >= 95) {
            double val = decrypt_val(state);
            std::cout << "  Gate " << i << ": value=" << val
                      << " level=" << state->GetLevel() << "\n";
        }
    }

    auto end = high_resolution_clock::now();
    auto duration = duration_cast<seconds>(end - start).count();

    double final_val = decrypt_val(state);
    std::cout << "\n  Final Value: " << final_val << "\n";
    std::cout << "  Final Level: " << state->GetLevel() << "\n";
    std::cout << "  Bounded: " << (std::abs(final_val) < 2.0 ? "✅" : "⚠️") << "\n";
    std::cout << "  Time: " << duration << " seconds\n";

    return 0;
}
