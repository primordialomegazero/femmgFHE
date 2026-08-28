// HOMOMORPHIC THRESHOLD V2 — Walang Decrypt sa Gitna
// Threshold > 0.5 gamit ang natural na zero crossing
// NAND = 2 - (a+b) na may step function

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  HOMOMORPHIC THRESHOLD V2\n";
    std::cout << "  Natural Zero Crossing\n";
    std::cout << "========================================\n\n";

    CCParams<CryptoContextCKKSRNS> params;
    params.SetMultiplicativeDepth(20);
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

    // NAND = 2 - (a+b)
    auto eval_nand_raw = [&](auto a, auto b) {
        auto sum = cc->EvalAdd(a, b);
        return cc->EvalSub(ct_two, sum);
    };

    // Homomorphic threshold: step function via polynomial approximation
    // step(x) = 0.5 + 0.5 * tanh(k * (x - 0.5))
    // Para sa malaking k, lumalapit sa ideal step function
    // Sa polynomial approximation: step(x) ≈ 0.5 + 0.5 * (x-0.5) * (3 - (x-0.5)²) / 2
    auto eval_threshold = [&](auto x) {
        // Shift: x - 0.5
        auto shifted = cc->EvalSub(x, ct_half);
        
        // Polynomial approximation ng step function
        // step(x) ≈ 0.5 + 0.5 * sign(x)
        // Para sa |x| ≤ 1: sign(x) ≈ x * (3 - x²) / 2
        
        // x²
        auto x_sq = cc->EvalMult(shifted, shifted);
        
        // 3 - x²
        auto three_minus_x_sq = cc->EvalSub(make_ct(3.0), x_sq);
        
        // x * (3 - x²)
        auto x_times = cc->EvalMult(shifted, three_minus_x_sq);
        
        // x * (3 - x²) / 2
        auto sign_approx = cc->EvalMult(x_times, make_ct(0.5));
        
        // 0.5 + 0.5 * sign_approx
        auto half_sign = cc->EvalMult(sign_approx, ct_half);
        return cc->EvalAdd(ct_half, half_sign);
    };

    // Full homomorphic NAND
    auto eval_nand_homomorphic = [&](auto a, auto b) {
        auto nand_raw = eval_nand_raw(a, b);
        return eval_threshold(nand_raw);
    };

    std::cout << "HOMOMORPHIC THRESHOLD TEST:\n";
    std::cout << "===========================\n\n";

    // Test values
    auto test_vals = {0.0, 1.0, 2.0};
    for (double v : test_vals) {
        auto ct_val = make_ct(v);
        auto ct_thresholded = eval_threshold(ct_val);
        double result = decrypt_val(ct_thresholded);
        std::cout << "  threshold(" << v << ") = " << result << "\n";
    }
    std::cout << "\n";

    // Test NAND
    std::cout << "HOMOMORPHIC NAND:\n";
    std::cout << "=================\n\n";

    struct TestCase {
        int a_bit;
        int b_bit;
        Ciphertext<DCRTPoly> ct_a;
        Ciphertext<DCRTPoly> ct_b;
    };

    std::vector<TestCase> tests = {
        {0, 0, ct_zero, ct_zero},
        {0, 1, ct_zero, ct_one},
        {1, 0, ct_one, ct_zero},
        {1, 1, ct_one, ct_one}
    };

    int correct = 0;
    for (auto& t : tests) {
        auto nand_result = eval_nand_homomorphic(t.ct_a, t.ct_b);
        double val = decrypt_val(nand_result);
        int got = (val > 0.5) ? 1 : 0;
        int expected = !(t.a_bit & t.b_bit);

        if (got == expected) correct++;

        std::cout << "  NAND(" << t.a_bit << "," << t.b_bit << ") = "
                  << expected << " → " << got
                  << " (value=" << val
                  << ", level=" << nand_result->GetLevel() << ")"
                  << (got == expected ? " ✓" : " ✗") << "\n";
    }

    std::cout << "\n  NAND: " << correct << "/4\n";
    std::cout << "  Level: " << eval_nand_homomorphic(ct_one, ct_one)->GetLevel() << "\n";

    return 0;
}
