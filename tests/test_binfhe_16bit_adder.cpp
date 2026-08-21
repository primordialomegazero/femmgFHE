// BINFHE 16-BIT ADDER — Mas Malaking Circuit
// 16 full adders na naka-chain
// 64 random samples (para sa speed)

#include "openfhe.h"
#include <iostream>
#include <chrono>
#include <random>

using namespace lbcrypto;
using namespace std::chrono;

int main() {
    std::cout << "========================================\n";
    std::cout << "  BINFHE 16-BIT ADDER\n";
    std::cout << "  64 Random Samples\n";
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

    int errors = 0;
    int total_tests = 64;
    std::mt19937 rng(12345);

    std::cout << "TESTING 64 RANDOM SAMPLES (16-bit)...\n";
    std::cout << "========================================\n\n";

    auto t_start = high_resolution_clock::now();

    for (int test = 0; test < total_tests; test++) {
        int a_val = rng() % 65536;
        int b_val = rng() % 65536;

        std::vector<decltype(cc.Encrypt(sk, 0))> a_bits, b_bits;
        for (int i = 0; i < 16; i++) {
            a_bits.push_back(cc.Encrypt(sk, (a_val >> i) & 1));
            b_bits.push_back(cc.Encrypt(sk, (b_val >> i) & 1));
        }

        auto carry = cc.Encrypt(sk, 0);
        std::vector<decltype(carry)> sum_bits;

        for (int i = 0; i < 16; i++) {
            auto [sum, cout] = FULL_ADDER(a_bits[i], b_bits[i], carry);
            carry = cout;
            sum_bits.push_back(sum);
        }

        int sum_val = 0;
        for (int i = 0; i < 16; i++) {
            sum_val |= (decrypt_bit(sum_bits[i]) << i);
        }
        int carry_val = decrypt_bit(carry);

        int expected = (a_val + b_val) & 0xFFFF;
        int expected_carry = (a_val + b_val) > 65535 ? 1 : 0;

        if (sum_val != expected || carry_val != expected_carry) {
            errors++;
            std::cout << "  ERROR: " << a_val << " + " << b_val
                      << " = " << sum_val << " expected " << expected << "\n";
        }

        if (test % 16 == 0 && test > 0) {
            auto t_now = high_resolution_clock::now();
            auto elapsed = duration_cast<seconds>(t_now - t_start).count();
            std::cout << "  [" << test << "/" << total_tests << "] elapsed=" << elapsed << "s errors=" << errors << "\n";
            std::cout.flush();
        }
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
