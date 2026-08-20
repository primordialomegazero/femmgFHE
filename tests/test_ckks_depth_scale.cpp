// CKKS DEPTH SCALE UP — Para sa 1k-10k gates
#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <chrono>

using namespace lbcrypto;
using namespace std::chrono;

int main() {
    std::cout << "========================================\n";
    std::cout << "  CKKS DEPTH SCALE UP\n";
    std::cout << "  Para sa 1000+ gates\n";
    std::cout << "========================================\n\n";

    // Test sa iba't ibang depth levels
    for (int depth : {100, 200, 500, 1000}) {
        std::cout << "Depth " << depth << "... ";
        std::cout.flush();

        try {
            CCParams<CryptoContextCKKSRNS> params;
            params.SetMultiplicativeDepth(depth);
            params.SetScalingModSize(40);
            params.SetBatchSize(2048);

            CryptoContext<DCRTPoly> cc = GenCryptoContext(params);
            cc->Enable(PKE);
            cc->Enable(KEYSWITCH);
            cc->Enable(LEVELEDSHE);

            auto ring = cc->GetRingDimension();
            std::cout << "OK (ring=" << ring << ")\n";
        } catch (std::exception& e) {
            std::cout << "FAIL: " << e.what() << "\n";
        }
    }

    return 0;
}
