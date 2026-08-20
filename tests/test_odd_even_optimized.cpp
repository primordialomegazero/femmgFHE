// ODD+EVEN OPTIMIZED — Reset every 2 operations (hindi every op!)
#include "openfhe.h"
#include <iostream>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  ODD+EVEN OPTIMIZED\n";
    std::cout << "  (Reset every 2 operations)\n";
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
    // OPTIMIZED: 2 operations + 1 reset (3 NANDs total)
    // ============================================
    auto process_pair = [&](Ciphertext<DCRTPoly> x) {
        // Operation 1: NOT (1 NAND)
        auto op1 = nand(x, x);
        // Operation 2: NOT (1 NAND) — net effect = identity
        auto op2 = nand(op1, op1);
        // Reset (1 NAND para sa period-2)
        auto reset = nand(op2, op2);
        return reset;
    };

    // ============================================
    // TEST: DEEP CHAIN with OPTIMIZED strategy
    // ============================================
    std::cout << "DEEP CHAIN (optimized reset):\n";
    std::cout << "-------------------------------\n";
    
    auto current = ct1;
    int errors = 0;
    
    for (int i = 0; i < 10; i++) {
        current = process_pair(current);
        int val = decrypt(current);
        
        // process_pair: NOT(NOT(x)) = x, tapos reset = x
        int expected = 1;
        
        std::cout << "  Iteration " << i+1 << ": " << val 
                  << " (exp " << expected << ")"
                  << (val == expected ? " ✅" : " ❌") << "\n";
        
        if (val != expected) errors++;
    }

    std::cout << "\n========================================\n";
    std::cout << "  Result: " << (errors == 0 ? "✅ ALL CORRECT!" 
              : "❌ " + std::to_string(errors) + " errors") << "\n";
    std::cout << "========================================\n";

    return 0;
}
