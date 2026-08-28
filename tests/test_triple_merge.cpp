// TRIPLE MERGE — NAND + Refresh + Period-0
// NAND = φ² - (a+b) - φ
// Refresh = -x² + 2x + 1
// Period-0 = (x + φ) mod 1
// Lahat pinagsama para sa natural boundedness

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
    std::cout << "  TRIPLE MERGE\n";
    std::cout << "  NAND + Refresh + Period-0\n";
    std::cout << "========================================\n\n";

    const double PHI = 1.6180339887498948482;
    const double PHI_SQ = PHI * PHI;
    const double PHI_INV = 1.0 / PHI;
    const double PHI_MOD = 0.6180339887498949;

    CCParams<CryptoContextCKKSRNS> params;
    params.SetMultiplicativeDepth(20);
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

    auto ct_0 = make_ct(0.0);
    auto ct_1 = make_ct(PHI_INV);
    auto ct_const = make_ct(PHI_SQ - PHI);
    auto ct_phi_mod = make_ct(PHI_MOD);

    // NAND: φ² - (a+b) - φ = 1 - (a+b)
    auto eval_nand = [&](auto a, auto b) {
        auto sum = cc->EvalAdd(a, b);
        return cc->EvalSub(ct_const, sum);
    };

    // Refresh: -x² + 2x + 1
    auto eval_refresh = [&](auto x) {
        auto x_sq = cc->EvalMult(x, x);
        auto neg_x_sq = cc->EvalNegate(x_sq);
        auto two_x = cc->EvalAdd(x, x);
        auto result = cc->EvalAdd(neg_x_sq, two_x);
        return cc->EvalAdd(result, make_ct(1.0));
    };

    // Period-0 rotation: (x + φ) mod 1
    auto period0_rotate = [&](auto x) {
        return cc->EvalAdd(x, ct_phi_mod);
    };

    std::cout << "TRIPLE MERGE TEST (100 gates):\n";
    std::cout << "==============================\n\n";

    auto state = eval_nand(ct_1, ct_1);
    auto start = high_resolution_clock::now();

    for (int i = 0; i < 100; i++) {
        // Period-0 rotation muna
        state = period0_rotate(state);
        
        // Tapos NAND
        state = eval_nand(state, state);
        
        // Tapos refresh
        state = eval_refresh(state);
        
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
    std::cout << "  Bounded: " << (std::abs(final_val) < 10.0 ? "✅" : "⚠️") << "\n";
    std::cout << "  Time: " << duration << " seconds\n";

    return 0;
}
