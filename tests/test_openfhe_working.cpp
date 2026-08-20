// OPENFHE WORKING — Tamang Parameters na Tiyak Gumagana
#include <NTL/ZZ.h>
#include "openfhe.h"
#include <iostream>
#include <vector>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  OPENFHE WORKING TEST\n";
    std::cout << "========================================\n\n";

    // I-verify muna kung anong depth ang gumagana sa ring 16384
    for (int depth : {1, 5, 10, 20}) {
        std::cout << "Testing depth " << depth << "... ";
        std::cout.flush();

        try {
            CCParams<CryptoContextBFVRNS> parameters;
            parameters.SetMultiplicativeDepth(depth);
            parameters.SetPlaintextModulus(65537);

            CryptoContext<DCRTPoly> cc = GenCryptoContext(parameters);
            cc->Enable(PKE);
            cc->Enable(KEYSWITCH);
            cc->Enable(LEVELEDSHE);

            auto ring_dim = cc->GetRingDimension();
            std::cout << "OK (ring=" << ring_dim << ")\n";
        } catch (std::exception& e) {
            std::cout << "FAIL: " << e.what() << "\n";
        }
    }

    return 0;
}
