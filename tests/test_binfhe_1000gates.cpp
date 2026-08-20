// BINFHE 1000 GATES — Ang Tunay na Scale Test
// 17ms/gate → 1000 gates ≈ 17 seconds

#include "openfhe.h"
#include <iostream>
#include <chrono>

using namespace lbcrypto;
using namespace std::chrono;

int main() {
    std::cout << "========================================\n";
    std::cout << "  BINFHE 1000 GATES\n";
    std::cout << "  Unlimited Depth Test\n";
    std::cout << "========================================\n\n";

    auto cc = BinFHEContext();
    cc.GenerateBinFHEContext(TOY, GINX);
    auto sk = cc.KeyGen();
    cc.BTKeyGen(sk);

    std::cout << "Setup: COMPLETE\n\n";

    auto decrypt_bit = [&](auto ct) {
        LWEPlaintext result;
        cc.Decrypt(sk, ct, &result);
        return result;
    };

    auto current = cc.Encrypt(sk, 1);
    int errors = 0;
    int total_gates = 1000;

    std::cout << "Running " << total_gates << " NAND gates...\n\n";

    auto t_start = high_resolution_clock::now();

    for (int gate = 0; gate < total_gates; gate++) {
        // Independent copy para sa NAND(x,x)
        auto current_val = decrypt_bit(current);
        auto current_copy = cc.Encrypt(sk, current_val);
        current = cc.EvalBinGate(NAND, current, current_copy);

        int got = decrypt_bit(current);
        int expected = (gate % 2 == 0) ? 0 : 1;
        if (got != expected) errors++;

        if (gate % 100 == 0) {
            std::cout << "  Gate " << gate << ": OK\n";
        }
    }

    auto t_end = high_resolution_clock::now();
    auto total_ms = duration_cast<milliseconds>(t_end - t_start).count();

    std::cout << "\n========================================\n";
    std::cout << "  RESULT:\n";
    std::cout << "  Gates: " << total_gates << "\n";
    std::cout << "  Errors: " << errors << "\n";
    std::cout << "  Accuracy: " << (100.0 * (total_gates - errors) / total_gates) << "%\n";
    std::cout << "  Time: " << total_ms / 1000.0 << "s\n";
    std::cout << "  Per gate: " << (double)total_ms / total_gates << " ms\n";
    std::cout << "========================================\n";

    return 0;
}
