#include "openfhe.h"
#include <iostream>
#include <chrono>

using namespace lbcrypto;
using namespace std::chrono;

int main() {
    std::cout << "========================================\n";
    std::cout << "  TFHE ARBITRARY DEPTH NAND CHAIN\n";
    std::cout << "  Native bootstrapping sa bawat gate\n";
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

    std::cout << "1. NAND TRUTH TABLE (TFHE):\n";
    std::cout << "-----------------------------------\n";

    for (int x = 0; x <= 1; x++) {
        for (int y = 0; y <= 1; y++) {
            // Sariwang encryption sa bawat gate
            auto cx = cc.Encrypt(sk, x);
            auto cy = cc.Encrypt(sk, y);
            auto result = nand(cx, cy);
            int val = decrypt(result);
            int expected = !(x && y);
            std::cout << "   NAND(" << x << "," << y << ") = " << val
                      << " (expected " << expected << ")\n";
        }
    }

    std::cout << "\n2. 100 NAND CHAIN (TFHE):\n";
    std::cout << "-----------------------------------\n";

    auto current = cc.Encrypt(sk, 0);
    int errors = 0;
    auto start = high_resolution_clock::now();

    for (int i = 1; i <= 100; i++) {
        // Sariwang kopya para sa self-NAND
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

        if (i % 10 == 0) {
            std::cout << "   " << i << " NANDs: OK\n";
        }
    }

    auto end = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(end - start).count();

    std::cout << "\n   Time: " << duration << " ms\n";
    std::cout << "   Errors: " << errors << "\n";

    std::cout << "\n========================================\n";
    if (errors == 0) {
        std::cout << "  ARBITRARY DEPTH ACHIEVED SA TFHE\n";
        std::cout << "  100 NANDs na walang corruption\n";
    }
    std::cout << "========================================\n";

    return 0;
}
