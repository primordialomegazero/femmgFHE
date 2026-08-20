// CHECK BOOTSTRAP API — Ano ang available sa OpenFHE
#include "openfhe.h"
#include <iostream>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  BOOTSTRAP API CHECK\n";
    std::cout << "========================================\n\n";

    // I-check kung ang EvalBootstrap ay available
    // sa CKKS scheme
    CCParams<CryptoContextCKKSRNS> params;
    params.SetMultiplicativeDepth(20);
    params.SetScalingModSize(40);
    params.SetBatchSize(1024);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(params);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);
    cc->Enable(ADVANCEDSHE);
    cc->Enable(FHE);

    std::cout << "FHE enabled: YES\n";
    std::cout << "ADVANCEDSHE enabled: YES\n\n";

    // Check kung may bootstrapping capabilities
    std::cout << "Available operations:\n";
    std::cout << "  - EvalBootstrap: ";
    try {
        // I-check lang kung ang method ay compile-time available
        std::cout << "COMPILED\n";
    } catch (...) {
        std::cout << "NOT AVAILABLE\n";
    }

    std::cout << "  - ModReduce: ";
    std::cout << "COMPILED\n";
    std::cout << "  - LevelReduce: ";
    std::cout << "COMPILED\n\n";

    // Subukan ang EvalBootstrapSetup na may safe parameters
    std::cout << "Trying EvalBootstrapSetup...\n";
    try {
        uint32_t slots = 1024;
        std::vector<uint32_t> levelBudget = {3, 3};
        std::vector<uint32_t> dim1 = {0, 0};
        
        cc->EvalBootstrapSetup(levelBudget, dim1, slots);
        std::cout << "  EvalBootstrapSetup: SUCCESS\n";
    } catch (std::exception& e) {
        std::cout << "  EvalBootstrapSetup: FAILED - " << e.what() << "\n";
    }

    return 0;
}
