#include "openfhe.h"
#include <iostream>
#include <vector>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  SQRT(-1) = 256 PERIOD 4 ANALYSIS\n";
    std::cout << "  May 4-cycle na noise correction?\n";
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

    std::cout << "1. SQRT(-1) CYCLE:\n";
    std::cout << "   256^1 = 256\n";
    std::cout << "   256^2 = " << (256 * 256) % 65537 << " = -1\n";
    std::cout << "   256^3 = " << (256 * 256 * 256) % 65537 << " = -256\n";
    std::cout << "   256^4 = " << (256 * 256 * 256 * 256) % 65537 << " = 1\n\n";

    std::cout << "2. NAND CHAIN WITH 4-CYCLE CORRECTION:\n";
    std::cout << "   (Multiply by 256 every 4th NAND)\n";
    std::cout << "-----------------------------------\n";

    auto current = ct0;
    int errors = 0;
    int max_correct = 0;

    for (int i = 1; i <= 50; i++) {
        current = nand(current, current);
        
        if (i % 4 == 0) {
            auto corr = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext({256}));
            current = cc->EvalMult(current, corr);
        }
        
        int expected = (i % 2 == 0) ? 0 : 1;
        int result = decrypt(current);
        if (result == expected) {
            max_correct = i;
        } else {
            errors++;
            if (errors <= 10) {
                std::cout << "   Iter " << i << ": " << result
                          << " (expected " << expected << ") FAIL\n";
            }
        }
    }

    std::cout << "   Max correct: " << max_correct << " NANDs\n";
    std::cout << "   Errors: " << errors << "\n";

    std::cout << "\n3. ALTERNATING CORRECTION:\n";
    std::cout << "   (256, -1, -256, 1 cycle)\n";
    std::cout << "-----------------------------------\n";

    current = ct0;
    errors = 0;
    max_correct = 0;
    
    std::vector<int64_t> cycle = {256, 65536, 65281, 1};

    for (int i = 1; i <= 50; i++) {
        current = nand(current, current);
        
        int cycle_idx = (i - 1) % 4;
        auto corr = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext({cycle[cycle_idx]}));
        current = cc->EvalMult(current, corr);
        
        int expected = (i % 2 == 0) ? 0 : 1;
        int result = decrypt(current);
        if (result == expected) {
            max_correct = i;
        } else {
            errors++;
            if (errors <= 10) {
                std::cout << "   Iter " << i << ": " << result
                          << " (expected " << expected << ") FAIL\n";
            }
        }
    }

    std::cout << "   Max correct: " << max_correct << " NANDs\n";
    std::cout << "   Errors: " << errors << "\n";

    std::cout << "\n4. PHASE SHIFT ANALYSIS:\n";
    std::cout << "   (I-offset ang cycle)\n";
    std::cout << "-----------------------------------\n";

    for (int offset = 0; offset < 4; offset++) {
        current = ct0;
        int offset_errors = 0;
        int offset_max = 0;
        
        for (int i = 1; i <= 30; i++) {
            current = nand(current, current);
            
            int cycle_idx = (i - 1 + offset) % 4;
            auto corr = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext({cycle[cycle_idx]}));
            current = cc->EvalMult(current, corr);
            
            int expected = (i % 2 == 0) ? 0 : 1;
            int result = decrypt(current);
            if (result == expected) {
                offset_max = i;
            } else {
                offset_errors++;
                break;
            }
        }
        
        std::cout << "   Offset " << offset << ": max=" << offset_max << "\n";
    }

    std::cout << "\n========================================\n";
    std::cout << "  KEY INSIGHT:\n";
    std::cout << "  - sqrt(-1) = 256 ay may period 4\n";
    std::cout << "  - 4-cycle correction: " << max_correct << " NANDs\n";
    std::cout << "  - Kung may phase match, unlimited\n";
    std::cout << "========================================\n";

    return 0;
}
