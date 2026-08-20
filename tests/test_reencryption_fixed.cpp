#include "openfhe.h"
#include <iostream>
#include <vector>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  RE-ENCRYPTION WITH FRESH ONES\n";
    std::cout << "  Fresh ct1 sa bawat cycle\n";
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

    auto decrypt = [&](Ciphertext<DCRTPoly> ct) -> int64_t {
        Plaintext pt;
        cc->Decrypt(keys.secretKey, ct, &pt);
        return pt->GetPackedValue()[0];
    };

    // NAND na laging gumagamit ng fresh ct1
    auto nand_fresh = [&](Ciphertext<DCRTPoly> a, Ciphertext<DCRTPoly> b,
                          Ciphertext<DCRTPoly> fresh_one) {
        auto ab = cc->EvalMult(a, b);
        return cc->EvalSub(fresh_one, ab);
    };

    auto ct0_original = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(zeros));
    auto ct1_original = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(ones));

    std::cout << "1. NAND CHAIN WITH FRESH ct1:\n";
    std::cout << "   (Fresh one every operation)\n";
    std::cout << "-----------------------------------\n";

    auto current = ct0_original;
    int max_correct = 0;
    int first_fail = 0;

    for (int i = 1; i <= 30; i++) {
        // Fresh ct1 sa bawat operation
        auto fresh_one = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(ones));
        current = nand_fresh(current, current, fresh_one);
        
        int expected = (i % 2 == 0) ? 0 : 1;
        int result = decrypt(current);
        if (result == expected) {
            max_correct = i;
        } else {
            first_fail = i;
            break;
        }
    }

    std::cout << "   Max correct: " << max_correct << " NANDs\n";
    if (first_fail > 0) {
        std::cout << "   First fail: " << first_fail << "\n";
    }

    std::cout << "\n2. RE-ENCRYPTION WITH FRESH ONES:\n";
    std::cout << "   (Fresh ct1 + re-encryption)\n";
    std::cout << "-----------------------------------\n";

    current = ct0_original;
    int errors = 0;
    int reencryptions = 0;

    for (int i = 1; i <= 50; i++) {
        auto fresh_one = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(ones));
        current = nand_fresh(current, current, fresh_one);
        
        if (i % 8 == 0) {
            Plaintext pt;
            cc->Decrypt(keys.secretKey, current, &pt);
            current = cc->Encrypt(keys.publicKey, pt);
            reencryptions++;
        }
        
        int expected = (i % 2 == 0) ? 0 : 1;
        int result = decrypt(current);
        if (result != expected) {
            errors++;
            if (errors <= 5) {
                std::cout << "   Iter " << i << ": " << result
                          << " (expected " << expected << ") FAIL\n";
            }
        }
    }

    std::cout << "   Re-encryptions: " << reencryptions << "\n";
    std::cout << "   Errors: " << errors << "\n";

    std::cout << "\n3. OPTIMAL INTERVAL WITH FRESH ONES:\n";
    std::cout << "-----------------------------------\n";

    for (int interval : {5, 8, 10, 12, 15}) {
        current = ct0_original;
        int interval_errors = 0;
        int max_correct_interval = 0;
        
        for (int i = 1; i <= 50; i++) {
            auto fresh_one = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(ones));
            current = nand_fresh(current, current, fresh_one);
            
            if (i % interval == 0) {
                Plaintext pt;
                cc->Decrypt(keys.secretKey, current, &pt);
                current = cc->Encrypt(keys.publicKey, pt);
            }
            
            int expected = (i % 2 == 0) ? 0 : 1;
            int result = decrypt(current);
            if (result == expected) {
                max_correct_interval = i;
            } else {
                interval_errors++;
                break;
            }
        }
        
        std::cout << "   Interval " << interval << ": max=" 
                  << max_correct_interval << " NANDs";
        if (interval_errors > 0) {
            std::cout << " (FAIL at " << max_correct_interval + 1 << ")";
        }
        std::cout << "\n";
    }

    std::cout << "\n========================================\n";
    std::cout << "  RESULT:\n";
    std::cout << "  - Fresh ct1: " << max_correct << " NANDs\n";
    std::cout << "  - With re-encryption: " 
              << (errors == 0 ? "ALL CORRECT" : "WITH ERRORS") << "\n";
    std::cout << "========================================\n";

    return 0;
}
