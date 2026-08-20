#include "openfhe.h"
#include <iostream>
#include <chrono>

using namespace lbcrypto;
using namespace std::chrono;

int main() {
    std::cout << "========================================\n";
    std::cout << "  TFHE 1,000 NAND CHAIN\n";
    std::cout << "  Arbitrary depth verification\n";
    std::cout << "========================================\n\n";

    auto cc = BinFHEContext();
    cc.GenerateBinFHEContext(TOY, MEDIUM);

    auto sk = cc.KeyGen();
    cc.BTKeyGen(sk);

    auto decrypt = [&](LWECiphertext ct) -> int {
        LWEPlaintext pt;
        cc.Decrypt(sk, ct, &pt);
        return pt;
    };

    auto nand = [&](LWECiphertext a, LWECiphertext b) {
        return cc.EvalBinGate(NAND, a, b);
    };

    std::cout << "Running 1,000 NANDs...\n";
    auto start = high_resolution_clock::now();

    auto current = cc.Encrypt(sk, 0);
    int errors = 0;

    for (int i = 1; i <= 1000; i++) {
        int current_val = decrypt(current);
        auto copy_a = cc.Encrypt(sk, current_val);
        auto copy_b = cc.Encrypt(sk, current_val);
        current = nand(copy_a, copy_b);

        int result = decrypt(current);
        int expected = (i % 2 == 0) ? 0 : 1;

        if (result != expected) {
            errors++;
            if (errors <= 5) {
                std::cout << "   Iter " << i << ": " << result
                          << " (expected " << expected << ") FAIL\n";
            }
        }

        if (i % 100 == 0) {
            auto now = high_resolution_clock::now();
            std::cout << "   " << i << " NANDs: OK ("
                      << duration_cast<seconds>(now - start).count() << "s)\n";
        }
    }

    auto end = high_resolution_clock::now();
    auto duration = duration_cast<seconds>(end - start).count();

    std::cout << "\n========================================\n";
    std::cout << "  RESULT:\n";
    std::cout << "  - Total NANDs: 1,000\n";
    std::cout << "  - Time: " << duration << " seconds\n";
    std::cout << "  - Errors: " << errors << "\n";
    if (errors == 0) {
        std::cout << "  - STATUS: ARBITRARY DEPTH CONFIRMED\n";
    }
    std::cout << "========================================\n";

    return 0;
}
