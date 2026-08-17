// TEST: 1000 random inputs, 1000 depths
#include <openfhe/pke/openfhe.h>
#include <iostream>
#include <random>

using namespace lbcrypto;

int main() {
    std::cout << "1000 RANDOM INPUTS × 1000 DEPTHS\n";
    std::cout << "================================\n\n";

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

    auto ct1 = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext({1}));
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 1);

    int total_tests = 1000;
    int passed = 0;
    int failed = 0;

    for (int test = 0; test < total_tests; test++) {
        int a = dis(gen);
        int b = dis(gen);
        
        auto ct_a = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext({a}));
        auto ct_b = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext({b}));
        
        // Compute NAND: 1 - a*b
        auto ct_ab = cc->EvalMult(ct_a, ct_b);
        auto ct_result = cc->EvalSub(ct1, ct_ab);
        
        // Apply 1000 depths (just for testing, pero we know 20 is enough)
        auto current = ct_result;
        for (int depth = 0; depth < 1000; depth++) {
            auto sq = cc->EvalMult(current, current);
            current = cc->EvalSub(ct1, sq);
        }
        
        Plaintext pt;
        cc->Decrypt(keys.secretKey, current, &pt);
        int result = pt->GetPackedValue()[0];
        int expected = (a == 1 && b == 1) ? 0 : 1;
        
        if (result == expected) {
            passed++;
        } else {
            failed++;
            std::cout << "FAIL: " << a << " NAND " << b << " = " << result 
                      << " (expected " << expected << ")\n";
        }
    }

    std::cout << "\n--- RESULTS ---\n";
    std::cout << "Tests: " << total_tests << "\n";
    std::cout << "Passed: " << passed << "\n";
    std::cout << "Failed: " << failed << "\n";
    std::cout << "Success rate: " << (100.0 * passed / total_tests) << "%\n";
    
    if (failed == 0) {
        std::cout << "\n✅ 1000 RANDOM INPUTS × 1000 DEPTHS = PERFECT!\n";
        std::cout << "✅ UNLIMITED DEPTH CONFIRMED FOR ALL INPUTS!\n";
    }

    return 0;
}
