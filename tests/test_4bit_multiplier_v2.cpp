// 4-BIT MULTIPLIER — WITH CARRY PROPAGATION
#include "openfhe.h"
#include <iostream>
#include <vector>

using namespace lbcrypto;

int main() {
    std::cout << "4-BIT MULTIPLIER V2 (With Carry)\n";
    std::cout << "===============================\n\n";

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

    auto full_adder = [&](Ciphertext<DCRTPoly> a, Ciphertext<DCRTPoly> b, 
                           Ciphertext<DCRTPoly> cin) {
        auto sum1 = XOR(a, b);
        auto sum = XOR(sum1, cin);
        auto carry1 = AND(a, b);
        auto carry2 = AND(sum1, cin);
        auto carry = OR(carry1, carry2);
        return std::make_pair(sum, carry);
    };

    auto dec = [&](Ciphertext<DCRTPoly> ct) {
        Plaintext pt;
        cc->Decrypt(keys.secretKey, ct, &pt);
        return (int)pt->GetPackedValue()[0];
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
        // Encode inputs
        std::vector<Ciphertext<DCRTPoly>> A(4), B(4);
        for (int i = 0; i < 4; i++) {
            A[i] = ((a_val >> i) & 1) ? ct1 : ct0;
            B[i] = ((b_val >> i) & 1) ? ct1 : ct0;
        }

        // Multiply with carry propagation
        std::vector<Ciphertext<DCRTPoly>> result(8, ct0);
        
        for (int i = 0; i < 4; i++) {
            Ciphertext<DCRTPoly> carry = ct0;
            for (int j = 0; j < 4; j++) {
                auto partial = AND(A[j], B[i]);
                auto sum = XOR(result[i+j], partial);
                auto carry1 = AND(result[i+j], partial);
                auto carry2 = AND(sum, carry);
                auto new_carry = OR(carry1, carry2);
                result[i+j] = XOR(sum, carry);
                carry = new_carry;
            }
            result[i+4] = carry;
        }

        // Decrypt
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
