// BINFHE ALTERNATING φ-ψ BOOTSTRAPPING
// Subukan kung ang alternating refresh ay may
// ibang timing o noise behavior

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <chrono>
#include <cmath>

using namespace lbcrypto;
using namespace std::chrono;

int main() {
    std::cout << "========================================\n";
    std::cout << "  BINFHE ALTERNATING φ-ψ\n";
    std::cout << "========================================\n\n";

    const double phi = 1.6180339887498948482;

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
    // TEST 1: 100 GATES — STANDARD BOOTSTRAPPING
    // ============================================
    std::cout << "TEST 1: 100 GATES STANDARD\n";
    std::cout << "==========================\n\n";

    auto current = cc.Encrypt(sk, 1);
    int errors = 0;
    auto t_start = high_resolution_clock::now();

    for (int gate = 0; gate < 100; gate++) {
        auto current_val = decrypt_bit(current);
        auto current_copy = cc.Encrypt(sk, current_val);
        current = cc.EvalBinGate(NAND, current, current_copy);
        
        int got = decrypt_bit(current);
        int expected = (gate % 2 == 0) ? 0 : 1;
        if (got != expected) errors++;
    }

    auto t_end = high_resolution_clock::now();
    auto standard_time = duration_cast<milliseconds>(t_end - t_start).count();

    std::cout << "  Errors: " << errors << "/100\n";
    std::cout << "  Time: " << standard_time << " ms\n\n";

    // ============================================
    // TEST 2: 100 GATES — MANUAL ALTERNATING
    // ============================================
    std::cout << "TEST 2: 100 GATES ALTERNATING\n";
    std::cout << "==============================\n\n";

    current = cc.Encrypt(sk, 1);
    errors = 0;
    t_start = high_resolution_clock::now();

    for (int gate = 0; gate < 100; gate++) {
        auto current_val = decrypt_bit(current);
        auto current_copy = cc.Encrypt(sk, current_val);
        
        // Alternating: minsan NAND, minsan NOR
        if (gate % 2 == 0) {
            current = cc.EvalBinGate(NAND, current, current_copy);
        } else {
            // NOR = NOT(OR) = NOT(NOT(NAND)) = NAND
            // Sa BinFHE, pareho lang ang NAND
            current = cc.EvalBinGate(NAND, current, current_copy);
        }
        
        int got = decrypt_bit(current);
        int expected = (gate % 2 == 0) ? 0 : 1;
        if (got != expected) errors++;
    }

    t_end = high_resolution_clock::now();
    auto alt_time = duration_cast<milliseconds>(t_end - t_start).count();

    std::cout << "  Errors: " << errors << "/100\n";
    std::cout << "  Time: " << alt_time << " ms\n\n";

    // ============================================
    // COMPARISON
    // ============================================
    std::cout << "COMPARISON:\n";
    std::cout << "============\n\n";
    std::cout << "  Standard: " << standard_time << " ms\n";
    std::cout << "  Alternating: " << alt_time << " ms\n";
    std::cout << "  Ratio: " << (double)standard_time / alt_time << "\n";
    std::cout << "  φ = " << phi << "\n\n";

    if (std::abs((double)standard_time / alt_time - phi) < 0.1) {
        std::cout << "  ✓ MAY φ-RATIO SA TIMING!\n";
    } else {
        std::cout << "  ✗ Walang φ-ratio sa timing\n";
    }

    return 0;
}
