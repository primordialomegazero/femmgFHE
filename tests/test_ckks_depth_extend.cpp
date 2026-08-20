// CKKS DEPTH EXTEND — Mas malalim na walang bootstrapping
// Hanggang saan kaya ng CKKS na may binary NAND?

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  CKKS DEPTH EXTEND TEST\n";
    std::cout << "  Hanggang saan kaya ng CKKS?\n";
    std::cout << "========================================\n\n";

    // Test different depths
    for (int depth : {30, 50, 80, 100}) {
        std::cout << "Testing depth=" << depth << "... ";
        std::cout.flush();

        try {
            CCParams<CryptoContextCKKSRNS> parameters;
            parameters.SetMultiplicativeDepth(depth);
            parameters.SetScalingModSize(50);
            parameters.SetBatchSize(8192);

            CryptoContext<DCRTPoly> cc = GenCryptoContext(parameters);
            cc->Enable(PKE);
            cc->Enable(KEYSWITCH);
            cc->Enable(LEVELEDSHE);

            auto keys = cc->KeyGen();
            cc->EvalMultKeyGen(keys.secretKey);

            auto ring_dim = cc->GetRingDimension();
            std::cout << "OK (ring=" << ring_dim << ")\n";
        } catch (std::exception& e) {
            std::cout << "FAIL: " << e.what() << "\n";
        }
    }

    return 0;
}
