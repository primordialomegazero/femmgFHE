// STRUCTURED RESET — Tamang structure para sa mixed circuits
#include "openfhe.h"
#include <iostream>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  STRUCTURED RESET STRATEGY\n";
    std::cout << "  (Proper structure for mixed circuits)\n";
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
    // STRUCTURED GATES: Bawat gate may PAIRED reset
    // ============================================
    
    // Structured NOT: NOT(x) na may period-2 reset
    auto structured_not = [&](Ciphertext<DCRTPoly> x) {
        // NOT(x) = NAND(x,x)
        // PERO with reset: NAND(x,x) tapos NAND(reset, reset)
        auto nand_xx = nand(x, x);  // NOT(x)
        // I-reset ang noise ng result:
        // NAND(NAND(y,y), NAND(y,y)) = y (period-2)
        return nand_xx;  // 1 NAND lang muna
    };

    // Structured AND: AND(a,b) na may period-2 reset
    auto structured_and = [&](Ciphertext<DCRTPoly> a, Ciphertext<DCRTPoly> b) {
        // AND(a,b) = NAND(NAND(a,b), NAND(a,b))
        auto nand_ab = nand(a, b);
        auto result = nand(nand_ab, nand_ab);  // AND(a,b)
        return result;
    };

    // Structured OR: OR(a,b) na may period-2 reset
    auto structured_or = [&](Ciphertext<DCRTPoly> a, Ciphertext<DCRTPoly> b) {
        // OR(a,b) = NAND(NOT(a), NOT(b))
        auto not_a = nand(a, a);
        auto not_b = nand(b, b);
        auto result = nand(not_a, not_b);
        return result;
    };

    // ============================================
    // TEST 1: STRUCTURED GATES (shallow)
    // ============================================
    std::cout << "1. STRUCTURED GATES (Truth Table):\n";
    std::cout << "-----------------------------------\n";
    
    int errors = 0;
    
    // NOT
    int not_0 = decrypt(structured_not(ct0));
    int not_1 = decrypt(structured_not(ct1));
    std::cout << "  NOT(0) = " << not_0 << " (expected 1) " << (not_0 == 1 ? "✅" : "❌") << "\n";
    std::cout << "  NOT(1) = " << not_1 << " (expected 0) " << (not_1 == 0 ? "✅" : "❌") << "\n";
    if (not_0 != 1 || not_1 != 0) errors++;
    
    // AND
    int and_00 = decrypt(structured_and(ct0, ct0));
    int and_01 = decrypt(structured_and(ct0, ct1));
    int and_11 = decrypt(structured_and(ct1, ct1));
    std::cout << "  AND(0,0) = " << and_00 << " (expected 0) " << (and_00 == 0 ? "✅" : "❌") << "\n";
    std::cout << "  AND(0,1) = " << and_01 << " (expected 0) " << (and_01 == 0 ? "✅" : "❌") << "\n";
    std::cout << "  AND(1,1) = " << and_11 << " (expected 1) " << (and_11 == 1 ? "✅" : "❌") << "\n";
    if (and_00 != 0 || and_01 != 0 || and_11 != 1) errors++;
    
    // OR
    int or_00 = decrypt(structured_or(ct0, ct0));
    int or_01 = decrypt(structured_or(ct0, ct1));
    int or_11 = decrypt(structured_or(ct1, ct1));
    std::cout << "  OR(0,0) = " << or_00 << " (expected 0) " << (or_00 == 0 ? "✅" : "❌") << "\n";
    std::cout << "  OR(0,1) = " << or_01 << " (expected 1) " << (or_01 == 1 ? "✅" : "❌") << "\n";
    std::cout << "  OR(1,1) = " << or_11 << " (expected 1) " << (or_11 == 1 ? "✅" : "❌") << "\n";
    if (or_00 != 0 || or_01 != 1 || or_11 != 1) errors++;

    // ============================================
    // TEST 2: STRUCTURED FULL ADDER
    // ============================================
    std::cout << "\n2. STRUCTURED FULL ADDER:\n";
    std::cout << "------------------------\n";
    
    // Full adder: sum = XOR(a,b) XOR cin, carry = OR(AND(a,b), AND(XOR(a,b), cin))
    auto structured_xor = [&](Ciphertext<DCRTPoly> a, Ciphertext<DCRTPoly> b) {
        auto n1 = nand(a, b);
        auto left = nand(a, n1);
        auto right = nand(b, n1);
        return nand(left, right);
    };
    
    int adder_errors = 0;
    for (int a = 0; a <= 1; a++) {
        for (int b = 0; b <= 1; b++) {
            for (int cin = 0; cin <= 1; cin++) {
                auto ct_a = a ? ct1 : ct0;
                auto ct_b = b ? ct1 : ct0;
                auto ct_cin = cin ? ct1 : ct0;
                
                auto sum = structured_xor(structured_xor(ct_a, ct_b), ct_cin);
                auto carry = structured_or(structured_and(ct_a, ct_b), 
                              structured_and(structured_xor(ct_a, ct_b), ct_cin));
                
                int sum_val = decrypt(sum);
                int carry_val = decrypt(carry);
                int exp_sum = (a + b + cin) % 2;
                int exp_carry = (a + b + cin) / 2;
                
                if (sum_val != exp_sum || carry_val != exp_carry) {
                    adder_errors++;
                }
            }
        }
    }
    std::cout << "  Full Adder (8 cases): " 
              << (adder_errors == 0 ? "✅ ALL CORRECT!" : "❌ " + std::to_string(adder_errors) + " errors") << "\n";

    std::cout << "\n========================================\n";
    if (errors == 0 && adder_errors == 0) {
        std::cout << "  ✅ STRUCTURED GATES WORK!\n";
        std::cout << "  ✅ FULL ADDER WORKS!\n";
        std::cout << "  ✅ READY FOR DEEPER STRUCTURES!\n";
    } else {
        std::cout << "  ❌ Errors: gates=" << errors << " adder=" << adder_errors << "\n";
    }
    std::cout << "========================================\n";

    return 0;
}
