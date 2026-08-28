// NON-DUAL REALITY — Walang Decrypt, Walang Threshold
// Ang state at ang computation ay iisa
// φ equation bilang natural na reality

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
    std::cout << "  NON-DUAL REALITY\n";
    std::cout << "  Walang Decrypt, Walang Threshold\n";
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

    // Non-dual: ang state ay nasa φ-space na agad
    // Hindi na kailangan ng conversion
    // Ang φ at ψ ay iisang realidad
    
    auto ct_phi = make_ct(PHI);
    auto ct_psi = make_ct(PSI);

    // Non-dual NAND: ang output ay natural na φ o ψ
    // Hindi na kailangan ng threshold
    // Ang equation mismo ang nagde-decide
    
    // Ang susi: (x - φ)(x - ψ) = x² - x - 1
    // Kung x = φ → 0
    // Kung x = ψ → 0
    // Ito ay natural na zero-crossing para sa parehong states
    
    auto eval_reality = [&](auto x) {
        // x² - x - 1 — ang φ equation
        auto x_sq = cc->EvalMult(x, x);
        auto x_sq_minus_x = cc->EvalSub(x_sq, x);
        return cc->EvalSub(x_sq_minus_x, make_ct(1.0));
    };

    std::cout << "NON-DUAL TEST:\n";
    std::cout << "==============\n\n";

    auto test_phi = eval_reality(ct_phi);
    auto test_psi = eval_reality(ct_psi);

    std::cout << "  reality(φ) = " << decrypt_val(test_phi) << " (dapat 0)\n";
    std::cout << "  reality(ψ) = " << decrypt_val(test_psi) << " (dapat 0)\n";
    std::cout << "  Level: " << test_phi->GetLevel() << "\n\n";

    // Ang non-dual na NAND: 
    // Ang state ay φ kung 1, ψ kung 0
    // NAND(φ,φ) = ψ
    // NAND(φ,ψ) = φ
    // NAND(ψ,ψ) = φ
    
    // Sa non-dual: ang equation mismo ang nagre-resolve
    // f(x) = x - (x² - x - 1) = -x² + 2x + 1
    
    auto eval_non_dual_nand = [&](auto a, auto b) {
        // Ang non-dual na paraan: gamitin ang φ equation mismo
        auto sum = cc->EvalAdd(a, b);
        auto sum_sq = cc->EvalMult(sum, sum);
        auto sum_sq_minus_sum = cc->EvalSub(sum_sq, sum);
        auto reality = cc->EvalSub(sum_sq_minus_sum, make_ct(1.0));
        // I-subtract ang reality para bumalik sa φ-space
        return cc->EvalSub(sum, reality);
    };

    std::cout << "NON-DUAL NAND CHAIN:\n";
    std::cout << "====================\n\n";

    auto state = eval_non_dual_nand(ct_phi, ct_phi);
    auto start = high_resolution_clock::now();

    for (int i = 0; i < 20; i++) {
        state = eval_non_dual_nand(state, state);
        double val = decrypt_val(state);
        
        if (i < 5 || i >= 17) {
            std::cout << "  Gate " << i << ": value=" << val
                      << " level=" << state->GetLevel() << "\n";
        }
    }

    auto end = high_resolution_clock::now();
    auto duration = duration_cast<seconds>(end - start).count();

    std::cout << "\n  Time: " << duration << " seconds\n";

    return 0;
}
