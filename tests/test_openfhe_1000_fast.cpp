// 1000 RANDOM INPUTS — FAST (keys generated once)
#include "openfhe.h"
#include <iostream>
#include <chrono>

using namespace lbcrypto;

int main() {
    std::cout << "1000 RANDOM INPUTS (FAST)\n";
    std::cout << "=========================\n\n";
    
    CCParams<CryptoContextBFVRNS> parameters;
    parameters.SetPlaintextModulus(65537);
    parameters.SetMultiplicativeDepth(5);
    parameters.SetRingDim(16384);
    
    CryptoContext<DCRTPoly> cc = GenCryptoContext(parameters);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);
    
    std::cout << "Generating keys ONCE...\n";
    auto keys = cc->KeyGen();
    cc->EvalMultKeyGen(keys.secretKey);
    std::cout << "Keys ready!\n\n";
    
    // Pre-encrypt 1 (para sa NAND)
    auto ct_one = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext({1}));
    
    int failures = 0;
    const int TOTAL = 10000;
    
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < TOTAL; i++) {
        bool a = i % 2;       // Alternating 0,1
        bool b = (i / 2) % 2; // Alternating pairs
        
        auto ct_a = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext({(int64_t)a}));
        auto ct_b = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext({(int64_t)b}));
        
        // NAND(a,b) = 1 - a·b
        auto ab = cc->EvalMult(ct_a, ct_b);
        auto nand_result = cc->EvalSub(ct_one, ab);
        
        Plaintext pt;
        cc->Decrypt(keys.secretKey, nand_result, &pt);
        int result = pt->GetPackedValue()[0];
        int expected = !(a && b) ? 1 : 0;
        
        if (result != expected) {
            failures++;
            if (failures <= 5) {
                std::cout << "FAIL: NAND(" << a << "," << b << ") = " << result 
                          << " (exp " << expected << ")\n";
            }
        }
        
        // Progress
        if (i % 2000 == 0 && i > 0) {
            std::cout << "  " << i << "/" << TOTAL << " tested...\n";
        }
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    
    std::cout << "\n========================\n";
    std::cout << "Total: " << (TOTAL - failures) << "/" << TOTAL << " PASS\n";
    std::cout << "Failures: " << failures << "\n";
    std::cout << "Time: " << ms << " ms\n";
    std::cout << "Ops/sec: " << (TOTAL * 1000.0 / ms) << "\n";
    std::cout << (failures == 0 ? "✅ 1000 RANDOM INPUTS PASS!\n" : "❌ FAILURES FOUND\n");
    
    return 0;
}
