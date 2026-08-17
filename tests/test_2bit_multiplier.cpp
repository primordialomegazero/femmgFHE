// 2-BIT MULTIPLIER — FULL CARRY (baseline muna)
#include "openfhe.h"
#include <iostream>
#include <vector>

using namespace lbcrypto;

int main() {
    std::cout << "2-BIT MULTIPLIER — FULL CARRY\n";
    std::cout << "==============================\n\n";

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

    // 2-bit multiplier: A[1:0] × B[1:0] = R[3:0]
    auto multiply_2bit = [&](std::vector<Ciphertext<DCRTPoly>> A,
                              std::vector<Ciphertext<DCRTPoly>> B) {
        std::vector<Ciphertext<DCRTPoly>> R(4, ct0);

        // Partial products
        auto pp00 = AND(A[0], B[0]);
        auto pp01 = AND(A[0], B[1]);
        auto pp10 = AND(A[1], B[0]);
        auto pp11 = AND(A[1], B[1]);

        // Bit 0: pp00
        R[0] = pp00;

        // Bit 1: pp01 + pp10 (with carry)
        R[1] = XOR(pp01, pp10);
        auto carry1 = AND(pp01, pp10);

        // Bit 2: pp11 + carry1
        R[2] = XOR(pp11, carry1);
        auto carry2 = AND(pp11, carry1);

        // Bit 3: carry2
        R[3] = carry2;

        return R;
    };

    std::cout << "TESTING ALL 16 COMBINATIONS:\n\n";
    
    int passed = 0;
    int total = 0;
    for (int a = 0; a < 4; a++) {
        for (int b = 0; b < 4; b++) {
            std::vector<Ciphertext<DCRTPoly>> A(2), B(2);
            A[0] = (a & 1) ? ct1 : ct0;
            A[1] = (a & 2) ? ct1 : ct0;
            B[0] = (b & 1) ? ct1 : ct0;
            B[1] = (b & 2) ? ct1 : ct0;

            auto R = multiply_2bit(A, B);

            int actual = 0;
            for (int bit = 0; bit < 4; bit++) {
                actual |= (dec(R[bit]) << bit);
            }
            int expected = a * b;
            bool ok = (actual == expected);
            if (ok) passed++;
            total++;
            
            std::cout << (ok ? "  ✅ " : "  ❌ ")
                      << a << " × " << b << " = " << actual
                      << " (expected " << expected << ")\n";
        }
    }

    std::cout << "\n  Result: " << passed << "/" << total << " passed\n";
    
    if (passed == total) {
        std::cout << "  ✅ 2-BIT MULTIPLIER FULLY WORKS!\n";
        std::cout << "  ✅ READY FOR 4-BIT EXTENSION!\n";
    }

    return 0;
}
