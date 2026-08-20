#include "openfhe.h"
#include <iostream>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  BGV SCHEME TEST\n";
    std::cout << "  May mas magandang noise properties\n";
    std::cout << "  kaysa BFV para sa NAND chains\n";
    std::cout << "========================================\n\n";

    CCParams<CryptoContextBGVRNS> parameters;
    parameters.SetPlaintextModulus(65537);
    parameters.SetMultiplicativeDepth(20);
    parameters.SetRingDim(32768);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(parameters);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);

    auto keys = cc->KeyGen();
    cc->EvalMultKeyGen(keys.secretKey);

    std::vector<int64_t> zeros(16384, 0);
    std::vector<int64_t> ones(16384, 1);

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

    std::cout << "BGV NAND CHAIN:\n";
    std::cout << "----------------\n";

    auto current = ct0;
    int max_correct = 0;

    for (int i = 1; i <= 30; i++) {
        current = nand(current, current);
        int result = decrypt(current);
        int expected = (i % 2 == 0) ? 0 : 1;
        
        if (result == expected) {
            max_correct = i;
            if (i <= 10 || i % 5 == 0) {
                std::cout << "  Iter " << i << ": OK\n";
            }
        } else {
            std::cout << "  Iter " << i << ": " << result
                      << " (expected " << expected << ") FAIL\n";
            break;
        }
    }

    std::cout << "\n========================================\n";
    std::cout << "  BGV max NANDs: " << max_correct << "\n";
    std::cout << "  BFV max NANDs: 23 (from previous test)\n";
    std::cout << "========================================\n";

    return 0;
}
