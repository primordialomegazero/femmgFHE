// BINFHE 8-BIT COMPARATOR
// A > B circuit — ibang structure kaysa adder
// 32 random samples

#include "openfhe.h"
#include <iostream>
#include <chrono>
#include <random>

using namespace lbcrypto;
using namespace std::chrono;

int main() {
    std::cout << "========================================\n";
    std::cout << "  BINFHE 8-BIT COMPARATOR\n";
    std::cout << "  A > B Circuit\n";
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

    // 8-bit comparator: A > B
    auto comparator = [&](auto a_bits, auto b_bits) {
        // diff_i = A_i XOR B_i
        // gt_i = A_i AND NOT(B_i)
        // A > B = gt_7 OR (diff_7 AND gt_6) OR (diff_7 AND diff_6 AND gt_5) OR ...
        
        auto result = cc.Encrypt(sk, 0);  // False initially
        
        auto all_diff = cc.Encrypt(sk, 1);  // Accumulate diff from MSB
        for (int i = 7; i >= 0; i--) {
            auto diff = XOR(a_bits[i], b_bits[i]);
            auto not_b = NOT(b_bits[i]);
            auto gt = AND(a_bits[i], not_b);
            auto term = AND(all_diff, gt);
            result = OR(result, term);
            all_diff = AND(all_diff, diff);
        }
        
        return result;
    };

    int errors = 0;
    int total_tests = 32;
    std::mt19937 rng(777);

    std::cout << "TESTING 32 RANDOM SAMPLES (8-bit comparator)...\n";
    std::cout << "=================================================\n\n";

    auto t_start = high_resolution_clock::now();

    for (int test = 0; test < total_tests; test++) {
        int a_val = rng() % 256;
        int b_val = rng() % 256;

        std::vector<decltype(cc.Encrypt(sk, 0))> a_bits, b_bits;
        for (int i = 0; i < 8; i++) {
            a_bits.push_back(cc.Encrypt(sk, (a_val >> i) & 1));
            b_bits.push_back(cc.Encrypt(sk, (b_val >> i) & 1));
        }

        auto result = comparator(a_bits, b_bits);
        int got = decrypt_bit(result);
        int expected = (a_val > b_val) ? 1 : 0;

        if (got != expected) {
            errors++;
            std::cout << "  ERROR: " << a_val << " > " << b_val
                      << " = " << got << " expected " << expected << "\n";
        }

        if (test % 8 == 0 && test > 0) {
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
