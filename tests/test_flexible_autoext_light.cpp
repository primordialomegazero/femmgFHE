// FLEXIBLEAUTOEXT — LIGHT SETUP
// Para sa 16GB RAM

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <chrono>

using namespace lbcrypto;
using namespace std::chrono;

int main() {
    std::cout << "========================================\n";
    std::cout << "  FLEXIBLEAUTOEXT — LIGHT SETUP\n";
    std::cout << "========================================\n\n";

    struct ParamSet {
        int depth;
        int scaling_mod_size;
        int batch_size;
    };

    std::vector<ParamSet> candidates = {
        {40, 30, 64},
        {40, 30, 128},
        {50, 25, 64},
        {50, 25, 128},
        {60, 25, 64},
        {60, 25, 128},
        {80, 20, 64},
        {80, 20, 128}
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
            params.SetScalingTechnique(FLEXIBLEAUTOEXT);
            params.SetKeySwitchTechnique(BV);

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
