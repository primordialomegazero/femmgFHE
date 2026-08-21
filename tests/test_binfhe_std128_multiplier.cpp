// BINFHE STD128 — 8-BIT MULTIPLIER
// Practical circuit test

#include "openfhe.h"
#include <iostream>
#include <chrono>

using namespace lbcrypto;
using namespace std::chrono;

int main() {
    std::cout << "========================================\n";
    std::cout << "  BINFHE STD128 — 8-BIT MULTIPLIER\n";
    std::cout << "========================================\n\n";

    auto cc = BinFHEContext();
    cc.GenerateBinFHEContext(STD128, GINX);
    auto sk = cc.KeyGen();
    cc.BTKeyGen(sk);

    auto decrypt_bit = [&](auto ct) -> int {
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
        auto ab_and = AND(a, b);
        auto ac_and = AND(a_xor_b, cin);
        auto carry = cc.EvalBinGate(NAND, NOT(ab_and), NOT(ac_and));
        return std::make_pair(sum, carry);
    };

    // 8-bit multiplier
    auto multiply_8bit = [&](std::vector<LWECiphertext> A,
                              std::vector<LWECiphertext> B) {
        std::vector<LWECiphertext> result(16);
        for (int i = 0; i < 16; i++) {
            result[i] = cc.Encrypt(sk, 0);
        }

        for (int i = 0; i < 8; i++) {
            std::vector<LWECiphertext> partial(16);
            for (int j = 0; j < 16; j++) {
                partial[j] = cc.Encrypt(sk, 0);
            }

            for (int j = 0; j < 8; j++) {
                auto bit = AND(A[j], B[i]);
                partial[i + j] = bit;
            }

            // Add partial to result
            auto carry = cc.Encrypt(sk, 0);
            for (int j = 0; j < 16; j++) {
                auto adder_result = FULL_ADDER(result[j], partial[j], carry);
                result[j] = adder_result.first;
                carry = adder_result.second;
            }
        }
        return result;
    };

    // Test: 5 × 7 = 35
    std::cout << "Test: 5 × 7 = 35\n\n";

    auto A = [&](int val) {
        std::vector<LWECiphertext> bits(8);
        for (int i = 0; i < 8; i++) {
            bits[i] = cc.Encrypt(sk, (val >> i) & 1);
        }
        return bits;
    };

    auto t_start = high_resolution_clock::now();
    auto bits5 = A(5);
    auto bits7 = A(7);
    auto result = multiply_8bit(bits5, bits7);
    auto t_end = high_resolution_clock::now();

    int product = 0;
    for (int i = 0; i < 16; i++) {
        int bit = decrypt_bit(result[i]);
        product |= (bit << i);
    }

    auto elapsed = duration_cast<seconds>(t_end - t_start).count();

    std::cout << "  5 × 7 = " << product << " (expected 35)\n";
    std::cout << (product == 35 ? "  ✓ PASS" : "  ✗ FAIL") << "\n";
    std::cout << "  Time: " << elapsed << "s\n";

    return 0;
}
