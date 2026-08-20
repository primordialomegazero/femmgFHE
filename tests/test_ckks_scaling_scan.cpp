// CKKS SCALING SCAN — Hanapin ang minimum na scaling
#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <chrono>

using namespace lbcrypto;
using namespace std::chrono;

int main() {
    std::cout << "========================================\n";
    std::cout << "  CKKS SCALING SCAN\n";
    std::cout << "  Minimum scaling na gumagana\n";
    std::cout << "========================================\n\n";

    // I-scan ang iba't ibang scaling values
    for (int scaling : {40, 35, 30, 28, 26, 24, 22, 20}) {
        std::cout << "Scaling " << scaling << " bits, depth 30... ";
        std::cout.flush();

        try {
            CCParams<CryptoContextCKKSRNS> params;
            params.SetMultiplicativeDepth(30);
            params.SetScalingModSize(scaling);
            params.SetBatchSize(2048);

            CryptoContext<DCRTPoly> cc = GenCryptoContext(params);
            cc->Enable(PKE);
            cc->Enable(KEYSWITCH);
            cc->Enable(LEVELEDSHE);

            auto keys = cc->KeyGen();
            cc->EvalMultKeyGen(keys.secretKey);

            std::cout << "OK\n";
        } catch (std::exception& e) {
            std::cout << "FAIL: " << e.what() << "\n";
        }
    }

    return 0;
}
