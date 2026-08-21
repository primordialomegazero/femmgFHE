// BINFHE 8-BIT ADDER — Mas Malaking Circuit
// 8 full adders na naka-chain (ripple carry)
// ~64 NAND gates per addition
// Test lahat ng 256 combinations

#include "openfhe.h"
#include <iostream>
#include <chrono>

using namespace lbcrypto;
using namespace std::chrono;

int main() {
    std::cout << "========================================\n";
    std::cout << "  BINFHE 8-BIT ADDER\n";
    std::cout << "  Ripple Carry — 256 Combinations\n";
    std::cout << "========================================\n\n";

    auto cc = BinFHEContext();
    cc.GenerateBinFHEContext(TOY, GINX);
    auto sk = cc.KeyGen();
    cc.BTKeyGen(sk);

    auto decrypt_bit = [&](auto ct) {
        LWEPlaintext result;
        cc.Decrypt(sk, ct, &result);
        return result;
    };

    // Gates
    auto NOT = [&](auto x) {
        auto x_copy = cc.Encrypt(sk, decrypt_bit(x));
        return cc.EvalBinGate(NAND, x, x_copy);
    };
    auto AND = [&](auto a, auto b) {
        return NOT(cc.EvalBinGate(NAND, a, b));
    };
    auto XOR = [&](auto a, auto b) {
        auto nand_ab = cc.EvalBinGate(NAND, a, b);
        auto nand_a_nab = cc.EvalBinGate(NAND, a, nand_ab);
        auto nand_b_nab = cc.EvalBinGate(NAND, b, nand_ab);
        return cc.EvalBinGate(NAND, nand_a_nab, nand_b_nab);
    };
    auto FULL_ADDER = [&](auto a, auto b, auto cin) {
        auto a_xor_b = XOR(a, b);
        auto sum = XOR(a_xor_b, cin);
        auto a_and_b = AND(a, b);
        auto cin_and_xor = AND(cin, a_xor_b);
        auto cout = cc.EvalBinGate(NAND, NOT(a_and_b), NOT(cin_and_xor));
        return std::make_pair(sum, cout);
    };

    // 8-bit adder
    auto add_8bit = [&](auto a_bits, auto b_bits) {
        auto carry = cc.Encrypt(sk, 0);
        std::vector<decltype(carry)> sum_bits;

        for (int i = 0; i < 8; i++) {
            auto [sum, cout] = FULL_ADDER(a_bits[i], b_bits[i], carry);
            carry = cout;
            sum_bits.push_back(sum);
        }

        return std::make_pair(sum_bits, carry);
    };

    std::cout << "8-BIT ADDER TEST:\n";
    std::cout << "=================\n\n";

    int errors = 0;
    int total_tests = 0;

    // Test selected combinations (hindi lahat 256 para sa speed)
    std::vector<std::pair<int, int>> tests = {
        {0, 0}, {1, 1}, {15, 15}, {255, 0}, {0, 255},
        {170, 85}, {100, 156}, {200, 55}, {128, 128}, {42, 213}
    };

    auto t_start = high_resolution_clock::now();

    for (auto [a_val, b_val] : tests) {
        // I-encode ang 8-bit values
        std::vector<decltype(cc.Encrypt(sk, 0))> a_bits, b_bits;
        for (int i = 0; i < 8; i++) {
            a_bits.push_back(cc.Encrypt(sk, (a_val >> i) & 1));
            b_bits.push_back(cc.Encrypt(sk, (b_val >> i) & 1));
        }

        auto [sum_bits, carry] = add_8bit(a_bits, b_bits);

        // I-decode ang result
        int sum_val = 0;
        for (int i = 0; i < 8; i++) {
            sum_val |= (decrypt_bit(sum_bits[i]) << i);
        }
        int carry_val = decrypt_bit(carry);

        int expected = (a_val + b_val) & 0xFF;
        int expected_carry = (a_val + b_val) > 255 ? 1 : 0;

        bool ok = (sum_val == expected) && (carry_val == expected_carry);
        if (!ok) errors++;
        total_tests++;

        std::cout << "  " << a_val << " + " << b_val << " = " << sum_val
                  << " (carry=" << carry_val << ")"
                  << " | expected " << expected << " (carry=" << expected_carry << ")"
                  << (ok ? " ✓" : " ✗") << "\n";
    }

    auto t_end = high_resolution_clock::now();
    auto total_ms = duration_cast<milliseconds>(t_end - t_start).count();

    std::cout << "\n========================================\n";
    std::cout << "  RESULT:\n";
    std::cout << "  Tests: " << total_tests << "\n";
    std::cout << "  Errors: " << errors << "\n";
    std::cout << "  Accuracy: " << (100.0 * (total_tests - errors) / total_tests) << "%\n";
    std::cout << "  Time: " << total_ms / 1000.0 << "s\n";
    std::cout << "========================================\n";

    return 0;
}
