// CLAMPED CHAIN — Direct Feedback
// I-clamp agad ang values sa {0,1} pagkatapos bawat gate
// Level-0 NAND + immediate threshold

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
    std::cout << "  CLAMPED CHAIN\n";
    std::cout << "  Direct Feedback sa {0,1}\n";
    std::cout << "========================================\n\n";

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

    auto ct_zero = make_ct(0.0);
    auto ct_one = make_ct(1.0);
    auto ct_two = make_ct(2.0);
    auto ct_half = make_ct(0.5);

    // Homomorphic threshold (level 4)
    auto eval_threshold = [&](auto x) {
        auto shifted = cc->EvalSub(x, ct_half);
        auto x_sq = cc->EvalMult(shifted, shifted);
        auto three_minus_x_sq = cc->EvalSub(make_ct(3.0), x_sq);
        auto x_times = cc->EvalMult(shifted, three_minus_x_sq);
        auto sign_approx = cc->EvalMult(x_times, make_ct(0.5));
        auto half_sign = cc->EvalMult(sign_approx, ct_half);
        return cc->EvalAdd(ct_half, half_sign);
    };

    // Full NAND: Level-0 + threshold
    auto eval_nand = [&](auto a, auto b) {
        auto sum = cc->EvalAdd(a, b);
        auto nand_raw = cc->EvalSub(ct_two, sum);
        return eval_threshold(nand_raw);
    };

    std::cout << "CLAMPED CHAIN TEST (50 gates):\n";
    std::cout << "==============================\n\n";

    auto state = eval_nand(ct_one, ct_one);
    auto start = high_resolution_clock::now();

    for (int i = 1; i <= 50; i++) {
        state = eval_nand(state, state);
        double val = decrypt_val(state);
        
        if (i <= 5 || i >= 48) {
            std::cout << "  Gate " << i << ": value=" << val
                      << " level=" << state->GetLevel() << "\n";
        }
    }

    auto end = high_resolution_clock::now();
    auto duration = duration_cast<seconds>(end - start).count();

    double final_val = decrypt_val(state);
    std::cout << "\n  Final Value: " << final_val << "\n";
    std::cout << "  Final Level: " << state->GetLevel() << "\n";
    std::cout << "  Time: " << duration << " seconds\n";

    return 0;
}
