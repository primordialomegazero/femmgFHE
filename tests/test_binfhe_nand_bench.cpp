// BINFHE NAND BENCHMARK — Fixed Independent Ciphertexts
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

    // Encrypt — magkaibang encryption para sa 0 at 1
    auto ct1 = cc.Encrypt(sk, 1);
    auto ct0 = cc.Encrypt(sk, 0);

    // Helper para sa decrypt
    auto decrypt_bit = [&](auto ct) {
        LWEPlaintext result;
        cc.Decrypt(sk, ct, &result);
        return result;
    };

    // NAND truth table — gamitin ang independent ciphertexts
    std::cout << "TRUTH TABLE:\n";
    auto nand_00 = cc.EvalBinGate(NAND, ct0, ct0);  // Same ct0 — but this should work?
    std::cout << "  Trying NAND(0,0)...\n";
    
    // Kung pareho ang input, kailangan ng COPY
    auto ct0_copy = cc.Encrypt(sk, 0);
    auto ct1_copy = cc.Encrypt(sk, 1);
    
    auto nand_00b = cc.EvalBinGate(NAND, ct0, ct0_copy);
    auto nand_01 = cc.EvalBinGate(NAND, ct0, ct1);
    auto nand_10 = cc.EvalBinGate(NAND, ct1, ct0);
    auto nand_11 = cc.EvalBinGate(NAND, ct1, ct1_copy);

    std::cout << "  NAND(0,0) = " << decrypt_bit(nand_00b) << " (expected 1)\n";
    std::cout << "  NAND(0,1) = " << decrypt_bit(nand_01) << " (expected 1)\n";
    std::cout << "  NAND(1,0) = " << decrypt_bit(nand_10) << " (expected 1)\n";
    std::cout << "  NAND(1,1) = " << decrypt_bit(nand_11) << " (expected 0)\n\n";

    // Benchmark: 100 NAND gates
    std::cout << "BENCHMARK: 100 NAND GATES\n";
    std::cout << "==========================\n\n";

    auto current = ct1;
    int errors = 0;
    int total_gates = 100;

    auto t_start = high_resolution_clock::now();

    for (int gate = 0; gate < total_gates; gate++) {
        // Kailangan ng copy para sa parehong input
        auto current_copy = cc.Encrypt(sk, decrypt_bit(current));
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
    std::cout << "  Accuracy: " << (100.0 * (total_gates - errors) / total_gates) << "%\n";

    return 0;
}
