// PURE BINFHE NAND CHAIN — Walang decrypt-reencrypt sa gitna
#include "openfhe.h"
#include <iostream>
#include <chrono>

using namespace lbcrypto;
using namespace std::chrono;

int main() {
    std::cout << "========================================\n";
    std::cout << "  PURE BINFHE NAND CHAIN\n";
    std::cout << "  No decrypt-reencrypt\n";
    std::cout << "========================================\n\n";

    auto cc = BinFHEContext();
    cc.GenerateBinFHEContext(TOY, GINX);
    auto sk = cc.KeyGen();
    cc.BTKeyGen(sk);

    auto decrypt_bit = [&](auto ct) {
        LWEPlaintext result;
        cc.Decrypt(sk, ct, &result);
        return result;
    };

    int errors = 0;
    int total_gates = 1000;

    auto current = cc.Encrypt(sk, 1);

    for (int gate = 0; gate < total_gates; gate++) {
        current = cc.EvalBinGate(NAND, current, current);

        if (gate % 100 == 0) {
            int got = decrypt_bit(current);
            int expected = (gate % 2 == 0) ? 0 : 1;
            std::cout << "Gate " << gate << ": got=" << got
                      << " expected=" << expected
                      << (got == expected ? " ✓" : " ✗") << "\n";
            if (got != expected) errors++;
        }
    }

    std::cout << "\n========================================\n";
    std::cout << "  Errors: " << errors << "\n";
    std::cout << "========================================\n";

    return 0;
}
