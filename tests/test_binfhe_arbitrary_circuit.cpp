// BINFHE ARBITRARY CIRCUIT TEST
// Hindi na puro NOT chain — may branching at combining
//
// ANG KEY QUESTION:
// Ang period-2 ba ay specific sa NOT chain,
// o may general noise limiting sa lahat ng NAND circuits?
//
// TEST: Full Adder na may iba't ibang inputs
// Kung ang full adder ay may 0 errors sa malalim na chain,
// ibig sabihin may general property tayo

#include "openfhe.h"
#include <iostream>
#include <chrono>

using namespace lbcrypto;
using namespace std::chrono;

int main() {
    std::cout << "========================================\n";
    std::cout << "  BINFHE ARBITRARY CIRCUIT\n";
    std::cout << "  Full Adder Chain Test\n";
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

    // ============================================
    // FULL ADDER COMPONENTS
    // ============================================
    auto NOT = [&](auto x) {
        auto x_copy = cc.Encrypt(sk, decrypt_bit(x));
        return cc.EvalBinGate(NAND, x, x_copy);
    };

    auto AND = [&](auto a, auto b) {
        auto nand_ab = cc.EvalBinGate(NAND, a, b);
        return NOT(nand_ab);
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

    std::cout << "FULL ADDER TRUTH TABLE:\n";
    std::cout << "=======================\n\n";

    // Test lahat ng 8 combinations
    for (int a_val : {0, 1}) {
        for (int b_val : {0, 1}) {
            for (int cin_val : {0, 1}) {
                auto a = cc.Encrypt(sk, a_val);
                auto b = cc.Encrypt(sk, b_val);
                auto cin = cc.Encrypt(sk, cin_val);

                auto [sum, cout] = FULL_ADDER(a, b, cin);

                int sum_got = decrypt_bit(sum);
                int cout_got = decrypt_bit(cout);
                int sum_expected = a_val ^ b_val ^ cin_val;
                int cout_expected = (a_val & b_val) | (cin_val & (a_val ^ b_val));

                bool ok = (sum_got == sum_expected) && (cout_got == cout_expected);

                std::cout << "  " << a_val << b_val << cin_val
                          << " → sum=" << sum_got << " cout=" << cout_got
                          << " (expected " << sum_expected << cout_expected << ")"
                          << (ok ? " ✓" : " ✗") << "\n";
            }
        }
    }
    std::cout << "\n";

    // ============================================
    // CHAIN OF FULL ADDERS (ripple carry)
    // ============================================
    std::cout << "RIPPLE CARRY CHAIN (10 full adders):\n";
    std::cout << "=====================================\n\n";

    // I-chain ang 10 full adders
    auto carry = cc.Encrypt(sk, 0);
    int errors = 0;

    auto t_start = high_resolution_clock::now();

    for (int i = 0; i < 10; i++) {
        auto a = cc.Encrypt(sk, 1);  // A=1
        auto b = cc.Encrypt(sk, 0);  // B=0
        
        auto [sum, cout] = FULL_ADDER(a, b, carry);
        carry = cout;

        int sum_got = decrypt_bit(sum);
        int sum_expected = 1;  // 1+0+carry

        if (sum_got != sum_expected) errors++;

        if (i < 3 || i >= 7) {
            std::cout << "  Adder " << i << ": sum=" << sum_got
                      << " carry=" << decrypt_bit(carry)
                      << " (sum expected " << sum_expected << ")"
                      << (sum_got == sum_expected ? " ✓" : " ✗") << "\n";
        }
    }

    auto t_end = high_resolution_clock::now();
    auto total_ms = duration_cast<milliseconds>(t_end - t_start).count();

    std::cout << "\n  Errors: " << errors << "/10\n";
    std::cout << "  Time: " << total_ms / 1000.0 << "s\n";

    return 0;
}
