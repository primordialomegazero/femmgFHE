// OPTIMIZED STRESS TEST — Minimal Decrypt
// I-check lang sa dulo, hindi sa bawat layer
// 1000-gate chain na walang decrypt bottleneck

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
    std::cout << "  OPTIMIZED STRESS TEST\n";
    std::cout << "  Minimal Decrypt, Maximum Speed\n";
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

    // Homomorphic NAND with threshold
    auto eval_nand = [&](auto a, auto b) {
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

    auto decrypt_bit = [&](auto ct) {
        double val = decrypt_val(ct);
        return (val > 0.5) ? 1 : 0;
    };

    // ========================================
    // TEST 1: 1000-GATE CHAIN — Walang Decrypt sa Bawat Layer
    // ========================================
    std::cout << "TEST 1: 1000-GATE CHAIN (Walang Decrypt sa Bawat Layer)\n";
    std::cout << "=====================================================\n\n";

    auto state = eval_nand(ct_one, ct_one);
    auto start = high_resolution_clock::now();

    // Papalitan natin ang decrypt-bit check ng pure homomorphic evaluation
    // Gumamit ng direct feedback na walang decrypt
    for (int i = 1; i <= 1000; i++) {
        // Direct feedback: kung ang state > 0.5, gamitin ct_one, else ct_zero
        // Hindi natin kailangang i-decrypt — gagamitin natin ang state mismo
        state = eval_nand(state, state);
    }

    auto end = high_resolution_clock::now();
    auto duration = duration_cast<seconds>(end - start).count();

    double final_val = decrypt_val(state);
    std::cout << "  Layers: 1000\n";
    std::cout << "  Time: " << duration << " seconds\n";
    std::cout << "  Final Value: " << final_val << "\n";
    std::cout << "  Final Level: " << state->GetLevel() << "\n\n";

    // ========================================
    // TEST 2: PURE HOMOMORPHIC CHAIN — Walang Decrypt
    // ========================================
    std::cout << "TEST 2: PURE HOMOMORPHIC CHAIN\n";
    std::cout << "===============================\n\n";

    auto state2 = eval_nand(ct_zero, ct_zero);
    auto start2 = high_resolution_clock::now();

    for (int i = 1; i <= 100; i++) {
        state2 = eval_nand(state2, state2);
    }

    auto end2 = high_resolution_clock::now();
    auto duration2 = duration_cast<seconds>(end2 - start2).count();

    double final_val2 = decrypt_val(state2);
    std::cout << "  Layers: 100\n";
    std::cout << "  Time: " << duration2 << " seconds\n";
    std::cout << "  Final Value: " << final_val2 << "\n";
    std::cout << "  Final Level: " << state2->GetLevel() << "\n\n";

    // ========================================
    // TEST 3: BATCH PERFORMANCE
    // ========================================
    std::cout << "TEST 3: BATCH PERFORMANCE\n";
    std::cout << "=========================\n\n";

    auto start3 = high_resolution_clock::now();

    for (int i = 0; i < 10; i++) {
        auto nand_result = eval_nand(ct_one, ct_one);
        auto and_result = eval_nand(eval_nand(nand_result, nand_result), 
                                     eval_nand(nand_result, nand_result));
        auto xor_result = cc->EvalMult(cc->EvalSub(ct_one, ct_zero), 
                                       cc->EvalSub(ct_one, ct_zero));
    }

    auto end3 = high_resolution_clock::now();
    auto duration3 = duration_cast<milliseconds>(end3 - start3).count();

    std::cout << "  10 iterations ng mixed gates: " << duration3 << " ms\n\n";

    std::cout << "========================================\n";
    std::cout << "  OPTIMIZED STRESS TEST COMPLETE\n";
    std::cout << "========================================\n";

    return 0;
}
