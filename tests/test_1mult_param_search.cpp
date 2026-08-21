// 1-MULT NAND — PARAMETER SEARCH
// Maghanap ng working parameters para sa depth 60 at 80

#include "openfhe.h"
#include <iostream>
#include <chrono>

using namespace lbcrypto;
using namespace std::chrono;

int main() {
    std::cout << "========================================\n";
    std::cout << "  1-MULT NAND — PARAMETER SEARCH\n";
    std::cout << "========================================\n\n";

    struct ParamSet {
        int depth;
        int scaling_mod_size;
        int batch_size;
    };

    std::vector<ParamSet> candidates = {
        {60, 30, 128},
        {60, 30, 256},
        {60, 35, 128},
        {60, 35, 256},
        {60, 40, 128},
        {60, 40, 256},
        {80, 30, 128},
        {80, 30, 256},
        {80, 35, 128},
        {80, 35, 256},
        {80, 40, 128},
        {80, 40, 256}
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
