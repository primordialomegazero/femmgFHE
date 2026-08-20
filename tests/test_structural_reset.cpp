#include "openfhe.h"
#include <iostream>
#include <vector>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  STRUCTURAL RESET VIA ALGEBRAIC IDENTITY\n";
    std::cout << "  NAND(NAND(x,x), NAND(x,x)) = x\n";
    std::cout << "========================================\n\n";

    CCParams<CryptoContextBFVRNS> parameters;
    parameters.SetPlaintextModulus(65537);
    parameters.SetMultiplicativeDepth(20);
    parameters.SetRingDim(32768);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(parameters);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);

    auto keys = cc->KeyGen();
    cc->EvalMultKeyGen(keys.secretKey);

    auto ct0 = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext({0}));
    auto ct1 = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext({1}));

    auto decrypt = [&](Ciphertext<DCRTPoly> ct) -> int64_t {
        Plaintext pt;
        cc->Decrypt(keys.secretKey, ct, &pt);
        return pt->GetPackedValue()[0];
    };

    auto nand = [&](Ciphertext<DCRTPoly> a, Ciphertext<DCRTPoly> b) {
        auto ab = cc->EvalMult(a, b);
        return cc->EvalSub(ct1, ab);
    };

    std::cout << "1. STRUCTURAL IDENTITY VERIFICATION:\n";
    std::cout << "   NAND(NAND(x,x), NAND(x,x)) = x\n";
    std::cout << "-----------------------------------\n";

    int errors = 0;
    for (int x = 0; x <= 1; x++) {
        auto cx = x ? ct1 : ct0;
        auto inner = nand(cx, cx);
        auto outer = nand(inner, inner);
        int result = decrypt(outer);
        int expected = x;
        std::cout << "   x=" << x << " -> " << result
                  << " (expected " << expected << ") "
                  << (result == expected ? "OK" : "FAIL") << "\n";
        if (result != expected) errors++;
    }

    std::cout << "\n2. DEEP CHAIN WITH STRUCTURAL RESET:\n";
    std::cout << "-----------------------------------\n";

    auto current = ct0;
    int chain_errors = 0;

    for (int i = 1; i <= 20; i++) {
        current = nand(current, current);

        if (i % 2 == 0) {
            auto inner = nand(current, current);
            current = nand(inner, inner);
        }

        int expected = (i % 2 == 0) ? 0 : 1;
        int result = decrypt(current);
        if (result != expected) {
            chain_errors++;
            if (chain_errors <= 10) {
                std::cout << "   Iteration " << i << ": " << result
                          << " (expected " << expected << ") FAIL\n";
            }
        }
    }

    std::cout << "   Chain errors: " << chain_errors << "\n";

    std::cout << "\n3. PAIRED NAND AS SINGLE OPERATION:\n";
    std::cout << "-----------------------------------\n";

    auto pair_reset = [&](Ciphertext<DCRTPoly> x) {
        auto inner = nand(x, x);
        return nand(inner, inner);
    };

    current = ct0;
    int pair_errors = 0;

    for (int i = 1; i <= 20; i++) {
        current = nand(current, current);
        current = pair_reset(current);

        int expected = (i % 2 == 0) ? 0 : 1;
        int result = decrypt(current);
        if (result != expected) {
            pair_errors++;
            if (pair_errors <= 10) {
                std::cout << "   Iteration " << i << ": " << result
                          << " (expected " << expected << ") FAIL\n";
            }
        }
    }

    std::cout << "   Pair errors: " << pair_errors << "\n";

    std::cout << "\n========================================\n";
    std::cout << "  SUMMARY:\n";
    std::cout << "  - Identity errors: " << errors << "\n";
    std::cout << "  - Chain errors: " << chain_errors << "\n";
    std::cout << "  - Pair errors: " << pair_errors << "\n";
    std::cout << "========================================\n";

    return 0;
}
