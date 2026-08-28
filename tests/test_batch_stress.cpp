// BATCH STRESS TEST — Grouped Gates
// I-batch ang gates para ma-optimize ang level usage
// Level 0 NAND + periodic threshold

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
    std::cout << "  BATCH STRESS TEST\n";
    std::cout << "  Grouped Gates, Optimized Level\n";
    std::cout << "========================================\n\n";

    CCParams<CryptoContextCKKSRNS> params;
    params.SetMultiplicativeDepth(40);
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

    auto ct_zero = make_ct(0.0);
    auto ct_one = make_ct(1.0);
    auto ct_two = make_ct(2.0);
    auto ct_half = make_ct(0.5);

    // Level-0 NAND (walang threshold)
    auto eval_nand_level0 = [&](auto a, auto b) {
        auto sum = cc->EvalAdd(a, b);
        return cc->EvalSub(ct_two, sum);
    };

    // Level-4 NAND (may threshold)
    auto eval_nand_level4 = [&](auto a, auto b) {
        auto sum = cc->EvalAdd(a, b);
        auto nand_raw = cc->EvalSub(ct_two, sum);
        
        auto shifted = cc->EvalSub(nand_raw, ct_half);
        auto x_sq = cc->EvalMult(shifted, shifted);
        auto three_minus_x_sq = cc->EvalSub(make_ct(3.0), x_sq);
        auto x_times = cc->EvalMult(shifted, three_minus_x_sq);
        auto sign_approx = cc->EvalMult(x_times, make_ct(0.5));
        auto half_sign = cc->EvalMult(sign_approx, ct_half);
        return cc->EvalAdd(ct_half, half_sign);
    };

    std::cout << "TEST: LEVEL-0 CHAIN + PERIODIC THRESHOLD\n";
    std::cout << "========================================\n\n";

    // Mag-chain gamit ang level-0 NAND
    // I-threshold lamang tuwing 10 gates
    auto state = eval_nand_level0(ct_one, ct_one);
    auto start = high_resolution_clock::now();

    for (int batch = 0; batch < 10; batch++) {
        // 10 level-0 NAND gates
        for (int i = 0; i < 10; i++) {
            state = eval_nand_level0(state, state);
        }
        
        // I-threshold ang output
        state = eval_nand_level4(state, state);
    }

    auto end = high_resolution_clock::now();
    auto duration = duration_cast<seconds>(end - start).count();

    double final_val = decrypt_val(state);
    std::cout << "  Total gates: " << (10 * 10 + 10) << "\n";
    std::cout << "  Time: " << duration << " seconds\n";
    std::cout << "  Final Value: " << final_val << "\n";
    std::cout << "  Final Level: " << state->GetLevel() << "\n";

    return 0;
}
