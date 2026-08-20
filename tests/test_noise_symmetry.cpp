#include "openfhe.h"
#include <iostream>
#include <vector>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  NOISE SYMMETRY EXPLOITATION\n";
    std::cout << "  May symmetric structure ba ang noise?\n";
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

    std::cout << "1. NOISE MIRROR TEST:\n";
    std::cout << "   (Forward + Backward NAND)\n";
    std::cout << "-----------------------------------\n";

    // Ideya: Kung ang noise ay may directional structure,
    // ang pag-apply ng NAND sa forward at backward
    // na direksyon ay magca-cancel

    auto forward = ct0;
    auto backward = ct0;
    int errors = 0;

    for (int i = 1; i <= 30; i++) {
        // Forward NAND
        forward = nand(forward, forward);
        
        // Backward NAND (reversed operands)
        auto f_copy = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext({decrypt(forward)}));
        backward = nand(f_copy, f_copy);
        
        // Combine: forward + backward
        auto combined = cc->EvalAdd(forward, backward);
        
        int result = decrypt(combined);
        int expected = (i % 2 == 0) ? 0 : 2;
        
        if (result != expected && result != expected - 1 && result != expected + 1) {
            errors++;
            if (errors <= 10) {
                std::cout << "   Iter " << i << ": " << result
                          << " (expected " << expected << ") FAIL\n";
            }
        }
    }

    std::cout << "   Errors: " << errors << "\n";

    std::cout << "\n2. NOISE CONJUGATION TEST:\n";
    std::cout << "   (Positive + Negative NAND)\n";
    std::cout << "-----------------------------------\n";

    // Ideya: Kung ang noise ay may sign structure,
    // ang pag-apply ng NAND at NOT(NAND) ay magca-cancel

    auto positive = ct0;
    auto negative = ct1;
    errors = 0;

    for (int i = 1; i <= 30; i++) {
        positive = nand(positive, positive);
        negative = nand(negative, negative);
        
        // Difference: positive - negative
        auto diff = cc->EvalSub(positive, negative);
        
        int result = decrypt(diff);
        int expected = (i % 2 == 0) ? -1 : 1;
        
        if (std::abs(result - expected) > 1) {
            errors++;
            if (errors <= 10) {
                std::cout << "   Iter " << i << ": " << result
                          << " (expected " << expected << ") FAIL\n";
            }
        }
    }

    std::cout << "   Errors: " << errors << "\n";

    std::cout << "\n3. PAIRED NOISE CANCELLATION:\n";
    std::cout << "   (NAND(x,x) + NAND(NOT(x), NOT(x)))\n";
    std::cout << "-----------------------------------\n";

    errors = 0;
    auto x = ct0;
    auto not_x = ct1;

    for (int i = 1; i <= 30; i++) {
        auto n1 = nand(x, x);
        auto n2 = nand(not_x, not_x);
        auto sum = cc->EvalAdd(n1, n2);
        
        int result = decrypt(sum);
        int expected = 1; // NAND(x,x) + NAND(NOT(x), NOT(x)) = NOT(x) + x = 1
        
        if (result != expected) {
            errors++;
            if (errors <= 10) {
                std::cout << "   Iter " << i << ": " << result
                          << " (expected " << expected << ") FAIL\n";
            }
        }
        
        x = n1;
        not_x = n2;
    }

    std::cout << "   Errors: " << errors << "\n";

    std::cout << "\n========================================\n";
    std::cout << "  RESULT:\n";
    std::cout << "  - Mirror: " << errors << " errors\n";
    std::cout << "  - Conjugation: " << errors << " errors\n";
    std::cout << "  - Paired: " << errors << " errors\n";
    std::cout << "========================================\n";

    return 0;
}
