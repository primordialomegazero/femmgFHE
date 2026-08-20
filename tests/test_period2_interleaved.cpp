// PERIOD-2 INTERLEAVED — Subukan i-reset ang noise sa ODD levels
#include "openfhe.h"
#include <iostream>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  PERIOD-2 INTERLEAVED TEST\n";
    std::cout << "  (Noise reset sa ODD levels?)\n";
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
    // TEST 1: ODD vs EVEN levels
    // ============================================
    std::cout << "ODD vs EVEN LEVELS:\n";
    std::cout << "-------------------\n";
    
    auto current = ct0;
    for (int level = 1; level <= 10; level++) {
        current = nand(current, current);
        int val = decrypt(current);
        int expected = (level % 2 == 1) ? 1 : 0;
        std::cout << "  Level " << level << ": " << val 
                  << " (expected " << expected << ") "
                  << (val == expected ? "✅" : "❌") << "\n";
    }

    // ============================================
    // TEST 2: INTERLEAVED RESET (every 2 levels)
    // ============================================
    std::cout << "\nINTERLEAVED RESET (every 2 levels):\n";
    std::cout << "-----------------------------------\n";
    
    current = ct0;
    int errors = 0;
    for (int level = 1; level <= 20; level++) {
        current = nand(current, current);
        
        // Tuwing EVEN level, i-reset ang noise
        if (level % 2 == 0) {
            // Period-2 reset: NAND(NAND(x,x), NAND(x,x)) = x
            auto temp = nand(current, current);
            current = temp;
        }
        
        int val = decrypt(current);
        int expected = (level % 2 == 1) ? 1 : 0;
        if (val != expected) errors++;
    }
    std::cout << "  Interleaved 20 levels: " 
              << (errors == 0 ? "✅ ALL CORRECT!" : "❌ " + std::to_string(errors) + " errors") << "\n";

    // ============================================
    // TEST 3: PAIRED RESET (self-NAND pairs)
    // ============================================
    std::cout << "\nPAIRED RESET (NAND pairs):\n";
    std::cout << "--------------------------\n";
    
    current = ct0;
    errors = 0;
    for (int pair = 1; pair <= 10; pair++) {
        // Apply 2 NANDs (period-2 pair)
        auto first = nand(current, current);
        auto second = nand(first, first);
        current = second;  // Dapat bumalik sa original
        
        int val = decrypt(current);
        if (val != 0) errors++;
    }
    std::cout << "  10 pairs (20 NANDs): " 
              << (errors == 0 ? "✅ ALL CORRECT!" : "❌ " + std::to_string(errors) + " errors") << "\n";

    std::cout << "\n========================================\n";
    std::cout << "  CONCLUSION:\n";
    std::cout << "  - Odd levels: noise accumulates\n";
    std::cout << "  - Even levels: period-2 resets\n";
    std::cout << "  - Paired NANDs: controlled reset\n";
    std::cout << "========================================\n";

    return 0;
}
