#include "openfhe.h"
#include <iostream>
#include <vector>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  GOLDEN PERIOD 32 - Phase Analysis\n";
    std::cout << "  φ³² ≡ φ mod 65537\n";
    std::cout << "  May noise reset ba sa period 32?\n";
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

    // PERIOD 32 ANG SUSI
    // φ = 32769 = -32768 mod 65537
    // φ² = 49153
    // φ³² = φ
    // Ibig sabihin: φ¹⁶ = -1 (half period)

    std::cout << "1. HALF-PERIOD COMPENSATION:\n";
    std::cout << "   (I-compensate sa iteration 16)\n";
    std::cout << "-----------------------------------\n";

    int64_t phi = 32769;
    int64_t phi16 = 1;
    for (int i = 0; i < 16; i++) {
        phi16 = (phi16 * phi) % 65537;
    }
    
    std::cout << "   φ¹⁶ mod 65537 = " << phi16 << "\n";
    std::cout << "   (Dapat -1 kung may clean period)\n\n";

    // Test: NAND chain na may period-16 compensation
    auto current = ct0;
    int errors = 0;
    int max_correct = 0;

    for (int i = 1; i <= 40; i++) {
        current = nand(current, current);
        
        // Sa iteration 16, i-multiply sa φ¹⁶ = -1
        if (i == 16) {
            auto neg_ct = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext({phi16}));
            current = cc->EvalMult(current, neg_ct);
        }
        
        int expected = (i % 2 == 0) ? 0 : 1;
        int result = decrypt(current);
        if (result == expected) {
            max_correct = i;
        } else {
            errors++;
            if (errors <= 5) {
                std::cout << "   Iter " << i << ": " << result
                          << " (expected " << expected << ") FAIL\n";
            }
        }
    }

    std::cout << "   Max correct: " << max_correct << " NANDs\n";
    std::cout << "   Errors: " << errors << "\n";

    std::cout << "\n2. PERIOD-32 NOISE FLOOR:\n";
    std::cout << "   (Noise sa bawat kalahati ng period)\n";
    std::cout << "-----------------------------------\n";

    current = ct0;
    std::vector<int64_t> noise_floor;

    for (int i = 1; i <= 25; i++) {
        current = nand(current, current);
        
        int result = decrypt(current);
        int expected = (i % 2 == 0) ? 0 : 1;
        int noise = result - expected;
        
        if (i % 4 == 0 || i >= 20) {
            std::cout << "   Iter " << i << ": noise=" << noise << "\n";
        }
    }

    std::cout << "\n========================================\n";
    std::cout << "  KEY INSIGHT:\n";
    std::cout << "  - φ¹⁶ = " << phi16 << "\n";
    std::cout << "  - Kung ito ay -1, may phase shift\n";
    std::cout << "  - Ang period 32 ay emergent property\n";
    std::cout << "    ng Fermat prime 65537\n";
    std::cout << "========================================\n";

    return 0;
}
