// CKKS DEPTH PROBE PARA SA 1000 GATES
// I-check kung kaya ng PC ang depth 1000
// Lightweight — walang actual computation

#include "openfhe.h"
#include <iostream>
#include <chrono>

using namespace lbcrypto;
using namespace std::chrono;

int main() {
    std::cout << "========================================\n";
    std::cout << "  DEPTH PROBE PARA SA 1000 GATES\n";
    std::cout << "========================================\n\n";

    // I-test kung anong depths ang gumagana
    for (int depth : {200, 500, 1000}) {
        std::cout << "Depth " << depth << "... ";
        std::cout.flush();

        try {
            CCParams<CryptoContextCKKSRNS> params;
            params.SetMultiplicativeDepth(depth);
            params.SetScalingModSize(40);
            params.SetBatchSize(512);

            auto t1 = high_resolution_clock::now();
            CryptoContext<DCRTPoly> cc = GenCryptoContext(params);
            cc->Enable(PKE);
            cc->Enable(KEYSWITCH);
            cc->Enable(LEVELEDSHE);
            auto t2 = high_resolution_clock::now();

            auto ring = cc->GetRingDimension();
            auto ms = duration_cast<milliseconds>(t2-t1).count();
            std::cout << "OK (ring=" << ring << ", context_gen=" << ms << "ms)\n";
        } catch (std::exception& e) {
            std::cout << "FAIL: " << e.what() << "\n";
        }
    }

    return 0;
}
