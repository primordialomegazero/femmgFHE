#include "openfhe.h"
#include <iostream>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  BOOTSTRAP TEST - UNLIMITED DEPTH\n";
    std::cout << "========================================\n\n";

    CCParams<CryptoContextCKKSRNS> parameters;
    parameters.SetMultiplicativeDepth(30);
    parameters.SetRingDim(65536);
    parameters.SetScalingModSize(50);
    parameters.SetBatchSize(32768);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(parameters);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);
    cc->Enable(ADVANCEDSHE);
    cc->Enable(FHE);

    auto keys = cc->KeyGen();
    cc->EvalMultKeyGen(keys.secretKey);
    cc->EvalBootstrapSetup();

    auto ct0 = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext({0}));
    auto ct1 = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext({1}));

    auto decrypt = [&](Ciphertext<DCRTPoly> ct) -> double {
        Plaintext pt;
        cc->Decrypt(keys.secretKey, ct, &pt);
        return pt->GetPackedValue()[0];
    };

    auto nand = [&](Ciphertext<DCRTPoly> a, Ciphertext<DCRTPoly> b) {
        auto ab = cc->EvalMult(a, b);
        return cc->EvalSub(ct1, ab);
    };

    std::cout << "1. NAND CHAIN WITH BOOTSTRAP (bootstrap every 10):\n";
    std::cout << "----------------------------------------------\n";

    auto current = ct0;
    int errors = 0;

    for (int i = 1; i <= 100; i++) {
        current = nand(current, current);

        if (i % 10 == 0) {
            current = cc->EvalBootstrap(current);
        }

        int expected = (i % 2 == 0) ? 0 : 1;
        double result = decrypt(current);
        if (std::abs(result - expected) > 0.1) {
            errors++;
            if (errors <= 5) {
                std::cout << "  Iteration " << i << ": " << result
                          << " (expected " << expected << ") FAIL\n";
            }
        }
    }

    if (errors == 0) {
        std::cout << "  100 NANDs with bootstrap: ALL CORRECT\n";
    } else {
        std::cout << "  Errors: " << errors << "\n";
    }

    std::cout << "\n========================================\n";
    if (errors == 0) {
        std::cout << "  RESULT: ARBITRARY DEPTH ACHIEVED\n";
    } else {
        std::cout << "  RESULT: Requires parameter tuning\n";
    }
    std::cout << "========================================\n";

    return 0;
}
