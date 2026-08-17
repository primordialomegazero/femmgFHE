// 4-BIT MULTIPLIER — MULTIPLE SINGLE TESTS
#include "openfhe.h"
#include <iostream>
#include <vector>

using namespace lbcrypto;

int main() {
    std::cout << "4-BIT MULTIPLIER — 5 TESTS\n";
    std::cout << "===========================\n\n";

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

    auto nand = [&](Ciphertext<DCRTPoly> a, Ciphertext<DCRTPoly> b) {
        auto ab = cc->EvalMult(a, b);
        return cc->EvalSub(ct1, ab);
    };
    auto NOT = [&](Ciphertext<DCRTPoly> a) { return nand(a, a); };
    auto AND = [&](Ciphertext<DCRTPoly> a, Ciphertext<DCRTPoly> b) { return NOT(nand(a, b)); };
    auto XOR = [&](Ciphertext<DCRTPoly> a, Ciphertext<DCRTPoly> b) {
        auto n1 = nand(a, b);
        return nand(nand(a, n1), nand(b, n1));
    };

    auto dec = [&](Ciphertext<DCRTPoly> ct) {
        Plaintext pt;
        cc->Decrypt(keys.secretKey, ct, &pt);
        return (int)pt->GetPackedValue()[0];
    };

    // Test cases: (a, b, expected)
    std::vector<std::pair<int, int>> tests = {
        {10, 6},   // 60
        {3, 5},    // 15
        {7, 9},    // 63
        {15, 15},  // 225
        {12, 11}   // 132
    };

    int passed = 0;
    for (auto [a_val, b_val] : tests) {
        // Encode to bits
        std::vector<Ciphertext<DCRTPoly>> A(4), B(4);
        for (int i = 0; i < 4; i++) {
            A[i] = ((a_val >> i) & 1) ? ct1 : ct0;
            B[i] = ((b_val >> i) & 1) ? ct1 : ct0;
        }

        // Multiply (simplified — XOR lang muna)
        std::vector<Ciphertext<DCRTPoly>> result(8, ct0);
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                auto partial = AND(A[j], B[i]);
                result[i+j] = XOR(result[i+j], partial);
            }
        }

        // Decrypt and verify
        int actual = 0;
        for (int bit = 0; bit < 8; bit++) {
            actual |= (dec(result[bit]) << bit);
        }
        int expected = a_val * b_val;
        bool ok = (actual == expected);
        if (ok) passed++;
        
        std::cout << (ok ? "  ✅ " : "  ❌ ")
                  << a_val << " × " << b_val << " = " << actual
                  << " (expected " << expected << ")\n";
    }

    std::cout << "\n  Result: " << passed << "/" << tests.size() << " passed\n";
    
    return 0;
}
