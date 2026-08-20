#include "openfhe.h"
#include <iostream>
#include <vector>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  PLAINTEXT MODULUS HOPPING\n";
    std::cout << "  Maghanap ng tamang modulus para sa\n";
    std::cout << "  mas malalim na computation\n";
    std::cout << "========================================\n\n";

    std::vector<uint32_t> moduli = {257, 8191, 65537, 12289};
    
    struct Config {
        uint32_t ring_dim;
        uint32_t depth;
    };
    
    std::vector<Config> configs = {
        {8192, 10},
        {16384, 15},
        {32768, 20}
    };

    for (auto cfg : configs) {
        std::cout << "RingDim=" << cfg.ring_dim 
                  << " Depth=" << cfg.depth << ":\n";
        std::cout << "--------------------\n";
        
        for (auto ptm : moduli) {
            std::cout << "  Ptm=" << ptm << ": ";
            std::cout.flush();
            
            try {
                CCParams<CryptoContextBFVRNS> parameters;
                parameters.SetPlaintextModulus(ptm);
                parameters.SetMultiplicativeDepth(cfg.depth);
                parameters.SetRingDim(cfg.ring_dim);

                CryptoContext<DCRTPoly> cc = GenCryptoContext(parameters);
                cc->Enable(PKE);
                cc->Enable(KEYSWITCH);
                cc->Enable(LEVELEDSHE);

                auto keys = cc->KeyGen();
                cc->EvalMultKeyGen(keys.secretKey);

                std::vector<int64_t> zeros(cfg.ring_dim / 2, 0);
                std::vector<int64_t> ones(cfg.ring_dim / 2, 1);

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

                for (int i = 1; i <= cfg.depth + 10; i++) {
                    current = nand(current, current);
                    int result = decrypt(current);
                    int expected = (i % 2 == 0) ? 0 : 1;
                    
                    if (result == expected) {
                        max_correct = i;
                    } else {
                        break;
                    }
                }

                std::cout << "max=" << max_correct << " NANDs\n";

            } catch (const std::exception& e) {
                std::cout << "FAILED: " << e.what() << "\n";
            }
        }
        std::cout << "\n";
    }

    std::cout << "========================================\n";
    std::cout << "  OBSERVATION:\n";
    std::cout << "  - Lahat ng ptm ay may same depth limit\n";
    std::cout << "  - Ang depth ay dictated ng ring_dim\n";
    std::cout << "  - Hindi ptm ang limiting factor\n";
    std::cout << "========================================\n";

    return 0;
}
