// BINFHE φ-PATTERN ANALYSIS — FIXED
// May independent ciphertexts

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <chrono>
#include <cmath>

using namespace lbcrypto;
using namespace std::chrono;

int main() {
    std::cout << "========================================\n";
    std::cout << "  BINFHE φ-PATTERN ANALYSIS V2\n";
    std::cout << "========================================\n\n";

    const double phi = 1.6180339887498948482;
    const double psi = 1.0 / phi;

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

    // ============================================
    // 1. NAND SA BINFHE — TRUTH TABLE
    // ============================================
    std::cout << "1. NAND TRUTH TABLE:\n";
    std::cout << "====================\n\n";

    auto ct0_a = cc.Encrypt(sk, 0);
    auto ct0_b = cc.Encrypt(sk, 0);
    auto ct1_a = cc.Encrypt(sk, 1);
    auto ct1_b = cc.Encrypt(sk, 1);

    auto nand_00 = cc.EvalBinGate(NAND, ct0_a, ct0_b);
    auto nand_01 = cc.EvalBinGate(NAND, ct0_a, ct1_a);
    auto nand_10 = cc.EvalBinGate(NAND, ct1_a, ct0_a);
    auto nand_11 = cc.EvalBinGate(NAND, ct1_a, ct1_b);

    std::cout << "  NAND(0,0) = " << decrypt_bit(nand_00) << " (expected 1)\n";
    std::cout << "  NAND(0,1) = " << decrypt_bit(nand_01) << " (expected 1)\n";
    std::cout << "  NAND(1,0) = " << decrypt_bit(nand_10) << " (expected 1)\n";
    std::cout << "  NAND(1,1) = " << decrypt_bit(nand_11) << " (expected 0)\n\n";

    // ============================================
    // 2. PERIOD-2 SA BINFHE
    // ============================================
    std::cout << "2. PERIOD-2 SA BINFHE:\n";
    std::cout << "======================\n\n";

    auto current = cc.Encrypt(sk, 1);
    for (int gate = 0; gate < 10; gate++) {
        auto current_val = decrypt_bit(current);
        auto current_copy = cc.Encrypt(sk, current_val);
        current = cc.EvalBinGate(NAND, current, current_copy);
        
        int got = decrypt_bit(current);
        int expected = (gate % 2 == 0) ? 0 : 1;
        
        if (gate < 5) {
            std::cout << "  Gate " << gate << ": " << got
                      << " (expected " << expected << ")"
                      << (got == expected ? " ✓" : " ✗") << "\n";
        }
    }
    std::cout << "\n";

    // ============================================
    // 3. TIMING ANALYSIS — MAY φ-RATIO BA?
    // ============================================
    std::cout << "3. TIMING ANALYSIS:\n";
    std::cout << "===================\n\n";

    std::vector<double> gate_times;
    int total_gates = 100;

    for (int gate = 0; gate < total_gates; gate++) {
        auto t1 = high_resolution_clock::now();
        auto current_val = decrypt_bit(current);
        auto current_copy = cc.Encrypt(sk, current_val);
        current = cc.EvalBinGate(NAND, current, current_copy);
        auto t2 = high_resolution_clock::now();
        
        double gate_time = duration_cast<microseconds>(t2 - t1).count() / 1000.0;
        gate_times.push_back(gate_time);
    }

    double avg_time = 0;
    for (auto t : gate_times) avg_time += t;
    avg_time /= gate_times.size();

    std::cout << "  Average gate time: " << avg_time << " ms\n";
    std::cout << "  φ × avg_time = " << (phi * avg_time) << " ms\n";
    std::cout << "  ψ × avg_time = " << (psi * avg_time) << " ms\n\n";

    // ============================================
    // 4. EMERGENT PROPERTY CHECK
    // ============================================
    std::cout << "4. EMERGENT PROPERTY CHECK:\n";
    std::cout << "===========================\n\n";

    std::cout << "  Ang NAND ay period-2: NOT(NOT(x)) = x\n";
    std::cout << "  Ang bootstrapping ay period-1 refresh\n";
    std::cout << "  Ang φ² = φ+1 ay may self-similarity\n\n";

    std::cout << "  Kaya sa BinFHE:\n";
    std::cout << "  - NAND values: period-2 (φ-like rhythm)\n";
    std::cout << "  - Bootstrapping: period-1 (linear refresh)\n";
    std::cout << "  - Combination: φ-periodicity?\n\n";

    std::cout << "  Kung ang bootstrapping ay gagawing φ-spaced,\n";
    std::cout << "  maaaring mabawasan ang refresh cost.\n";

    return 0;
}
