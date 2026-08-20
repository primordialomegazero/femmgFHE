// BINFHE FINAL — Tamang Independent Ciphertexts
#include "openfhe.h"
#include <iostream>
#include <chrono>

using namespace lbcrypto;
using namespace std::chrono;

int main() {
    std::cout << "========================================\n";
    std::cout << "  BINFHE NAND BENCHMARK\n";
    std::cout << "========================================\n\n";

    auto cc = BinFHEContext();
    cc.GenerateBinFHEContext(TOY, GINX);
    auto sk = cc.KeyGen();
    cc.BTKeyGen(sk);

    std::cout << "BinFHE Setup: COMPLETE\n\n";

    auto decrypt_bit = [&](auto ct) {
        LWEPlaintext result;
        cc.Decrypt(sk, ct, &result);
        return result;
    };

    // Encrypt independent ciphertexts
    auto ct1_a = cc.Encrypt(sk, 1);
    auto ct1_b = cc.Encrypt(sk, 1);  // Independent copy
    auto ct0_a = cc.Encrypt(sk, 0);
    auto ct0_b = cc.Encrypt(sk, 0);  // Independent copy

    // NAND truth table — lahat independent
    std::cout << "TRUTH TABLE:\n";
    auto nand_00 = cc.EvalBinGate(NAND, ct0_a, ct0_b);
    auto nand_01 = cc.EvalBinGate(NAND, ct0_a, ct1_a);
    auto nand_10 = cc.EvalBinGate(NAND, ct1_a, ct0_a);
    auto nand_11 = cc.EvalBinGate(NAND, ct1_a, ct1_b);

    std::cout << "  NAND(0,0) = " << decrypt_bit(nand_00) << " (expected 1)\n";
    std::cout << "  NAND(0,1) = " << decrypt_bit(nand_01) << " (expected 1)\n";
    std::cout << "  NAND(1,0) = " << decrypt_bit(nand_10) << " (expected 1)\n";
    std::cout << "  NAND(1,1) = " << decrypt_bit(nand_11) << " (expected 0)\n\n";

    // Benchmark: 100 NAND gates
    std::cout << "BENCHMARK: 100 NAND GATES\n";
    std::cout << "==========================\n\n";

    auto current = ct1_a;
    int errors = 0;
    int total_gates = 100;

    auto t_start = high_resolution_clock::now();

    for (int gate = 0; gate < total_gates; gate++) {
        // Para sa NAND(x,x), kailangan ng independent copy
        auto current_val = decrypt_bit(current);
        auto current_copy = cc.Encrypt(sk, current_val);
        current = cc.EvalBinGate(NAND, current, current_copy);
        
        int got = decrypt_bit(current);
        int expected = (gate % 2 == 0) ? 0 : 1;
        if (got != expected) errors++;
    }

    auto t_end = high_resolution_clock::now();
    auto total_ms = duration_cast<milliseconds>(t_end - t_start).count();

    std::cout << "  Errors: " << errors << "/" << total_gates << "\n";
    std::cout << "  Time: " << total_ms / 1000.0 << "s\n";
    std::cout << "  Per gate: " << (double)total_ms / total_gates << " ms\n";
    std::cout << "  Accuracy: " << (100.0 * (total_gates - errors) / total_gates) << "%\n\n";

    std::cout << "========================================\n";
    std::cout << "  CROSS-LIBRARY COMPARISON:\n";
    std::cout << "  CKKS:   100 gates, ~16936 ms/gate\n";
    std::cout << "  BinFHE: 100 gates, " << (double)total_ms / total_gates << " ms/gate\n";
    std::cout << "========================================\n";

    return 0;
}
