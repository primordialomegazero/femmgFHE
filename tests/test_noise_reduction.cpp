#include "openfhe.h"
#include <iostream>
#include <vector>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  NOISE REDUCTION VIA REDUNDANT ENCODING\n";
    std::cout << "  Encode 0 as {0,0,0,...} at 1 as {1,1,1,...}\n";
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

    auto decrypt = [&](Ciphertext<DCRTPoly> ct) -> int64_t {
        Plaintext pt;
        cc->Decrypt(keys.secretKey, ct, &pt);
        return pt->GetPackedValue()[0];
    };

    auto nand = [&](Ciphertext<DCRTPoly> a, Ciphertext<DCRTPoly> b) {
        auto ab = cc->EvalMult(a, b);
        auto one = cc->MakePackedPlaintext({1});
        auto ct1 = cc->Encrypt(keys.publicKey, one);
        return cc->EvalSub(ct1, ab);
    };

    // REDUNDANT ENCODING: 
    // 0 = {0,0,0,0,...}
    // 1 = {1,1,1,1,...}
    // Kapag nag-multiply, ang noise ay kumakalat sa lahat ng slots
    // Ang average ay mas stable

    std::vector<int64_t> zeros(16384, 0);
    std::vector<int64_t> ones(16384, 1);

    auto ct0 = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(zeros));
    auto ct1 = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(ones));

    std::cout << "1. REDUNDANT NAND CHAIN:\n";
    std::cout << "   (All slots carry same value)\n";
    std::cout << "-----------------------------------\n";

    auto current = ct0;
    int errors = 0;

    for (int i = 1; i <= 30; i++) {
        current = nand(current, current);
        int result = decrypt(current);
        int expected = (i % 2 == 0) ? 0 : 1;
        if (result != expected) {
            errors++;
            if (errors <= 10) {
                std::cout << "   Iteration " << i << ": " << result
                          << " (expected " << expected << ") FAIL\n";
            }
        }
    }

    std::cout << "   Errors: " << errors << "\n";

    std::cout << "\n2. MAJORITY VOTE DECODING:\n";
    std::cout << "   (Tingnan ang distribution ng slots)\n";
    std::cout << "-----------------------------------\n";

    current = ct0;
    for (int i = 1; i <= 15; i++) {
        current = nand(current, current);
        Plaintext pt;
        cc->Decrypt(keys.secretKey, current, &pt);
        auto values = pt->GetPackedValue();
        
        int count_zero = 0;
        int count_one = 0;
        int count_other = 0;
        for (auto v : values) {
            if (v == 0) count_zero++;
            else if (v == 1) count_one++;
            else count_other++;
        }
        
        int expected = (i % 2 == 0) ? 0 : 1;
        std::cout << "   Iter " << i << ": zero=" << count_zero
                  << " one=" << count_one
                  << " other=" << count_other
                  << " (expected " << expected << ")\n";
    }

    std::cout << "\n========================================\n";
    std::cout << "  OBSERVATION:\n";
    std::cout << "  - Redundant encoding: " << (errors == 0 ? "STABLE" : "FAILS") << "\n";
    std::cout << "  - Majority decoding: " << "SEE DISTRIBUTION" << "\n";
    std::cout << "========================================\n";

    return 0;
}
