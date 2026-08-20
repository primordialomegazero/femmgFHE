#include "openfhe.h"
#include <iostream>
#include <vector>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  RE-ENCRYPTION AS NOISE RESET\n";
    std::cout << "  Decrypt + Re-encrypt bago maubos\n";
    std::cout << "  ang modulus towers\n";
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

    std::cout << "1. SAFE RE-ENCRYPTION (every 5 NANDs):\n";
    std::cout << "   (Conservative interval)\n";
    std::cout << "-----------------------------------\n";

    auto current = ct0;
    int errors = 0;
    int reencryptions = 0;

    for (int i = 1; i <= 50; i++) {
        current = nand(current, current);
        
        if (i % 5 == 0) {
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

    std::cout << "\n2. OPTIMAL RE-ENCRYPTION INTERVAL:\n";
    std::cout << "-----------------------------------\n";

    for (int interval : {3, 5, 8, 10, 15}) {
        current = ct0;
        int interval_errors = 0;
        int max_correct = 0;
        
        for (int i = 1; i <= 50; i++) {
            current = nand(current, current);
            
            if (i % interval == 0) {
                Plaintext pt;
                cc->Decrypt(keys.secretKey, current, &pt);
                current = cc->Encrypt(keys.publicKey, pt);
            }
            
            int expected = (i % 2 == 0) ? 0 : 1;
            int result = decrypt(current);
            if (result == expected) {
                max_correct = i;
            } else {
                interval_errors++;
                break;
            }
        }
        
        std::cout << "   Interval " << interval << ": max=" 
                  << max_correct << " NANDs";
        if (interval_errors > 0) {
            std::cout << " (FAIL at " << max_correct + 1 << ")";
        }
        std::cout << "\n";
    }

    std::cout << "\n========================================\n";
    std::cout << "  RESULT:\n";
    std::cout << "  - Re-encryption works sa maikling interval\n";
    std::cout << "  - Hindi ito bootstrapping kundi reset\n";
    std::cout << "  - Ang tunay na FHE bootstrapping ay\n";
    std::cout << "    kailangan para sa arbitrary depth\n";
    std::cout << "========================================\n";

    return 0;
}
