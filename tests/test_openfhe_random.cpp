// TEST 1: RANDOM INPUTS — 1000 random NAND tests sa OpenFHE
#include "openfhe.h"
#include <iostream>
#include <random>

using namespace lbcrypto;

int main() {
    std::cout << "RANDOM INPUTS TEST (OpenFHE BFV)\n";
    std::cout << "================================\n\n";
    
    CCParams<CryptoContextBFVRNS> parameters;
    parameters.SetPlaintextModulus(65537);
    parameters.SetMultiplicativeDepth(5);  // Maliit para mabilis
    parameters.SetRingDim(16384);  // Minimum required by OpenFHE
    
    CryptoContext<DCRTPoly> cc = GenCryptoContext(parameters);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);
    
    auto keys = cc->KeyGen();
    cc->EvalMultKeyGen(keys.secretKey);
    
    std::mt19937_64 rng(42);
    
    int failures = 0;
    int total = 1000;
    
    std::cout << "Testing " << total << " random inputs...\n\n";
    
    for (int trial = 0; trial < total; trial++) {
        bool a = rng() % 2;
        bool b = rng() % 2;
        
        auto ct_a = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext({(int64_t)a}));
        auto ct_b = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext({(int64_t)b}));
        
        // NAND(a,b) = 1 - a·b
        auto ab = cc->EvalMult(ct_a, ct_b);
        auto ct_one = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext({1}));
        auto nand_result = cc->EvalSub(ct_one, ab);
        
        Plaintext pt;
        cc->Decrypt(keys.secretKey, nand_result, &pt);
        int result = pt->GetPackedValue()[0];
        int expected = !(a && b) ? 1 : 0;
        
        if (result != expected) {
            failures++;
            std::cout << "FAIL: NAND(" << a << "," << b << ") = " << result 
                      << " (exp " << expected << ")\n";
        }
    }
    
    std::cout << "\nResult: " << (total - failures) << "/" << total << " PASS\n";
    std::cout << "Failures: " << failures << "\n";
    std::cout << (failures == 0 ? "ALL RANDOM INPUTS PASS! ✓\n" : "FAILURES FOUND! ✗\n");
    
    return 0;
}
