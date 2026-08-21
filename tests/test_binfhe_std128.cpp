// BINFHE STD128 — Production Parameters
// Hindi na TOY — real security level
// I-test kung gumagana sa mas malalaking parameters

#include "openfhe.h"
#include <iostream>
#include <chrono>

using namespace lbcrypto;
using namespace std::chrono;

int main() {
    std::cout << "========================================\n";
    std::cout << "  BINFHE STD128 — PRODUCTION\n";
    std::cout << "========================================\n\n";

    auto cc = BinFHEContext();

    // Subukan ang iba't ibang security levels
    for (auto level : {TOY, MEDIUM, STD128}) {
        std::cout << "Trying security level: ";
        if (level == TOY) std::cout << "TOY";
        else if (level == MEDIUM) std::cout << "MEDIUM";
        else std::cout << "STD128";
        std::cout << "... ";
        std::cout.flush();

        try {
            auto t1 = high_resolution_clock::now();
            cc.GenerateBinFHEContext(level, GINX);
            auto sk = cc.KeyGen();
            cc.BTKeyGen(sk);
            auto t2 = high_resolution_clock::now();

            auto ms = duration_cast<milliseconds>(t2 - t1).count();
            std::cout << "OK (setup: " << ms / 1000.0 << "s)\n";

            // Quick NAND test
            auto ct1 = cc.Encrypt(sk, 1);
            auto ct0 = cc.Encrypt(sk, 0);
            auto nand_result = cc.EvalBinGate(NAND, ct1, ct0);
            LWEPlaintext result;
            cc.Decrypt(sk, nand_result, &result);
            std::cout << "  NAND(1,0) = " << result << " (expected 1)\n\n";

        } catch (std::exception& e) {
            std::cout << "FAIL: " << e.what() << "\n\n";
        }
    }

    return 0;
}
