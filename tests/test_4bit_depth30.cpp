// 4-BIT MULTIPLIER — DEPTH 30 (taasan ang budget)
#include "openfhe.h"
#include <iostream>
#include <vector>

using namespace lbcrypto;

int main() {
    std::cout << "4-BIT MULTIPLIER — DEPTH 30\n";
    std::cout << "===========================\n\n";

    CCParams<CryptoContextBFVRNS> parameters;
    parameters.SetPlaintextModulus(65537);
    parameters.SetMultiplicativeDepth(30);  // Taasan!
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
    auto OR = [&](Ciphertext<DCRTPoly> a, Ciphertext<DCRTPoly> b) { return nand(NOT(a), NOT(b)); };
    auto XOR = [&](Ciphertext<DCRTPoly> a, Ciphertext<DCRTPoly> b) {
        auto n1 = nand(a, b);
        return nand(nand(a, n1), nand(b, n1));
    };

    auto dec = [&](Ciphertext<DCRTPoly> ct) {
        Plaintext pt;
        cc->Decrypt(keys.secretKey, ct, &pt);
        return (int)pt->GetPackedValue()[0];
    };

    // 4-bit multiplier — full carry (galing sa 2-bit na proven)
    auto multiply_4bit = [&](std::vector<Ciphertext<DCRTPoly>> A,
                              std::vector<Ciphertext<DCRTPoly>> B) {
        std::vector<Ciphertext<DCRTPoly>> R(8, ct0);

        // Partial products
        auto pp00 = AND(A[0], B[0]);
        auto pp01 = AND(A[0], B[1]);
        auto pp02 = AND(A[0], B[2]);
        auto pp03 = AND(A[0], B[3]);
        auto pp10 = AND(A[1], B[0]);
        auto pp11 = AND(A[1], B[1]);
        auto pp12 = AND(A[1], B[2]);
        auto pp13 = AND(A[1], B[3]);
        auto pp20 = AND(A[2], B[0]);
        auto pp21 = AND(A[2], B[1]);
        auto pp22 = AND(A[2], B[2]);
        auto pp23 = AND(A[2], B[3]);
        auto pp30 = AND(A[3], B[0]);
        auto pp31 = AND(A[3], B[1]);
        auto pp32 = AND(A[3], B[2]);
        auto pp33 = AND(A[3], B[3]);

        // Column additions with carry
        R[0] = pp00;

        // Column 1
        R[1] = XOR(pp01, pp10);
        auto c1 = AND(pp01, pp10);

        // Column 2
        auto s2 = XOR(XOR(pp02, pp11), pp20);
        auto c2a = OR(OR(AND(pp02, pp11), AND(pp02, pp20)), AND(pp11, pp20));
        R[2] = XOR(s2, c1);
        auto c2 = OR(AND(s2, c1), c2a);

        // Column 3
        auto s3 = XOR(XOR(pp03, pp12), XOR(pp21, pp30));
        auto c3a = OR(OR(AND(pp03, pp12), AND(pp03, pp21)), 
                     OR(AND(pp03, pp30), OR(AND(pp12, pp21), 
                     OR(AND(pp12, pp30), AND(pp21, pp30)))));
        R[3] = XOR(s3, c2);
        auto c3 = OR(AND(s3, c2), c3a);

        // Column 4
        auto s4 = XOR(XOR(pp13, pp22), pp31);
        auto c4a = OR(OR(AND(pp13, pp22), AND(pp13, pp31)), AND(pp22, pp31));
        R[4] = XOR(s4, c3);
        auto c4 = OR(AND(s4, c3), c4a);

        // Column 5
        auto s5 = XOR(pp23, pp32);
        R[5] = XOR(s5, c4);
        auto c5 = OR(AND(s5, c4), AND(pp23, pp32));

        // Column 6
        R[6] = XOR(pp33, c5);
        R[7] = AND(pp33, c5);

        return R;
    };

    std::cout << "TESTING 4-BIT MULTIPLIER (5 cases):\n\n";
    
    std::vector<std::pair<int, int>> tests = {
        {10, 6},   // 60
        {3, 5},    // 15
        {7, 9},    // 63
        {15, 15},  // 225
        {12, 11}   // 132
    };

    int passed = 0;
    for (auto [a_val, b_val] : tests) {
        std::vector<Ciphertext<DCRTPoly>> A(4), B(4);
        for (int i = 0; i < 4; i++) {
            A[i] = ((a_val >> i) & 1) ? ct1 : ct0;
            B[i] = ((b_val >> i) & 1) ? ct1 : ct0;
        }

        auto R = multiply_4bit(A, B);

        int actual = 0;
        for (int bit = 0; bit < 8; bit++) {
            actual |= (dec(R[bit]) << bit);
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
