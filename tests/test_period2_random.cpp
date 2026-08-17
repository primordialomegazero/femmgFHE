// PERIOD-2 RANDOM INPUTS — 100 random tests sa 20 depths
#include "openfhe.h"
#include <iostream>
#include <random>

using namespace lbcrypto;

int main() {
    std::cout << "PERIOD-2 RANDOM INPUTS (100 × 20 depths)\n";
    std::cout << "=======================================\n\n";
    
    CCParams<CryptoContextBFVRNS> parameters;
    parameters.SetPlaintextModulus(65537);
    parameters.SetMultiplicativeDepth(20);
    parameters.SetRingDim(32768);
    
    CryptoContext<DCRTPoly> cc = GenCryptoContext(parameters);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);
    
    std::cout << "Generating keys...\n";
    auto keys = cc->KeyGen();
    cc->EvalMultKeyGen(keys.secretKey);
    std::cout << "Keys ready!\n\n";
    
    auto ct0 = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext({0}));
    auto ct1 = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext({1}));
    
    auto nand = [&](Ciphertext<DCRTPoly> a, Ciphertext<DCRTPoly> b) {
        auto ab = cc->EvalMult(a, b);
        return cc->EvalSub(ct1, ab);
    };
    
    auto decrypt_val = [&](Ciphertext<DCRTPoly> ct) -> int64_t {
        Plaintext pt;
        cc->Decrypt(keys.secretKey, ct, &pt);
        return pt->GetPackedValue()[0];
    };
    
    std::mt19937_64 rng(42);
    
    int total_tests = 0;
    int failures = 0;
    
    std::cout << "Running 100 random tests sa 20 depths...\n\n";
    
    for (int trial = 0; trial < 100; trial++) {
        bool start = rng() % 2;  // Random start: 0 o 1
        auto current = start ? ct1 : ct0;
        
        for (int depth = 0; depth <= 20; depth++) {
            int64_t val = decrypt_val(current);
            int64_t expected = (depth % 2 == 0) ? (start ? 1 : 0) : (start ? 0 : 1);
            
            total_tests++;
            if (val != expected) {
                failures++;
                if (failures <= 5) {
                    std::cout << "FAIL: trial=" << trial << " depth=" << depth 
                              << " start=" << start << " got=" << val 
                              << " exp=" << expected << "\n";
                }
            }
            
            current = nand(current, current);
        }
    }
    
    std::cout << "\n========================\n";
    std::cout << "Total tests: " << total_tests << "\n";
    std::cout << "Passed: " << (total_tests - failures) << "\n";
    std::cout << "Failed: " << failures << "\n";
    std::cout << "Success rate: " << ((total_tests - failures) * 100.0 / total_tests) << "%\n";
    std::cout << (failures == 0 ? "✅ ALL 100 RANDOM TESTS PASSED!\n" : "❌ FAILURES FOUND\n");
    
    return 0;
}
