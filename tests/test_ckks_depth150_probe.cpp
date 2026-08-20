// CKKS DEPTH 150 PROBE
#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <chrono>

using namespace lbcrypto;
using namespace std::chrono;

int main() {
    std::cout << "========================================\n";
    std::cout << "  DEPTH 150 PROBE\n";
    std::cout << "========================================\n\n";

    for (int depth : {140, 150}) {
        for (int scaling : {40, 35, 30}) {
            std::cout << "Depth " << depth << ", scaling " << scaling << "... ";
            std::cout.flush();

            try {
                CCParams<CryptoContextCKKSRNS> params;
                params.SetMultiplicativeDepth(depth);
                params.SetScalingModSize(scaling);
                params.SetBatchSize(1024);

                CryptoContext<DCRTPoly> cc = GenCryptoContext(params);
                auto ring = cc->GetRingDimension();
                auto slots = cc->GetEncodingParams()->GetBatchSize();
                std::cout << "OK (ring=" << ring << ", slots=" << slots << ")\n";
            } catch (std::exception& e) {
                std::cout << "FAIL\n";
            }
        }
    }

    return 0;
}
