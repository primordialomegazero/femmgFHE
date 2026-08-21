// BINFHE STD128 — FULL STRESS
// 1. 1000 NAND gates
// 2. 8-bit Adder 32 samples
// Production security level

#include "openfhe.h"
#include <iostream>
#include <chrono>
#include <random>

using namespace lbcrypto;
using namespace std::chrono;

int main() {
    std::cout << "========================================\n";
    std::cout << "  BINFHE STD128 — FULL STRESS\n";
    std::cout << "========================================\n\n";

    auto cc = BinFHEContext();
    auto t_setup_start = high_resolution_clock::now();
    cc.GenerateBinFHEContext(STD128, GINX);
    auto sk = cc.KeyGen();
    cc.BTKeyGen(sk);
    auto t_setup_end = high_resolution_clock::now();
    std::cout << "Setup: " << duration_cast<milliseconds>(t_setup_end - t_setup_start).count() / 1000.0 << "s\n\n";

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
    auto OR = [&](auto a, auto b) {
        return cc.EvalBinGate(NAND, NOT(a), NOT(b));
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

    // ============================================
    // TEST 1: 1000 NAND GATES
    // ============================================
    std::cout << "TEST 1: 1000 NAND GATES\n";
    std::cout << "========================\n\n";

    auto current = cc.Encrypt(sk, 1);
    int errors_nand = 0;

    auto t1 = high_resolution_clock::now();

    for (int gate = 0; gate < 1000; gate++) {
        auto val = decrypt_bit(current);
        auto copy = cc.Encrypt(sk, val);
        current = cc.EvalBinGate(NAND, current, copy);

        int got = decrypt_bit(current);
        int expected = (gate % 2 == 0) ? 0 : 1;
        if (got != expected) errors_nand++;

        if (gate % 200 == 0) {
            auto now = high_resolution_clock::now();
            auto elapsed = duration_cast<seconds>(now - t1).count();
            std::cout << "  [" << gate << "/1000] elapsed=" << elapsed << "s errors=" << errors_nand << "\n";
            std::cout.flush();
        }
    }

    auto t2 = high_resolution_clock::now();
    std::cout << "  Result: " << errors_nand << "/1000 errors ("
              << (100.0 * (1000 - errors_nand) / 1000) << "%)"
              << " | Time: " << duration_cast<seconds>(t2 - t1).count() << "s\n\n";

    // ============================================
    // TEST 2: 8-BIT ADDER (32 SAMPLES)
    // ============================================
    std::cout << "TEST 2: 8-BIT ADDER (32 SAMPLES)\n";
    std::cout << "==================================\n\n";

    int errors_adder = 0;
    int total_adder = 32;
    std::mt19937 rng(4242);

    t1 = high_resolution_clock::now();

    for (int test = 0; test < total_adder; test++) {
        int a_val = rng() % 256;
        int b_val = rng() % 256;

        std::vector<decltype(cc.Encrypt(sk, 0))> a_bits, b_bits;
        for (int i = 0; i < 8; i++) {
            a_bits.push_back(cc.Encrypt(sk, (a_val >> i) & 1));
            b_bits.push_back(cc.Encrypt(sk, (b_val >> i) & 1));
        }

        auto carry = cc.Encrypt(sk, 0);
        std::vector<decltype(carry)> sum_bits;

        for (int i = 0; i < 8; i++) {
            auto [sum, cout] = FULL_ADDER(a_bits[i], b_bits[i], carry);
            carry = cout;
            sum_bits.push_back(sum);
        }

        int sum_val = 0;
        for (int i = 0; i < 8; i++) {
            sum_val |= (decrypt_bit(sum_bits[i]) << i);
        }
        int carry_val = decrypt_bit(carry);

        int expected = (a_val + b_val) & 0xFF;
        int expected_carry = (a_val + b_val) > 255 ? 1 : 0;

        if (sum_val != expected || carry_val != expected_carry) {
            errors_adder++;
        }

        if (test % 8 == 0 && test > 0) {
            auto now = high_resolution_clock::now();
            auto elapsed = duration_cast<seconds>(now - t1).count();
            std::cout << "  [" << test << "/32] elapsed=" << elapsed << "s errors=" << errors_adder << "\n";
            std::cout.flush();
        }
    }

    t2 = high_resolution_clock::now();
    std::cout << "  Result: " << errors_adder << "/" << total_adder << " errors ("
              << (100.0 * (total_adder - errors_adder) / total_adder) << "%)"
              << " | Time: " << duration_cast<seconds>(t2 - t1).count() << "s\n\n";

    // ============================================
    // FINAL VERDICT
    // ============================================
    std::cout << "========================================\n";
    std::cout << "  FINAL VERDICT (STD128):\n";
    std::cout << "  1000 NAND: " << (errors_nand == 0 ? "PASS ✓" : "FAIL ✗") << "\n";
    std::cout << "  8-bit Adder: " << (errors_adder == 0 ? "PASS ✓" : "FAIL ✗") << "\n";
    std::cout << "========================================\n";

    return 0;
}
