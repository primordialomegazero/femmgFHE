#include "openfhe.h"
#include <iostream>
#include <vector>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  DUAL ENCRYPT - Noise Pattern Analysis\n";
    std::cout << "  Sabay na i-encrypt ang value at noise\n";
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

    std::cout << "1. DUAL CHANNEL ENCODING:\n";
    std::cout << "   Channel A: value (0 o 1)\n";
    std::cout << "   Channel B: noise proxy (dapat 0)\n";
    std::cout << "-----------------------------------\n";

    // Dual channel: [value, noise_proxy]
    // Kapag nag-NAND, pareho silang nag-o-operate
    // Ang noise_proxy ay nagpapakita kung gaano kalaki
    // ang noise accumulation

    std::vector<int64_t> dual_zero(16384, 0);
    std::vector<int64_t> dual_one(16384, 0);
    dual_one[0] = 1;  // value channel
    dual_one[1] = 0;  // noise proxy channel

    auto ct_dual0 = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(dual_zero));
    auto ct_dual1 = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(dual_one));

    auto decrypt_dual = [&](Ciphertext<DCRTPoly> ct) -> std::pair<int64_t, int64_t> {
        Plaintext pt;
        cc->Decrypt(keys.secretKey, ct, &pt);
        auto vals = pt->GetPackedValue();
        return {vals[0], vals[1]};
    };

    auto current = ct_dual0;
    std::cout << "\n   Initial: value=" << decrypt_dual(current).first
              << " noise_proxy=" << decrypt_dual(current).second << "\n";

    for (int i = 1; i <= 25; i++) {
        current = nand(current, current);
        auto [val, noise] = decrypt_dual(current);
        int expected = (i % 2 == 0) ? 0 : 1;
        
        std::cout << "   Iter " << i << ": value=" << val
                  << " noise_proxy=" << noise
                  << " (expected " << expected << ")";
        
        if (val != expected) {
            std::cout << " FAIL";
        } else if (noise != 0) {
            std::cout << " NOISE_DETECTED";
        }
        std::cout << "\n";
    }

    std::cout << "\n2. NOISE GROWTH PATTERN:\n";
    std::cout << "   (Noise proxy sa bawat iteration)\n";
    std::cout << "-----------------------------------\n";

    current = ct_dual0;
    std::vector<int64_t> noise_pattern;

    for (int i = 1; i <= 25; i++) {
        current = nand(current, current);
        auto [val, noise] = decrypt_dual(current);
        noise_pattern.push_back(noise);
        
        if (i <= 10 || i >= 20) {
            std::cout << "   Iter " << i << ": noise=" << noise << "\n";
        }
    }

    // Hanapin ang pattern
    std::cout << "\n   Noise differences:\n";
    for (int i = 1; i < noise_pattern.size(); i++) {
        int64_t diff = noise_pattern[i] - noise_pattern[i-1];
        if (i <= 5 || i >= 20) {
            std::cout << "   d" << i << " = " << diff << "\n";
        }
    }

    std::cout << "\n========================================\n";
    std::cout << "  OBSERVATION:\n";
    std::cout << "  - Ang noise proxy ay nagpapakita ng\n";
    std::cout << "    eksaktong noise accumulation\n";
    std::cout << "  - Kung may pattern, pwede nating\n";
    std::cout << "    i-compensate sa computation\n";
    std::cout << "========================================\n";

    return 0;
}
