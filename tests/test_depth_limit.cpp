// DEPTH LIMIT RESEARCH — Hanapin ang EXACT kaya ng period-2
#include "openfhe.h"
#include <iostream>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  DEPTH LIMIT RESEARCH\n";
    std::cout << "  (Hanapin ang exact boundary)\n";
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
    // TEST: SELF-NAND CHAIN — HANAPIN ANG LIMIT
    // ============================================
    std::cout << "SELF-NAND CHAIN (NAND(x,x) repeatedly):\n";
    std::cout << "-----------------------------------------\n";
    
    auto current = ct1;
    int max_depth = 0;
    
    for (int i = 1; i <= 50; i++) {
        current = nand(current, current);
        int val = decrypt(current);
        int expected = (i % 2 == 1) ? 0 : 1;  // 1→0→1→0...
        
        if (val != expected) {
            std::cout << "  ❌ CORRUPTION at NAND #" << i 
                      << ": got " << val << ", expected " << expected << "\n";
            break;
        }
        max_depth = i;
    }
    
    std::cout << "  ✅ Max self-NAND depth: " << max_depth << " NANDs\n\n";

    // ============================================
    // TEST: PAIRED NAND CHAIN — HANAPIN ANG LIMIT
    // ============================================
    std::cout << "PAIRED NAND CHAIN (NAND(NAND(x,x), NAND(x,x))):\n";
    std::cout << "------------------------------------------------\n";
    
    current = ct1;
    int max_pairs = 0;
    
    for (int i = 1; i <= 25; i++) {
        auto temp = nand(current, current);
        current = nand(temp, temp);
        int val = decrypt(current);
        
        if (val != 1) {  // After even pairs, dapat 1 pa rin
            std::cout << "  ❌ CORRUPTION at pair #" << i 
                      << ": got " << val << ", expected 1\n";
            break;
        }
        max_pairs = i;
    }
    
    std::cout << "  ✅ Max paired depth: " << (max_pairs * 2) << " NANDs\n\n";

    // ============================================
    // TEST: MIXED NAND CHAIN — HANAPIN ANG LIMIT
    // ============================================
    std::cout << "MIXED NAND CHAIN (NAND(a,b) alternating):\n";
    std::cout << "--------------------------------------------\n";
    
    auto a = ct1;
    auto b = ct0;
    int max_mixed = 0;
    
    for (int i = 1; i <= 20; i++) {
        auto result = nand(a, b);
        b = a;
        a = result;
        int val = decrypt(a);
        
        if (val < 0 || val > 1) {  // Corrupted kung hindi 0/1
            std::cout << "  ❌ CORRUPTION at mixed #" << i 
                      << ": got " << val << "\n";
            break;
        }
        max_mixed = i;
    }
    
    std::cout << "  ✅ Max mixed depth: " << max_mixed << " NANDs\n\n";

    // ============================================
    // SUMMARY
    // ============================================
    std::cout << "========================================\n";
    std::cout << "  DEPTH LIMIT SUMMARY:\n";
    std::cout << "  - Self-NAND: " << max_depth << " NANDs\n";
    std::cout << "  - Paired NAND: " << (max_pairs * 2) << " NANDs\n";
    std::cout << "  - Mixed NAND: " << max_mixed << " NANDs\n";
    std::cout << "  - Hardware: 16GB RAM, depth 20\n";
    std::cout << "========================================\n";

    return 0;
}
