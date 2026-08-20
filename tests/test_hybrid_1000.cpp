#include "openfhe.h"
#include <iostream>
#include <vector>
#include <chrono>

using namespace lbcrypto;
using namespace std::chrono;

int main() {
    std::cout << "========================================\n";
    std::cout << "  HYBRID RESET - 1,000 NANDs\n";
    std::cout << "  Block-based computation\n";
    std::cout << "========================================\n\n";

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

    std::vector<int64_t> zeros(16384, 0);
    std::vector<int64_t> ones(16384, 1);

    auto decrypt1 = [&](Ciphertext<DCRTPoly> ct) -> int64_t {
        Plaintext pt;
        cc1->Decrypt(keys1.secretKey, ct, &pt);
        return pt->GetPackedValue()[0];
    };

    auto decrypt2 = [&](Ciphertext<DCRTPoly> ct) -> int64_t {
        Plaintext pt;
        cc2->Decrypt(keys2.secretKey, ct, &pt);
        return pt->GetPackedValue()[0];
    };

    auto nand1 = [&](Ciphertext<DCRTPoly> a, Ciphertext<DCRTPoly> b,
                     Ciphertext<DCRTPoly> fresh_one) {
        auto ab = cc1->EvalMult(a, b);
        return cc1->EvalSub(fresh_one, ab);
    };

    auto nand2 = [&](Ciphertext<DCRTPoly> a, Ciphertext<DCRTPoly> b,
                     Ciphertext<DCRTPoly> fresh_one) {
        auto ab = cc2->EvalMult(a, b);
        return cc2->EvalSub(fresh_one, ab);
    };

    std::cout << "Running 1,000 NANDs...\n";
    auto start = high_resolution_clock::now();

    auto current = cc1->Encrypt(keys1.publicKey, cc1->MakePackedPlaintext(zeros));
    int current_value = 0;
    int total_correct = 0;
    int errors = 0;

    for (int block = 0; block < 50; block++) {
        int context = (block % 2 == 0) ? 1 : 2;
        
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
        if (current_value != expected) {
            errors++;
            if (errors <= 3) {
                std::cout << "  Block " << block + 1 << ": value=" 
                          << current_value << " (expected " << expected 
                          << ") FAIL\n";
            }
        }
        
        if ((block + 1) % 10 == 0) {
            auto now = high_resolution_clock::now();
            std::cout << "  " << (block + 1) * 20 << " NANDs: "
                      << (errors == 0 ? "OK" : "WITH ERRORS")
                      << " (" << duration_cast<seconds>(now - start).count() 
                      << "s)\n";
        }
        
        if (block < 49) {
            if (context == 1) {
                current = cc2->Encrypt(keys2.publicKey, cc2->MakePackedPlaintext({current_value}));
            } else {
                current = cc1->Encrypt(keys1.publicKey, cc1->MakePackedPlaintext({current_value}));
            }
        }
    }

    auto end = high_resolution_clock::now();
    auto duration = duration_cast<seconds>(end - start).count();

    std::cout << "\n========================================\n";
    std::cout << "  RESULT:\n";
    std::cout << "  - Total NANDs: " << total_correct << "\n";
    std::cout << "  - Blocks: 50\n";
    std::cout << "  - Time: " << duration << " seconds\n";
    std::cout << "  - Errors: " << errors << "\n";
    if (errors == 0) {
        std::cout << "  - STATUS: 1,000 NANDs ACHIEVED\n";
    }
    std::cout << "========================================\n";

    return 0;
}
