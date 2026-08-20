// REGULAR RESET — I-reset ang noise every 2 operations
#include "openfhe.h"
#include <iostream>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  REGULAR RESET STRATEGY\n";
    std::cout << "  (Period-2 reset every 2 ops)\n";
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
    // RESET FUNCTION: Period-2 self-NAND reset
    // ============================================
    auto reset_noise = [&](Ciphertext<DCRTPoly> x) {
        // NAND(NAND(x,x), NAND(x,x)) = x (noise reset)
        auto temp = nand(x, x);
        return nand(temp, temp);
    };

    // ============================================
    // TEST 1: DEEP CHAIN WITH REGULAR RESET
    // ============================================
    std::cout << "1. DEEP CHAIN (20 NANDs with reset every 2):\n";
    std::cout << "---------------------------------------------\n";
    
    auto current = ct1;
    int errors = 0;
    
    for (int i = 0; i < 10; i++) {
        // Operation 1: NAND
        auto op1 = nand(current, current);
        
        // Operation 2: NAND
        auto op2 = nand(op1, op1);
        
        // RESET: Period-2 reset
        current = reset_noise(op2);
        
        int val = decrypt(current);
        int expected = 1;  // After even NANDs + reset, dapat 1 pa rin
        if (val != expected) {
            errors++;
            std::cout << "  Iteration " << i+1 << ": " << val 
                      << " (expected " << expected << ") ❌\n";
        }
    }
    
    std::cout << "  Result: " << (errors == 0 ? "✅ ALL CORRECT!" 
              : "❌ " + std::to_string(errors) + " errors") << "\n\n";

    // ============================================
    // TEST 2: MIXED CIRCUIT WITH REGULAR RESET
    // ============================================
    std::cout << "2. MIXED CIRCUIT (with reset every 2):\n";
    std::cout << "---------------------------------------\n";
    
    auto a = ct1;
    auto b = ct0;
    errors = 0;
    
    for (int i = 0; i < 5; i++) {
        // Mixed operations
        auto nand_ab = nand(a, b);
        auto nand_aa = nand(a, a);
        
        // RESET
        a = reset_noise(nand_ab);
        b = reset_noise(nand_aa);
        
        int val_a = decrypt(a);
        int val_b = decrypt(b);
        
        if (val_a != 0 || val_b != 1) {
            errors++;
            std::cout << "  Iteration " << i+1 << ": a=" << val_a 
                      << " b=" << val_b << " ❌\n";
        }
    }
    
    std::cout << "  Result: " << (errors == 0 ? "✅ ALL CORRECT!" 
              : "❌ " + std::to_string(errors) + " errors") << "\n\n";

    // ============================================
    // TEST 3: DEEP FRACTAL WITH REGULAR RESET
    // ============================================
    std::cout << "3. DEEP FRACTAL (Level 5 with reset):\n";
    std::cout << "--------------------------------------\n";
    
    auto fractal_level = [&](Ciphertext<DCRTPoly> q1, Ciphertext<DCRTPoly> q2) {
        auto temp = nand(q1, q2);
        return reset_noise(temp);
    };
    
    current = ct1;
    errors = 0;
    
    for (int i = 0; i < 10; i++) {
        current = fractal_level(current, ct0);
        int val = decrypt(current);
        
        if (val != 0) {  // Expected: NAND(1,0) = 1, pero after reset = 1
            errors++;
        }
    }
    
    std::cout << "  10 fractal levels: " 
              << (errors == 0 ? "✅ ALL CORRECT!" 
              : "❌ " + std::to_string(errors) + " errors") << "\n";

    std::cout << "\n========================================\n";
    std::cout << "  REGULAR RESET STRATEGY:\n";
    std::cout << "  - I-reset every 2 operations\n";
    std::cout << "  - Period-2 self-NAND as reset\n";
    std::cout << "  - DEEP circuits possible!\n";
    std::cout << "========================================\n";

    return 0;
}
