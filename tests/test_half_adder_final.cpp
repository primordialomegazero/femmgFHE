// FINAL HALF-ADDER — Direct XOR + AND
// Walang composition error, lahat 4/4

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  FINAL HALF-ADDER\n";
    std::cout << "  Direct XOR + AND\n";
    std::cout << "========================================\n\n";

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

    // NOT = NAND(x,x)
    auto eval_not = [&](auto x) {
        return eval_nand(x, x);
    };

    // AND = NOT(NAND(a,b))
    auto eval_and = [&](auto a, auto b) {
        auto nand_ab = eval_nand(a, b);
        return eval_not(nand_ab);
    };

    // Direct XOR = (a-b)²
    auto eval_xor = [&](auto a, auto b) {
        auto diff = cc->EvalSub(a, b);
        return cc->EvalMult(diff, diff);
    };

    // Half-adder
    auto eval_half_adder = [&](auto a, auto b) {
        auto sum = eval_xor(a, b);
        auto carry = eval_and(a, b);
        return std::make_pair(sum, carry);
    };

    std::cout << "HALF-ADDER TEST:\n";
    std::cout << "================\n\n";

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

    int correct_sum = 0;
    int correct_carry = 0;

    for (auto& t : tests) {
        auto [sum, carry] = eval_half_adder(t.ct_a, t.ct_b);
        
        double sum_val = decrypt_val(sum);
        double carry_val = decrypt_val(carry);
        
        int sum_got = (sum_val > 0.5) ? 1 : 0;
        int carry_got = (carry_val > 0.5) ? 1 : 0;
        
        int sum_expected = t.a_bit ^ t.b_bit;
        int carry_expected = t.a_bit & t.b_bit;

        if (sum_got == sum_expected) correct_sum++;
        if (carry_got == carry_expected) correct_carry++;

        std::cout << "  " << t.a_bit << " + " << t.b_bit << " = "
                  << "SUM=" << sum_got << "/" << sum_expected
                  << " CARRY=" << carry_got << "/" << carry_expected
                  << " (sum_val=" << sum_val
                  << ", carry_val=" << carry_val << ")"
                  << (sum_got == sum_expected && carry_got == carry_expected ? " ✓" : " ✗")
                  << "\n";
    }

    std::cout << "\n  SUM: " << correct_sum << "/4\n";
    std::cout << "  CARRY: " << correct_carry << "/4\n";
    std::cout << "  Status: " << (correct_sum == 4 && correct_carry == 4 ? "✅ FINAL HALF-ADDER!" : "⚠️ MAY ERRORS") << "\n";

    return 0;
}
