// BINFHE MUX + SUBTRACTOR + EQUALITY — Iba't ibang circuits
// Compact test: lahat sabay-sabay

#include "openfhe.h"
#include <iostream>
#include <chrono>

using namespace lbcrypto;
using namespace std::chrono;

int main() {
    std::cout << "========================================\n";
    std::cout << "  BINFHE: MUX + SUBTRACTOR + EQUALITY\n";
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
    auto XNOR = [&](auto a, auto b) {
        return NOT(XOR(a, b));
    };
    auto MUX = [&](auto sel, auto a, auto b) {
        // sel=0 → a, sel=1 → b
        auto not_sel = NOT(sel);
        auto sel_a = AND(sel, a);
        auto not_sel_b = AND(not_sel, b);
        return OR(sel_a, not_sel_b);
    };

    int total_errors = 0;

    // ============================================
    // MUX TRUTH TABLE
    // ============================================
    std::cout << "MUX (2:1):\n";
    std::cout << "-----------\n";
    for (int sel : {0, 1}) {
        for (int a : {0, 1}) {
            for (int b : {0, 1}) {
                auto ct_sel = cc.Encrypt(sk, sel);
                auto ct_a = cc.Encrypt(sk, a);
                auto ct_b = cc.Encrypt(sk, b);
                auto result = MUX(ct_sel, ct_a, ct_b);
                int got = decrypt_bit(result);
                int expected = sel ? a : b;
                if (got != expected) total_errors++;
                std::cout << "  sel=" << sel << " a=" << a << " b=" << b
                          << " → " << got << " (exp " << expected << ")"
                          << (got == expected ? " ✓" : " ✗") << "\n";
            }
        }
    }
    std::cout << "\n";

    // ============================================
    // 1-BIT SUBTRACTOR
    // ============================================
    std::cout << "1-BIT SUBTRACTOR (A-B-Bin):\n";
    std::cout << "---------------------------\n";
    for (int a : {0, 1}) {
        for (int b : {0, 1}) {
            for (int bin : {0, 1}) {
                auto ct_a = cc.Encrypt(sk, a);
                auto ct_b = cc.Encrypt(sk, b);
                auto ct_bin = cc.Encrypt(sk, bin);
                
                // diff = A XOR B XOR Bin
                auto diff = XOR(XOR(ct_a, ct_b), ct_bin);
                // bout = (NOT(A) AND B) OR (NOT(A) AND Bin) OR (B AND Bin)
                auto not_a = NOT(ct_a);
                auto term1 = AND(not_a, ct_b);
                auto term2 = AND(not_a, ct_bin);
                auto term3 = AND(ct_b, ct_bin);
                auto bout = OR(OR(term1, term2), term3);

                int diff_got = decrypt_bit(diff);
                int bout_got = decrypt_bit(bout);
                int diff_exp = a ^ b ^ bin;
                int bout_exp = (b & bin) | ((!a) & (b | bin));
                
                bool ok = (diff_got == diff_exp) && (bout_got == bout_exp);
                if (!ok) total_errors++;
                std::cout << "  A=" << a << " B=" << b << " Bin=" << bin
                          << " → diff=" << diff_got << " bout=" << bout_got
                          << " (exp " << diff_exp << bout_exp << ")"
                          << (ok ? " ✓" : " ✗") << "\n";
            }
        }
    }
    std::cout << "\n";

    // ============================================
    // EQUALITY CHECKER (4-bit)
    // ============================================
    std::cout << "4-BIT EQUALITY CHECKER:\n";
    std::cout << "-----------------------\n";
    for (int test = 0; test < 8; test++) {
        int a = test;
        int b = (test + 3) % 16;
        
        std::vector<decltype(cc.Encrypt(sk, 0))> a_bits, b_bits;
        for (int i = 0; i < 4; i++) {
            a_bits.push_back(cc.Encrypt(sk, (a >> i) & 1));
            b_bits.push_back(cc.Encrypt(sk, (b >> i) & 1));
        }

        auto all_equal = cc.Encrypt(sk, 1);
        for (int i = 0; i < 4; i++) {
            auto bit_equal = XNOR(a_bits[i], b_bits[i]);
            all_equal = AND(all_equal, bit_equal);
        }

        int got = decrypt_bit(all_equal);
        int expected = (a == b) ? 1 : 0;
        if (got != expected) total_errors++;
        std::cout << "  " << a << " == " << b << " → " << got
                  << " (exp " << expected << ")"
                  << (got == expected ? " ✓" : " ✗") << "\n";
    }

    std::cout << "\n========================================\n";
    std::cout << "  TOTAL ERRORS: " << total_errors << "\n";
    std::cout << "========================================\n";

    return 0;
}
