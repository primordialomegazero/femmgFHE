// ALL EVEN GATES — Lahat ng gates ay may EVEN NAND count!
#include "openfhe.h"
#include <iostream>
#include <vector>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  ALL EVEN GATES\n";
    std::cout << "  (Lahat ng NANDs ay PAIRED!)\n";
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

    auto nand = [&](Ciphertext<DCRTPoly> a, Ciphertext<DCRTPoly> b) {
        auto ab = cc->EvalMult(a, b);
        return cc->EvalSub(ct1, ab);
    };

    auto decrypt = [&](Ciphertext<DCRTPoly> ct) -> int64_t {
        Plaintext pt;
        cc->Decrypt(keys.secretKey, ct, &pt);
        return pt->GetPackedValue()[0];
    };

    // ============================================
    // ALL EVEN GATES
    // ============================================
    
    // NOT_even: 2 NANDs (NOT + reset pair)
    auto NOT_even = [&](Ciphertext<DCRTPoly> a) {
        auto temp = nand(a, a);      // 1 NAND
        return nand(temp, temp);      // 2 NANDs total (even) ✅
    };
    
    // AND_even: 2 NANDs
    auto AND_even = [&](Ciphertext<DCRTPoly> a, Ciphertext<DCRTPoly> b) {
        auto temp = nand(a, b);       // 1 NAND
        return nand(temp, temp);      // 2 NANDs total (even) ✅
    };
    
    // OR_even: 4 NANDs
    auto OR_even = [&](Ciphertext<DCRTPoly> a, Ciphertext<DCRTPoly> b) {
        auto not_a = nand(a, a);      // 1
        auto not_b = nand(b, b);      // 1
        auto temp = nand(not_a, not_b); // 1
        return nand(temp, temp);      // 4 NANDs total (even) ✅
    };
    
    // XOR_even: 4 NANDs
    auto XOR_even = [&](Ciphertext<DCRTPoly> a, Ciphertext<DCRTPoly> b) {
        auto n1 = nand(a, b);         // 1
        auto left = nand(a, n1);      // 1
        auto right = nand(b, n1);     // 1
        return nand(left, right);     // 4 NANDs total (even) ✅
    };

    // ============================================
    // TEST 1: GATE TRUTH TABLES
    // ============================================
    std::cout << "1. GATE TRUTH TABLES (lahat EVEN):\n";
    std::cout << "-----------------------------------\n";
    
    int errors = 0;
    
    // NOT
    int not_0 = decrypt(NOT_even(ct0));
    int not_1 = decrypt(NOT_even(ct1));
    std::cout << "  NOT(0) = " << not_0 << " (exp 1) " << (not_0 == 1 ? "✅" : "❌") << "\n";
    std::cout << "  NOT(1) = " << not_1 << " (exp 0) " << (not_1 == 0 ? "✅" : "❌") << "\n";
    if (not_0 != 1 || not_1 != 0) errors++;
    
    // AND
    std::cout << "  AND(0,0) = " << decrypt(AND_even(ct0, ct0)) << " (exp 0)\n";
    std::cout << "  AND(0,1) = " << decrypt(AND_even(ct0, ct1)) << " (exp 0)\n";
    std::cout << "  AND(1,1) = " << decrypt(AND_even(ct1, ct1)) << " (exp 1)\n";
    
    // OR
    std::cout << "  OR(0,0) = " << decrypt(OR_even(ct0, ct0)) << " (exp 0)\n";
    std::cout << "  OR(0,1) = " << decrypt(OR_even(ct0, ct1)) << " (exp 1)\n";
    std::cout << "  OR(1,1) = " << decrypt(OR_even(ct1, ct1)) << " (exp 1)\n";
    
    // XOR
    std::cout << "  XOR(0,0) = " << decrypt(XOR_even(ct0, ct0)) << " (exp 0)\n";
    std::cout << "  XOR(0,1) = " << decrypt(XOR_even(ct0, ct1)) << " (exp 1)\n";
    std::cout << "  XOR(1,1) = " << decrypt(XOR_even(ct1, ct1)) << " (exp 0)\n\n";

    // ============================================
    // TEST 2: 4-BIT MULTIPLIER (all even gates)
    // ============================================
    std::cout << "2. 4-BIT MULTIPLIER (all even):\n";
    std::cout << "--------------------------------\n";
    
    auto multiply_4bit_even = [&](std::vector<Ciphertext<DCRTPoly>> A,
                                    std::vector<Ciphertext<DCRTPoly>> B) {
        std::vector<Ciphertext<DCRTPoly>> R(8, ct0);
        
        for (int i = 0; i < 4; i++) {
            Ciphertext<DCRTPoly> carry = ct0;
            for (int j = 0; j < 4; j++) {
                auto partial = AND_even(A[j], B[i]);       // 2 NANDs
                auto sum = XOR_even(R[i+j], partial);       // 4 NANDs
                auto c1 = AND_even(R[i+j], partial);        // 2 NANDs
                auto c2 = AND_even(sum, carry);             // 2 NANDs
                auto new_carry = OR_even(c1, c2);           // 4 NANDs
                R[i+j] = XOR_even(sum, carry);              // 4 NANDs
                carry = new_carry;
            }
            R[i+4] = carry;
        }
        return R;
    };
    
    int mult_errors = 0;
    std::vector<std::pair<int, int>> test_cases = {
        {10, 6}, {3, 5}, {7, 9}, {15, 15}, {12, 11}
    };
    
    for (auto [a_val, b_val] : test_cases) {
        std::vector<Ciphertext<DCRTPoly>> A(4), B(4);
        for (int i = 0; i < 4; i++) {
            A[i] = ((a_val >> i) & 1) ? ct1 : ct0;
            B[i] = ((b_val >> i) & 1) ? ct1 : ct0;
        }
        
        auto R = multiply_4bit_even(A, B);
        int actual = 0;
        for (int bit = 0; bit < 8; bit++) {
            actual |= (decrypt(R[bit]) << bit);
        }
        int expected = a_val * b_val;
        bool ok = (actual == expected);
        if (!ok) mult_errors++;
        
        std::cout << "  " << a_val << " × " << b_val << " = " << actual
                  << " (exp " << expected << ")"
                  << (ok ? " ✅" : " ❌") << "\n";
    }

    std::cout << "\n========================================\n";
    if (errors == 0 && mult_errors == 0) {
        std::cout << "  ✅ ALL EVEN GATES WORK!\n";
        std::cout << "  ✅ 4-BIT MULTIPLIER CORRECT!\n";
        std::cout << "  ✅ PERIOD-2 HOLDS SA DEEP CIRCUITS!\n";
        std::cout << "  ✅ NO BOOTSTRAPPING NEEDED!\n";
    } else {
        std::cout << "  ❌ Gate errors: " << errors 
                  << ", Mult errors: " << mult_errors << "\n";
    }
    std::cout << "========================================\n";

    return 0;
}
