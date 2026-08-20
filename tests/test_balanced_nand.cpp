#include "openfhe.h"
#include <iostream>
#include <vector>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  BALANCED NAND PAIRS\n";
    std::cout << "  NAND(a,b) + NAND(b,a) = symmetric\n";
    std::cout << "  May noise cancellation ba?\n";
    std::cout << "========================================\n\n";

    CCParams<CryptoContextBFVRNS> parameters;
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

    std::cout << "1. SYMMETRIC NAND PAIR:\n";
    std::cout << "   Result = NAND(a,b) + NAND(b,a)\n";
    std::cout << "   (Dapat 2 - 2ab, na may period-2)\n";
    std::cout << "-----------------------------------\n";

    auto current = ct0;
    int errors = 0;

    for (int i = 1; i <= 40; i++) {
        // Symmetric pair
        auto n1 = nand(current, current);
        auto n2 = nand(current, current);
        current = cc->EvalAdd(n1, n2);
        
        // Normalize to 0/1
        // current = current / 2 (pero walang division sa FHE)
        // Kaya gumamit ng comparison
        
        int expected = (i % 2 == 0) ? 0 : 1;
        int result = decrypt(current);
        if (result != expected) {
            errors++;
            if (errors <= 10) {
                std::cout << "   Iter " << i << ": " << result
                          << " (expected " << expected << ") FAIL\n";
            }
        }
    }

    std::cout << "   Errors: " << errors << "\n";

    std::cout << "\n2. DIFFERENCE NAND PAIR:\n";
    std::cout << "   Result = NAND(a,b) - NAND(b,a) = 0\n";
    std::cout << "   (Perfect cancellation?)\n";
    std::cout << "-----------------------------------\n";

    current = ct0;
    errors = 0;

    for (int i = 1; i <= 40; i++) {
        auto n1 = nand(current, current);
        auto n2 = nand(current, current);
        current = cc->EvalSub(n1, n2);
        
        // Dapat laging 0
        int result = decrypt(current);
        if (result != 0) {
            errors++;
            if (errors <= 10) {
                std::cout << "   Iter " << i << ": " << result
                          << " (expected 0) FAIL\n";
            }
        }
    }

    std::cout << "   Errors: " << errors << "\n";

    std::cout << "\n3. TRIPLE NAND BALANCE:\n";
    std::cout << "   Result = NAND(a,a) + NAND(a,a) - NAND(a,a)\n";
    std::cout << "   = NAND(a,a)\n";
    std::cout << "   (Noise: 2e + e - e = 2e?)\n";
    std::cout << "-----------------------------------\n";

    current = ct0;
    errors = 0;

    for (int i = 1; i <= 40; i++) {
        auto n1 = nand(current, current);
        auto n2 = nand(current, current);
        auto n3 = nand(current, current);
        auto sum = cc->EvalAdd(n1, n2);
        current = cc->EvalSub(sum, n3);
        
        int expected = (i % 2 == 0) ? 0 : 1;
        int result = decrypt(current);
        if (result != expected) {
            errors++;
            if (errors <= 10) {
                std::cout << "   Iter " << i << ": " << result
                          << " (expected " << expected << ") FAIL\n";
            }
        }
    }

    std::cout << "   Errors: " << errors << "\n";

    std::cout << "\n========================================\n";
    std::cout << "  RESULT:\n";
    std::cout << "  - Symmetric pair: " << errors << " errors\n";
    std::cout << "  - Difference pair: " << errors << " errors\n";
    std::cout << "  - Triple balance: " << errors << " errors\n";
    std::cout << "========================================\n";

    return 0;
}
