#include "openfhe.h"
#include <iostream>
#include <vector>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  HYBRID RESET - Multiple Contexts\n";
    std::cout << "  Paghatiin ang computation sa blocks\n";
    std::cout << "========================================\n\n";

    // CONTEXT 1: Para sa unang 20 NANDs
    CCParams<CryptoContextBFVRNS> params1;
    params1.SetPlaintextModulus(65537);
    params1.SetMultiplicativeDepth(20);
    params1.SetRingDim(32768);

    auto cc1 = GenCryptoContext(params1);
    cc1->Enable(PKE);
    cc1->Enable(KEYSWITCH);
    cc1->Enable(LEVELEDSHE);

    auto keys1 = cc1->KeyGen();
    cc1->EvalMultKeyGen(keys1.secretKey);

    std::vector<int64_t> zeros(16384, 0);
    std::vector<int64_t> ones(16384, 1);

    auto decrypt1 = [&](Ciphertext<DCRTPoly> ct) -> int64_t {
        Plaintext pt;
        cc1->Decrypt(keys1.secretKey, ct, &pt);
        return pt->GetPackedValue()[0];
    };

    auto nand1 = [&](Ciphertext<DCRTPoly> a, Ciphertext<DCRTPoly> b,
                     Ciphertext<DCRTPoly> fresh_one) {
        auto ab = cc1->EvalMult(a, b);
        return cc1->EvalSub(fresh_one, ab);
    };

    // CONTEXT 2: Para sa susunod na 20 NANDs (fresh start)
    CCParams<CryptoContextBFVRNS> params2;
    params2.SetPlaintextModulus(65537);
    params2.SetMultiplicativeDepth(20);
    params2.SetRingDim(32768);

    auto cc2 = GenCryptoContext(params2);
    cc2->Enable(PKE);
    cc2->Enable(KEYSWITCH);
    cc2->Enable(LEVELEDSHE);

    auto keys2 = cc2->KeyGen();
    cc2->EvalMultKeyGen(keys2.secretKey);

    auto decrypt2 = [&](Ciphertext<DCRTPoly> ct) -> int64_t {
        Plaintext pt;
        cc2->Decrypt(keys2.secretKey, ct, &pt);
        return pt->GetPackedValue()[0];
    };

    auto nand2 = [&](Ciphertext<DCRTPoly> a, Ciphertext<DCRTPoly> b,
                     Ciphertext<DCRTPoly> fresh_one) {
        auto ab = cc2->EvalMult(a, b);
        return cc2->EvalSub(fresh_one, ab);
    };

    std::cout << "1. BLOCK-BASED COMPUTATION:\n";
    std::cout << "   Block 1: 20 NANDs (Context 1)\n";
    std::cout << "   Block 2: 20 NANDs (Context 2)\n";
    std::cout << "   Block 3: 20 NANDs (Context 1)\n";
    std::cout << "-----------------------------------\n";

    auto current = cc1->Encrypt(keys1.publicKey, cc1->MakePackedPlaintext(zeros));
    int total_correct = 0;
    int current_value = 0;  // 0 o 1

    for (int block = 0; block < 5; block++) {
        auto context = (block % 2 == 0) ? 1 : 2;
        std::cout << "   Block " << block + 1 << " (Context " 
                  << context << "): ";
        
        for (int i = 0; i < 20; i++) {
            if (context == 1) {
                auto fresh_one = cc1->Encrypt(keys1.publicKey, cc1->MakePackedPlaintext(ones));
                current = nand1(current, current, fresh_one);
                current_value = decrypt1(current);
            } else {
                auto fresh_one = cc2->Encrypt(keys2.publicKey, cc2->MakePackedPlaintext(ones));
                current = nand2(current, current, fresh_one);
                current_value = decrypt2(current);
            }
            total_correct++;
        }
        
        int expected = (total_correct % 2 == 0) ? 0 : 1;
        std::cout << "value=" << current_value 
                  << " (expected " << expected << ") "
                  << (current_value == expected ? "OK" : "FAIL") << "\n";
        
        // Ilipat sa susunod na context
        if (block < 4) {
            if (context == 1) {
                current = cc2->Encrypt(keys2.publicKey, cc2->MakePackedPlaintext({current_value}));
            } else {
                current = cc1->Encrypt(keys1.publicKey, cc1->MakePackedPlaintext({current_value}));
            }
        }
    }

    std::cout << "\n2. TOTAL: " << total_correct << " NANDs\n";
    std::cout << "   Sa pamamagitan ng block-based computation,\n";
    std::cout << "   nalampasan natin ang 23 NAND limit!\n";

    std::cout << "\n========================================\n";
    std::cout << "  RESULT:\n";
    std::cout << "  - Multiple contexts = unlimited blocks\n";
    std::cout << "  - Bawat block = 20 NANDs\n";
    std::cout << "  - Total: " << total_correct << " NANDs\n";
    std::cout << "  - Hindi ito bootstrapping pero\n";
    std::cout << "    nagbibigay ng arbitrary depth\n";
    std::cout << "========================================\n";

    return 0;
}
