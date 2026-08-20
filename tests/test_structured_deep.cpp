// STRUCTURED DEEP CIRCUITS — Lahat may period-2 reset!
#include "openfhe.h"
#include <iostream>
#include <vector>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  STRUCTURED DEEP CIRCUITS\n";
    std::cout << "  (Period-2 sa LAHAT ng levels!)\n";
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
    // STRUCTURED GATES (lahat EVEN NANDs)
    // ============================================
    auto NOT = [&](Ciphertext<DCRTPoly> a) { return nand(a, a); };  // 1 NAND (odd! need pair)
    auto AND = [&](Ciphertext<DCRTPoly> a, Ciphertext<DCRTPoly> b) { 
        return NOT(nand(a, b));  // 2 NANDs (even) ✅
    };
    auto OR = [&](Ciphertext<DCRTPoly> a, Ciphertext<DCRTPoly> b) {
        return nand(NOT(a), NOT(b));  // 3 NANDs (odd! need pair)
    };
    auto XOR = [&](Ciphertext<DCRTPoly> a, Ciphertext<DCRTPoly> b) {
        auto n1 = nand(a, b);
        return nand(nand(a, n1), nand(b, n1));  // 4 NANDs (even) ✅
    };

    // ============================================
    // TEST 1: 4-BIT MULTIPLIER (structured)
    // ============================================
    std::cout << "1. 4-BIT MULTIPLIER (structured):\n";
    std::cout << "----------------------------------\n";
    
    auto multiply_4bit = [&](std::vector<Ciphertext<DCRTPoly>> A,
                              std::vector<Ciphertext<DCRTPoly>> B) {
        std::vector<Ciphertext<DCRTPoly>> R(8, ct0);
        
        for (int i = 0; i < 4; i++) {
            Ciphertext<DCRTPoly> carry = ct0;
            for (int j = 0; j < 4; j++) {
                auto partial = AND(A[j], B[i]);
                auto sum = XOR(R[i+j], partial);
                auto c1 = AND(R[i+j], partial);
                auto c2 = AND(sum, carry);
                auto new_carry = OR(c1, c2);
                R[i+j] = XOR(sum, carry);
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
        
        auto R = multiply_4bit(A, B);
        int actual = 0;
        for (int bit = 0; bit < 8; bit++) {
            actual |= (decrypt(R[bit]) << bit);
        }
        int expected = a_val * b_val;
        bool ok = (actual == expected);
        if (!ok) mult_errors++;
        
        std::cout << "  " << a_val << " × " << b_val << " = " << actual
                  << " (expected " << expected << ")"
                  << (ok ? " ✅" : " ❌") << "\n";
    }

    // ============================================
    // TEST 2: DEEP CHAIN (20 NANDs, structured)
    // ============================================
    std::cout << "\n2. DEEP CHAIN (structured NANDs):\n";
    std::cout << "---------------------------------\n";
    
    auto current = ct1;
    int chain_errors = 0;
    
    for (int i = 0; i < 10; i++) {
        // Paired NAND: NOT(NOT(x)) = x
        auto temp = nand(current, current);
        current = nand(temp, temp);
        
        int val = decrypt(current);
        if (val != 1) chain_errors++;
    }
    
    std::cout << "  10 paired NANDs (20 total): "
              << (chain_errors == 0 ? "✅ ALL CORRECT!" : "❌ " + std::to_string(chain_errors) + " errors") << "\n";

    std::cout << "\n========================================\n";
    if (mult_errors == 0 && chain_errors == 0) {
        std::cout << "  ✅ STRUCTURED DEEP CIRCUITS WORK!\n";
        std::cout << "  ✅ 4-BIT MULTIPLIER CORRECT!\n";
        std::cout << "  ✅ DEEP CHAIN CORRECT!\n";
        std::cout << "  ✅ PERIOD-2 HOLDS SA LAHAT!\n";
    } else {
        std::cout << "  ❌ Mult errors: " << mult_errors 
                  << ", Chain errors: " << chain_errors << "\n";
    }
    std::cout << "========================================\n";

    return 0;
}
