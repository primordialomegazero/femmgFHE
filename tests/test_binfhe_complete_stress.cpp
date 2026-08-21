// BINFHE COMPLETE STRESS — Lahat ng missing tests
// 1. Equality TRUE cases
// 2. 4-bit Comparator Chain
// 3. Ripple Borrow Subtractor (4-bit)
// 4. 2-bit Multiplier

#include "openfhe.h"
#include <iostream>
#include <chrono>

using namespace lbcrypto;
using namespace std::chrono;

int main() {
    std::cout << "========================================\n";
    std::cout << "  BINFHE COMPLETE STRESS\n";
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
    auto OR = [&](auto a, auto b) {
        return cc.EvalBinGate(NAND, NOT(a), NOT(b));
    };
    auto XOR = [&](auto a, auto b) {
        auto nand_ab = cc.EvalBinGate(NAND, a, b);
        auto nand_a_nab = cc.EvalBinGate(NAND, a, nand_ab);
        auto nand_b_nab = cc.EvalBinGate(NAND, b, nand_ab);
        return cc.EvalBinGate(NAND, nand_a_nab, nand_b_nab);
    };
    auto XNOR = [&](auto a, auto b) { return NOT(XOR(a, b)); };
    auto FULL_ADDER = [&](auto a, auto b, auto cin) {
        auto a_xor_b = XOR(a, b);
        auto sum = XOR(a_xor_b, cin);
        auto a_and_b = AND(a, b);
        auto cin_and_xor = AND(cin, a_xor_b);
        auto cout = cc.EvalBinGate(NAND, NOT(a_and_b), NOT(cin_and_xor));
        return std::make_pair(sum, cout);
    };

    int total_errors = 0;

    // ============================================
    // 1. EQUALITY CHECKER — MAY TRUE CASES
    // ============================================
    std::cout << "1. 4-BIT EQUALITY (may TRUE):\n";
    std::cout << "------------------------------\n";
    std::vector<std::pair<int,int>> eq_tests = {
        {5, 5}, {0, 0}, {15, 15}, {7, 7}, {10, 10}, {3, 3}, {12, 12}, {1, 2}
    };
    for (auto [a, b] : eq_tests) {
        std::vector<decltype(cc.Encrypt(sk, 0))> a_bits, b_bits;
        for (int i = 0; i < 4; i++) {
            a_bits.push_back(cc.Encrypt(sk, (a >> i) & 1));
            b_bits.push_back(cc.Encrypt(sk, (b >> i) & 1));
        }
        auto all_equal = cc.Encrypt(sk, 1);
        for (int i = 0; i < 4; i++) {
            all_equal = AND(all_equal, XNOR(a_bits[i], b_bits[i]));
        }
        int got = decrypt_bit(all_equal);
        int expected = (a == b) ? 1 : 0;
        if (got != expected) total_errors++;
        std::cout << "  " << a << "==" << b << " → " << got
                  << (got == expected ? " ✓" : " ✗") << "\n";
    }
    std::cout << "\n";

    // ============================================
    // 2. 4-BIT COMPARATOR CHAIN
    // ============================================
    std::cout << "2. 4-BIT COMPARATOR (A>B):\n";
    std::cout << "----------------------------\n";
    std::vector<std::pair<int,int>> cmp_tests = {
        {10, 5}, {3, 7}, {15, 15}, {0, 0}, {8, 4}, {2, 9}, {14, 13}, {1, 1}
    };
    for (auto [a, b] : cmp_tests) {
        std::vector<decltype(cc.Encrypt(sk, 0))> a_bits, b_bits;
        for (int i = 0; i < 4; i++) {
            a_bits.push_back(cc.Encrypt(sk, (a >> i) & 1));
            b_bits.push_back(cc.Encrypt(sk, (b >> i) & 1));
        }
        auto result = cc.Encrypt(sk, 0);
        auto prev_equal = cc.Encrypt(sk, 1);
        for (int i = 3; i >= 0; i--) {
            auto diff = XOR(a_bits[i], b_bits[i]);
            auto gt = AND(a_bits[i], NOT(b_bits[i]));
            result = OR(result, AND(prev_equal, gt));
            prev_equal = AND(prev_equal, NOT(diff));
        }
        int got = decrypt_bit(result);
        int expected = (a > b) ? 1 : 0;
        if (got != expected) total_errors++;
        std::cout << "  " << a << ">" << b << " → " << got
                  << (got == expected ? " ✓" : " ✗") << "\n";
    }
    std::cout << "\n";

    // ============================================
    // 3. 4-BIT RIPPLE BORROW SUBTRACTOR
    // ============================================
    std::cout << "3. 4-BIT SUBTRACTOR (A-B):\n";
    std::cout << "----------------------------\n";
    std::vector<std::pair<int,int>> sub_tests = {
        {10, 3}, {7, 2}, {15, 15}, {0, 0}, {8, 5}, {5, 8}, {12, 4}, {3, 3}
    };
    for (auto [a, b] : sub_tests) {
        std::vector<decltype(cc.Encrypt(sk, 0))> a_bits, b_bits;
        for (int i = 0; i < 4; i++) {
            a_bits.push_back(cc.Encrypt(sk, (a >> i) & 1));
            b_bits.push_back(cc.Encrypt(sk, (b >> i) & 1));
        }
        auto borrow = cc.Encrypt(sk, 0);
        std::vector<decltype(borrow)> diff_bits;
        for (int i = 0; i < 4; i++) {
            auto diff = XOR(XOR(a_bits[i], b_bits[i]), borrow);
            auto not_a = NOT(a_bits[i]);
            auto term1 = AND(not_a, b_bits[i]);
            auto term2 = AND(not_a, borrow);
            auto term3 = AND(b_bits[i], borrow);
            borrow = OR(OR(term1, term2), term3);
            diff_bits.push_back(diff);
        }
        int diff_val = 0;
        for (int i = 0; i < 4; i++) diff_val |= (decrypt_bit(diff_bits[i]) << i);
        int borrow_val = decrypt_bit(borrow);
        int expected_diff = (a - b) & 0xF;
        int expected_borrow = (a < b) ? 1 : 0;
        bool ok = (diff_val == expected_diff) && (borrow_val == expected_borrow);
        if (!ok) total_errors++;
        std::cout << "  " << a << "-" << b << "=" << diff_val << " (borrow=" << borrow_val << ")"
                  << (ok ? " ✓" : " ✗") << "\n";
    }
    std::cout << "\n";

    // ============================================
    // 4. 2-BIT MULTIPLIER
    // ============================================
    std::cout << "4. 2-BIT MULTIPLIER (A×B):\n";
    std::cout << "----------------------------\n";
    std::vector<std::pair<int,int>> mul_tests = {
        {0, 0}, {1, 2}, {2, 3}, {3, 3}, {2, 2}, {1, 1}, {3, 1}, {0, 3}
    };
    for (auto [a, b] : mul_tests) {
        // 2-bit multiplier: p0=a0·b0, p1=a1·b0⊕a0·b1, p2=a1·b1⊕carry, p3=carry
        auto a0 = cc.Encrypt(sk, a & 1);
        auto a1 = cc.Encrypt(sk, (a >> 1) & 1);
        auto b0 = cc.Encrypt(sk, b & 1);
        auto b1 = cc.Encrypt(sk, (b >> 1) & 1);

        auto p0 = AND(a0, b0);
        auto a1b0 = AND(a1, b0);
        auto a0b1 = AND(a0, b1);
        auto p1 = XOR(a1b0, a0b1);
        auto carry1 = AND(a1b0, a0b1);
        auto a1b1 = AND(a1, b1);
        auto p2 = XOR(a1b1, carry1);
        auto carry2 = AND(a1b1, carry1);
        auto p3 = carry2;

        int prod = decrypt_bit(p0) | (decrypt_bit(p1) << 1) | 
                   (decrypt_bit(p2) << 2) | (decrypt_bit(p3) << 3);
        int expected = a * b;
        bool ok = (prod == expected);
        if (!ok) total_errors++;
        std::cout << "  " << a << "×" << b << "=" << prod
                  << " (exp " << expected << ")"
                  << (ok ? " ✓" : " ✗") << "\n";
    }

    std::cout << "\n========================================\n";
    std::cout << "  TOTAL ERRORS: " << total_errors << "\n";
    std::cout << "========================================\n";

    return 0;
}
