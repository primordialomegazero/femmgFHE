#include "openfhe.h"
#include <iostream>
#include <vector>
#include <chrono>

using namespace lbcrypto;
using namespace std::chrono;

int main() {
    std::cout << "========================================\n";
    std::cout << "  DEPTH vs NOISE THRESHOLD\n";
    std::cout << "  Hanapin ang optimal parameters\n";
    std::cout << "========================================\n\n";

    // Test different depth settings
    std::vector<uint32_t> depths = {20, 30, 40, 50, 60};
    std::vector<uint32_t> ring_dims = {32768, 65536};

    for (auto ring_dim : ring_dims) {
        for (auto depth : depths) {
            std::cout << "RingDim=" << ring_dim 
                      << " Depth=" << depth << ": ";
            std::cout.flush();

            try {
                CCParams<CryptoContextBFVRNS> parameters;
                parameters.SetPlaintextModulus(65537);
                parameters.SetMultiplicativeDepth(depth);
                parameters.SetRingDim(ring_dim);

                CryptoContext<DCRTPoly> cc = GenCryptoContext(parameters);
                cc->Enable(PKE);
                cc->Enable(KEYSWITCH);
                cc->Enable(LEVELEDSHE);

                auto keys = cc->KeyGen();
                cc->EvalMultKeyGen(keys.secretKey);

                std::vector<int64_t> zeros(ring_dim / 2, 0);
                std::vector<int64_t> ones(ring_dim / 2, 1);

                auto ct0 = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(zeros));
                auto ct1 = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(ones));

                auto decrypt = [&](Ciphertext<DCRTPoly> ct) -> int64_t {
                    Plaintext pt;
                    cc->Decrypt(keys.secretKey, ct, &pt);
                    return pt->GetPackedValue()[0];
                };

                auto nand = [&](Ciphertext<DCRTPoly> a, Ciphertext<DCRTPoly> b) {
                    auto ab = cc->EvalMult(a, b);
                    return cc->EvalSub(ct1, ab);
                };

                auto current = ct0;
                int max_correct = 0;
                int first_fail = 0;

                for (int i = 1; i <= 100; i++) {
                    current = nand(current, current);
                    int result = decrypt(current);
                    int expected = (i % 2 == 0) ? 0 : 1;
                    
                    if (result == expected) {
                        max_correct = i;
                    } else {
                        first_fail = i;
                        break;
                    }
                }

                std::cout << "max=" << max_correct 
                          << " NANDs before corruption";
                if (first_fail > 0) {
                    std::cout << " (fail at " << first_fail << ")";
                }
                std::cout << "\n";

            } catch (const std::exception& e) {
                std::cout << "FAILED: " << e.what() << "\n";
            }
        }
        std::cout << "\n";
    }

    std::cout << "========================================\n";
    std::cout << "  CONCLUSION:\n";
    std::cout << "  - Higher depth = more modulus budget\n";
    std::cout << "  - Larger ring dim = more slots, same noise\n";
    std::cout << "  - Pero may hardware limit (16GB RAM)\n";
    std::cout << "========================================\n";

    return 0;
}
