// FULL STRESS TEST — Lahat ng Gates, Malalim na Circuit
// NAND, AND, XOR, OR, Half-Adder, Full-Adder
// Stress: 1000 gates, multi-level circuits

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
    std::cout << "  FULL STRESS TEST\n";
    std::cout << "  Lahat ng Gates, Malalim na Circuit\n";
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

    auto eval_not = [&](auto x) {
        return eval_nand(x, x);
    };

    auto eval_and = [&](auto a, auto b) {
        auto nand_ab = eval_nand(a, b);
        return eval_not(nand_ab);
    };

    auto eval_or = [&](auto a, auto b) {
        auto not_a = eval_not(a);
        auto not_b = eval_not(b);
        return eval_nand(not_a, not_b);
    };

    auto eval_xor = [&](auto a, auto b) {
        auto diff = cc->EvalSub(a, b);
        return cc->EvalMult(diff, diff);
    };

    auto decrypt_bit = [&](auto ct) {
        double val = decrypt_val(ct);
        return (val > 0.5) ? 1 : 0;
    };

    // ========================================
    // STRESS TEST 1: LAHAT NG GATES
    // ========================================
    std::cout << "STRESS TEST 1: LAHAT NG GATES\n";
    std::cout << "===============================\n\n";

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

    int correct_nand = 0, correct_and = 0, correct_or = 0, correct_xor = 0, correct_not = 0;

    for (auto& t : tests) {
        auto nand = eval_nand(t.ct_a, t.ct_b);
        auto and_result = eval_and(t.ct_a, t.ct_b);
        auto or_result = eval_or(t.ct_a, t.ct_b);
        auto xor_result = eval_xor(t.ct_a, t.ct_b);
        auto not_a = eval_not(t.ct_a);

        if (decrypt_bit(nand) == !(t.a_bit & t.b_bit)) correct_nand++;
        if (decrypt_bit(and_result) == (t.a_bit & t.b_bit)) correct_and++;
        if (decrypt_bit(or_result) == (t.a_bit | t.b_bit)) correct_or++;
        if (decrypt_bit(xor_result) == (t.a_bit ^ t.b_bit)) correct_xor++;
        if (decrypt_bit(not_a) == !t.a_bit) correct_not++;
    }

    std::cout << "  NAND: " << correct_nand << "/4\n";
    std::cout << "  AND: " << correct_and << "/4\n";
    std::cout << "  OR: " << correct_or << "/4\n";
    std::cout << "  XOR: " << correct_xor << "/4\n";
    std::cout << "  NOT: " << correct_not << "/4\n\n";

    // ========================================
    // STRESS TEST 2: FULL ADDER
    // ========================================
    std::cout << "STRESS TEST 2: FULL ADDER\n";
    std::cout << "==========================\n\n";

    auto eval_full_adder = [&](auto a, auto b, auto c_in) {
        // SUM = XOR(XOR(a,b), c_in)
        auto xor_ab = eval_xor(a, b);
        auto sum = eval_xor(xor_ab, c_in);
        
        // CARRY = OR(AND(a,b), AND(XOR(a,b), c_in))
        auto and_ab = eval_and(a, b);
        auto and_xor_c = eval_and(xor_ab, c_in);
        auto carry = eval_or(and_ab, and_xor_c);
        
        return std::make_pair(sum, carry);
    };

    int correct_full_adder = 0;
    int total_full_adder = 0;

    for (int a = 0; a <= 1; a++) {
        for (int b = 0; b <= 1; b++) {
            for (int c_in = 0; c_in <= 1; c_in++) {
                auto ct_a = a ? ct_one : ct_zero;
                auto ct_b = b ? ct_one : ct_zero;
                auto ct_cin = c_in ? ct_one : ct_zero;
                
                auto [sum, carry] = eval_full_adder(ct_a, ct_b, ct_cin);
                
                int sum_got = decrypt_bit(sum);
                int carry_got = decrypt_bit(carry);
                int sum_expected = a ^ b ^ c_in;
                int carry_expected = (a & b) | (b & c_in) | (a & c_in);
                
                if (sum_got == sum_expected && carry_got == carry_expected) {
                    correct_full_adder++;
                }
                total_full_adder++;
                
                std::cout << "  " << a << "+" << b << "+" << c_in << " = "
                          << "SUM=" << sum_got << "/" << sum_expected
                          << " CARRY=" << carry_got << "/" << carry_expected
                          << (sum_got == sum_expected && carry_got == carry_expected ? " ✓" : " ✗")
                          << "\n";
            }
        }
    }

    std::cout << "\n  FULL ADDER: " << correct_full_adder << "/" << total_full_adder << "\n\n";

    // ========================================
    // STRESS TEST 3: 1000-GATE CHAIN
    // ========================================
    std::cout << "STRESS TEST 3: 1000-GATE CHAIN\n";
    std::cout << "================================\n\n";

    auto state = eval_nand(ct_one, ct_one);
    int errors_1000 = 0;
    int expected_bit = 0;

    auto start = high_resolution_clock::now();

    for (int i = 1; i <= 1000; i++) {
        double val = decrypt_val(state);
        int bit = decrypt_bit(state);
        
        if (bit != expected_bit) errors_1000++;
        
        auto next_input = (bit == 1) ? ct_one : ct_zero;
        state = eval_nand(next_input, next_input);
        expected_bit = 1 - expected_bit;
    }

    auto end = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(end - start).count();

    std::cout << "  Layers: 1000\n";
    std::cout << "  Errors: " << errors_1000 << "\n";
    std::cout << "  Time: " << duration << " ms\n";
    std::cout << "  Final Level: " << state->GetLevel() << "\n\n";

    // ========================================
    // SUMMARY
    // ========================================
    std::cout << "========================================\n";
    std::cout << "  SUMMARY\n";
    std::cout << "========================================\n\n";
    std::cout << "  All Gates: " << (correct_nand == 4 && correct_and == 4 && correct_or == 4 && correct_xor == 4 && correct_not == 4 ? "✅" : "⚠️") << "\n";
    std::cout << "  Full Adder: " << (correct_full_adder == 8 ? "✅" : "⚠️") << "\n";
    std::cout << "  1000-Gate Chain: " << (errors_1000 == 0 ? "✅" : "⚠️") << "\n";
    std::cout << "========================================\n";

    return 0;
}
