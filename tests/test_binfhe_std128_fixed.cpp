// BINFHE STD128 — Fixed BTKeyGen
// Hiwalay na context para sa bawat security level

#include "openfhe.h"
#include <iostream>
#include <chrono>

using namespace lbcrypto;
using namespace std::chrono;

int main() {
    std::cout << "========================================\n";
    std::cout << "  BINFHE STD128 — FIXED\n";
    std::cout << "========================================\n\n";

    // Hiwalay na function para sa bawat level
    auto test_level = [](BINFHE_PARAMSET level, const char* name) {
        std::cout << "Testing " << name << "... ";
        std::cout.flush();

        try {
            auto cc = BinFHEContext();  // BAGONG context bawat level!
            auto t1 = high_resolution_clock::now();
            cc.GenerateBinFHEContext(level, GINX);
            auto sk = cc.KeyGen();
            cc.BTKeyGen(sk);
            auto t2 = high_resolution_clock::now();

            auto ms = duration_cast<milliseconds>(t2 - t1).count();
            std::cout << "OK (setup: " << ms / 1000.0 << "s)";

            // NAND test
            auto ct1 = cc.Encrypt(sk, 1);
            auto ct0 = cc.Encrypt(sk, 0);
            auto nand_result = cc.EvalBinGate(NAND, ct1, ct0);
            LWEPlaintext result;
            cc.Decrypt(sk, nand_result, &result);
            std::cout << " NAND(1,0)=" << result << " ✓\n";
        } catch (std::exception& e) {
            std::cout << "FAIL: " << e.what() << "\n";
        }
    };

    test_level(TOY, "TOY");
    test_level(MEDIUM, "MEDIUM");
    test_level(STD128, "STD128");

    return 0;
}
