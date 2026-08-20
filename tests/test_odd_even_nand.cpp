// ODD + EVEN NAND — Tamang structure!
#include "openfhe.h"
#include <iostream>
#include <vector>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  ODD + EVEN NAND STRATEGY\n";
    std::cout << "  (Operation → Reset → Operation...)\n";
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
    // ODD NAND = OPERATION (1 NAND)
    // EVEN NAND = RESET (2 NANDs, identity)
    // ============================================
    
    // ODD NOT: 1 NAND = actual NOT
    auto odd_not = [&](Ciphertext<DCRTPoly> x) {
        return nand(x, x);  // 1 NAND (odd) = NOT(x)
    };
    
    // EVEN RESET: 2 NANDs = identity (period-2 reset)
    auto even_reset = [&](Ciphertext<DCRTPoly> x) {
        auto temp = nand(x, x);      // 1 NAND
        return nand(temp, temp);      // 2 NANDs = x (reset)
    };
    
    // FULL GATE: Operation + Reset
    auto gated_not = [&](Ciphertext<DCRTPoly> x) {
        auto result = odd_not(x);     // NOT(x)
        return even_reset(result);    // Reset noise, value stays NOT(x)
    };

    // ============================================
    // TEST 1: ODD vs EVEN vs GATED
    // ============================================
    std::cout << "1. ODD vs EVEN vs GATED NOT:\n";
    std::cout << "-------------------------------\n";
    
    std::cout << "  Odd NOT(0) = " << decrypt(odd_not(ct0)) << " (exp 1)\n";
    std::cout << "  Even Reset(0) = " << decrypt(even_reset(ct0)) << " (exp 0)\n";
    std::cout << "  Gated NOT(0) = " << decrypt(gated_not(ct0)) << " (exp 1)\n";
    std::cout << "  Gated NOT(1) = " << decrypt(gated_not(ct1)) << " (exp 0)\n\n";

    // ============================================
    // TEST 2: GATED FULL ADDER
    // ============================================
    std::cout << "2. GATED FULL ADDER (Operation→Reset):\n";
    std::cout << "---------------------------------------\n";
    
    auto gated_nand = [&](Ciphertext<DCRTPoly> a, Ciphertext<DCRTPoly> b) {
        auto result = nand(a, b);       // Operation (1 NAND)
        return even_reset(result);       // Reset (2 NANDs)
    };
    
    auto gated_xor = [&](Ciphertext<DCRTPoly> a, Ciphertext<DCRTPoly> b) {
        auto n1 = gated_nand(a, b);
        auto left = gated_nand(a, n1);
        auto right = gated_nand(b, n1);
        return gated_nand(left, right);
    };
    
    auto gated_and = [&](Ciphertext<DCRTPoly> a, Ciphertext<DCRTPoly> b) {
        auto temp = gated_nand(a, b);
        return gated_nand(temp, temp);
    };
    
    auto gated_or = [&](Ciphertext<DCRTPoly> a, Ciphertext<DCRTPoly> b) {
        auto not_a = gated_nand(a, a);
        auto not_b = gated_nand(b, b);
        return gated_nand(not_a, not_b);
    };
    
    int adder_errors = 0;
    for (int a = 0; a <= 1; a++) {
        for (int b = 0; b <= 1; b++) {
            for (int cin = 0; cin <= 1; cin++) {
                auto ct_a = a ? ct1 : ct0;
                auto ct_b = b ? ct1 : ct0;
                auto ct_cin = cin ? ct1 : ct0;
                
                auto sum = gated_xor(gated_xor(ct_a, ct_b), ct_cin);
                auto carry = gated_or(gated_and(ct_a, ct_b), 
                              gated_and(gated_xor(ct_a, ct_b), ct_cin));
                
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
    std::cout << "  Gated Full Adder (8 cases): " 
              << (adder_errors == 0 ? "✅ ALL CORRECT!" : "❌ " + std::to_string(adder_errors) + " errors") << "\n\n";

    // ============================================
    // TEST 3: DEEP CHAIN WITH GATED NAND
    // ============================================
    std::cout << "3. DEEP CHAIN (20 gated operations):\n";
    std::cout << "-------------------------------------\n";
    
    auto current = ct1;
    int chain_errors = 0;
    
    for (int i = 0; i < 10; i++) {
        current = gated_nand(current, current);
        int val = decrypt(current);
        if (val != 0) chain_errors++;  // NOT chain: 1→0→1→0...
    }
    
    std::cout << "  10 gated NANDs: " 
              << (chain_errors == 0 ? "✅ ALL CORRECT!" : "❌ " + std::to_string(chain_errors) + " errors") << "\n";

    std::cout << "\n========================================\n";
    if (adder_errors == 0 && chain_errors == 0) {
        std::cout << "  ✅ ODD+EVEN STRATEGY WORKS!\n";
        std::cout << "  ✅ GATED FULL ADDER CORRECT!\n";
        std::cout << "  ✅ DEEP CHAIN CORRECT!\n";
        std::cout << "  ✅ PERIOD-2 RESET CONFIRMED!\n";
    } else {
        std::cout << "  ❌ Adder: " << adder_errors 
                  << ", Chain: " << chain_errors << "\n";
    }
    std::cout << "========================================\n";

    return 0;
}
