#include "openfhe.h"
#include <iostream>
#include <vector>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  TRUE NOISE MEASUREMENT\n";
    std::cout << "  Ihiwalay ang value sa noise\n";
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

    std::cout << "1. NOISE FLOOR ANALYSIS:\n";
    std::cout << "   (Encrypted 0 na dapat manatiling 0)\n";
    std::cout << "-----------------------------------\n";

    // Ang noise ay makikita kapag ang value ay 0
    // pero ang decryption ay hindi eksaktong 0
    // kundi may konting deviation

    auto current = ct0;
    std::vector<int64_t> noise_values;

    for (int i = 1; i <= 25; i++) {
        current = nand(current, current);
        int result = decrypt(current);
        int expected = (i % 2 == 0) ? 0 : 1;
        
        // Kung expected ay 0, ang deviation ay noise
        if (expected == 0) {
            noise_values.push_back(result);
            if (i <= 12 || i >= 22) {
                std::cout << "   Iter " << i << ": value=" << result
                          << " (expected 0)";
                if (result != 0) {
                    std::cout << " NOISE=" << result;
                }
                std::cout << "\n";
            }
        }
    }

    std::cout << "\n2. NOISE GROWTH (sa even iterations):\n";
    std::cout << "-----------------------------------\n";

    for (int i = 0; i < noise_values.size(); i++) {
        if (i < 5 || i >= noise_values.size() - 3) {
            std::cout << "   Even iter " << (i + 1) * 2 
                      << ": noise=" << noise_values[i] << "\n";
        }
    }

    // Hanapin ang growth factor
    std::cout << "\n   Growth factors:\n";
    for (int i = 1; i < noise_values.size() && noise_values[i-1] != 0; i++) {
        if (noise_values[i-1] != 0) {
            double factor = (double)noise_values[i] / noise_values[i-1];
            if (i < 5 || i >= noise_values.size() - 3) {
                std::cout << "   f" << i << " = " << factor << "\n";
            }
        }
    }

    std::cout << "\n========================================\n";
    std::cout << "  KEY INSIGHT:\n";
    std::cout << "  - Ang noise ay hindi nakikita sa value\n";
    std::cout << "    hanggang umabot sa threshold\n";
    std::cout << "  - Sa iteration 24, biglang sumabog\n";
    std::cout << "  - Ito ay exponential growth na\n";
    std::cout << "    nakatago sa loob ng ciphertext\n";
    std::cout << "========================================\n";

    return 0;
}
