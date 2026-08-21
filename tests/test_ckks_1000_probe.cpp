// CKKS 1000 GATES PROBE — Parameter Search
// Maghanap ng parameter set na kakasya sa depth 1000

#include "openfhe.h"
#include <iostream>
#include <chrono>

using namespace lbcrypto;
using namespace std::chrono;

int main() {
    std::cout << "========================================\n";
    std::cout << "  CKKS 1000 GATES — PARAMETER SEARCH\n";
    std::cout << "========================================\n\n";

    // Subukan iba't ibang parameter sets
    struct ParamSet {
        int depth;
        int scaling_mod_size;
        int batch_size;
    };

    std::vector<ParamSet> candidates = {
        {1000, 30, 256},
        {1000, 35, 256},
        {1000, 40, 256},
        {1000, 45, 256},
        {1000, 50, 256},
        {1000, 40, 128},
        {1000, 50, 128},
        {1000, 40, 64},
        {1000, 50, 64}
    };

    for (auto& p : candidates) {
        std::cout << "Depth=" << p.depth
                  << " Scaling=" << p.scaling_mod_size
                  << " Batch=" << p.batch_size << "... ";
        std::cout.flush();

        try {
            CCParams<CryptoContextCKKSRNS> params;
            params.SetMultiplicativeDepth(p.depth);
            params.SetScalingModSize(p.scaling_mod_size);
            params.SetBatchSize(p.batch_size);

            auto t1 = high_resolution_clock::now();
            CryptoContext<DCRTPoly> cc = GenCryptoContext(params);
            cc->Enable(PKE);
            cc->Enable(KEYSWITCH);
            cc->Enable(LEVELEDSHE);
            auto t2 = high_resolution_clock::now();

            auto ring = cc->GetRingDimension();
            auto ms = duration_cast<milliseconds>(t2 - t1).count();
            std::cout << "OK (ring=" << ring << ", gen=" << ms/1000.0 << "s)\n";
        } catch (std::exception& e) {
            std::cout << "FAIL: " << e.what() << "\n";
        }
    }

    return 0;
}
