// DEPTH 200 PARAMETER SEARCH
// Maghanap ng parameters na kasya sa 16GB RAM

#include "openfhe.h"
#include <iostream>
#include <chrono>

using namespace lbcrypto;
using namespace std::chrono;

int main() {
    std::cout << "========================================\n";
    std::cout << "  DEPTH 200 PARAMETER SEARCH\n";
    std::cout << "========================================\n\n";

    struct ParamSet {
        int depth;
        int scaling_mod_size;
        int batch_size;
    };

    std::vector<ParamSet> candidates = {
        {200, 25, 32},
        {200, 25, 64},
        {200, 25, 128},
        {200, 30, 32},
        {200, 30, 64},
        {200, 30, 128},
        {200, 35, 32},
        {200, 35, 64},
        {200, 35, 128},
        {200, 40, 32},
        {200, 40, 64},
        {200, 40, 128}
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
