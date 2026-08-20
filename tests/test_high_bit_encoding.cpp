#include "openfhe.h"
#include <iostream>
#include <vector>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  HIGH BIT ENCODING\n";
    std::cout << "  I-encode ang value sa bit 14\n";
    std::cout << "  para ma-delay ang noise corruption\n";
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

    // HIGH BIT ENCODING:
    // 0 = 0 (binary 000...000)
    // 1 = 16384 = 2^14 (binary 010...000)
    // Sa pag-encode sa bit 14, ang noise ay
    // kailangang lumaki ng 16384 beses bago
    // ma-corrupt ang value

    int64_t high_one = 16384; // 2^14
    auto ct_high_one = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext({high_one}));

    auto nand_high = [&](Ciphertext<DCRTPoly> a, Ciphertext<DCRTPoly> b) {
        auto ab = cc->EvalMult(a, b);
        return cc->EvalSub(ct_high_one, ab);
    };

    std::cout << "1. HIGH BIT ENCODING TEST:\n";
    std::cout << "   Value 0 = 0, Value 1 = 2^14 = " << high_one << "\n";
    std::cout << "-----------------------------------\n";

    auto current = ct0;
    int max_correct = 0;
    int first_fail = 0;

    for (int i = 1; i <= 40; i++) {
        current = nand_high(current, current);
        
        int result = decrypt(current);
        int expected = (i % 2 == 0) ? 0 : high_one;
        
        if (result == expected) {
            max_correct = i;
        } else {
            first_fail = i;
            std::cout << "   Iter " << i << ": " << result
                      << " (expected " << expected << ") FAIL\n";
            break;
        }
        
        if (i <= 5 || i % 5 == 0) {
            std::cout << "   Iter " << i << ": " << result << " OK\n";
        }
    }

    std::cout << "\n   Max correct: " << max_correct << " NANDs\n";
    if (first_fail > 0) {
        std::cout << "   First fail: " << first_fail << "\n";
    }

    std::cout << "\n2. COMPARISON:\n";
    std::cout << "   Regular encoding: 23 NANDs\n";
    std::cout << "   High bit encoding: " << max_correct << " NANDs\n";
    std::cout << "   Improvement: " << (max_correct - 23) << " NANDs\n";

    std::cout << "\n========================================\n";
    std::cout << "  RESULT:\n";
    std::cout << "  - High bit encoding: " 
              << (max_correct > 23 ? "MAS MALALIM" : "WALANG IMPROVEMENT") << "\n";
    std::cout << "========================================\n";

    return 0;
}
