// 4-BIT MULTIPLIER — KARATSUBA (mas mababaw!)
#include "openfhe.h"
#include <iostream>
#include <vector>

using namespace lbcrypto;

int main() {
    std::cout << "4-BIT MULTIPLIER — KARATSUBA\n";
    std::cout << "============================\n\n";

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

    // 2-bit adder (proven na gumagana)
    auto add_2bit = [&](std::vector<Ciphertext<DCRTPoly>> A,
                         std::vector<Ciphertext<DCRTPoly>> B) {
        std::vector<Ciphertext<DCRTPoly>> S(3, ct0);
        S[0] = XOR(A[0], B[0]);
        auto c0 = AND(A[0], B[0]);
        S[1] = XOR(XOR(A[1], B[1]), c0);
        auto c1 = OR(AND(A[1], B[1]), AND(XOR(A[1], B[1]), c0));
        S[2] = c1;
        return S;
    };

    // 2-bit multiplier (proven na gumagana — 16/16)
    auto mult_2bit = [&](std::vector<Ciphertext<DCRTPoly>> A,
                          std::vector<Ciphertext<DCRTPoly>> B) {
        std::vector<Ciphertext<DCRTPoly>> R(4, ct0);
        auto pp00 = AND(A[0], B[0]);
        auto pp01 = AND(A[0], B[1]);
        auto pp10 = AND(A[1], B[0]);
        auto pp11 = AND(A[1], B[1]);
        R[0] = pp00;
        R[1] = XOR(pp01, pp10);
        auto c1 = AND(pp01, pp10);
        R[2] = XOR(pp11, c1);
        R[3] = AND(pp11, c1);
        return R;
    };

    // KARATSUBA 4-bit: Hatiin sa 2-bit halves
    // A = A1·x² + A0,  B = B1·x² + B0
    // A×B = A1·B1·x⁴ + (A1·B1 + A0·B0 + (A1+A0)·(B1+B0))·x² + A0·B0
    auto multiply_4bit_karatsuba = [&](std::vector<Ciphertext<DCRTPoly>> A,
                                        std::vector<Ciphertext<DCRTPoly>> B) {
        std::vector<Ciphertext<DCRTPoly>> R(8, ct0);

        // Split into 2-bit halves
        std::vector<Ciphertext<DCRTPoly>> A0 = {A[0], A[1]};
        std::vector<Ciphertext<DCRTPoly>> A1 = {A[2], A[3]};
        std::vector<Ciphertext<DCRTPoly>> B0 = {B[0], B[1]};
        std::vector<Ciphertext<DCRTPoly>> B1 = {B[2], B[3]};

        // 3 multiplications (imbes na 4)
        auto P0 = mult_2bit(A0, B0);  // A0·B0
        auto P1 = mult_2bit(A1, B1);  // A1·B1
        
        // (A1+A0) at (B1+B0) — XOR for addition (no carry sa Karatsuba middle)
        std::vector<Ciphertext<DCRTPoly>> A_sum = {XOR(A0[0], A1[0]), XOR(A0[1], A1[1])};
        std::vector<Ciphertext<DCRTPoly>> B_sum = {XOR(B0[0], B1[0]), XOR(B0[1], B1[1])};
        auto P2 = mult_2bit(A_sum, B_sum);  // (A1+A0)·(B1+B0)

        // Middle term: P2 + P0 + P1 (XOR lang muna)
        std::vector<Ciphertext<DCRTPoly>> mid(4, ct0);
        for (int i = 0; i < 4; i++) {
            mid[i] = XOR(XOR(P2[i], P0[i]), P1[i]);
        }

        // Combine: R = P0 + mid·x² + P1·x⁴
        for (int i = 0; i < 4; i++) {
            R[i] = XOR(R[i], P0[i]);
            R[i+2] = XOR(R[i+2], mid[i]);
            R[i+4] = XOR(R[i+4], P1[i]);
        }

        return R;
    };

    std::cout << "TESTING KARATSUBA 4-BIT MULTIPLIER:\n\n";
    
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

        auto R = multiply_4bit_karatsuba(A, B);

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
    
    if (passed == tests.size()) {
        std::cout << "  ✅ KARATSUBA WORKS!\n";
        std::cout << "  ✅ 4-BIT MULTIPLIER ACHIEVED!\n";
        std::cout << "  ✅ READY FOR 8-BIT!\n";
    }

    return 0;
}
