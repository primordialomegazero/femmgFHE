// CKKS DEPTH SCAN — Hanapin ang mga working depths
#include "openfhe.h"
#include <iostream>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  CKKS DEPTH SCAN\n";
    std::cout << "  Hanapin ang working depths\n";
    std::cout << "========================================\n\n";

    std::vector<int> working_depths;
    std::vector<int> failed_depths;

    for (int depth = 50; depth <= 150; depth += 5) {
        std::cout << "Depth " << depth << "... ";
        std::cout.flush();

        try {
            CCParams<CryptoContextCKKSRNS> params;
            params.SetMultiplicativeDepth(depth);
            params.SetScalingModSize(40);
            params.SetBatchSize(2048);

            CryptoContext<DCRTPoly> cc = GenCryptoContext(params);
            auto ring = cc->GetRingDimension();
            std::cout << "OK (ring=" << ring << ")";
            working_depths.push_back(depth);
        } catch (std::exception& e) {
            std::cout << "FAIL";
            failed_depths.push_back(depth);
        }
        std::cout << "\n";
    }

    std::cout << "\nWorking depths: ";
    for (int d : working_depths) std::cout << d << " ";
    std::cout << "\nFailed depths: ";
    for (int d : failed_depths) std::cout << d << " ";
    std::cout << "\n";

    return 0;
}
