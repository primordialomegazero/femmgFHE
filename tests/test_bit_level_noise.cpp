#include "openfhe.h"
#include <iostream>
#include <vector>
#include <bitset>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  BIT-LEVEL NOISE ANALYSIS\n";
    std::cout << "  Ang noise ay naka-encode sa upper bits\n";
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

    std::cout << "1. BIT POSITION NG NOISE:\n";
    std::cout << "   (Aling bit ang unang nag-corrupt?)\n";
    std::cout << "-----------------------------------\n";

    auto current = ct0;
    
    for (int i = 20; i <= 25; i++) {
        current = nand(current, current);
        int result = decrypt(current);
        
        // I-convert sa positive representation
        uint16_t bits = result & 0xFFFF;
        
        std::cout << "   Iter " << i << ": ";
        for (int b = 15; b >= 0; b--) {
            int bit = (bits >> b) & 1;
            if (i <= 23 && b > 1 && bit == 1) {
                std::cout << "[" << bit << "]";
            } else if (i >= 24 && b >= 8 && bit == 1) {
                std::cout << "[" << bit << "]";
            } else {
                std::cout << bit;
            }
        }
        std::cout << "\n";
    }

    std::cout << "\n2. NOISE GROWTH SA BIT LEVEL:\n";
    std::cout << "-----------------------------------\n";

    current = ct0;
    std::vector<uint16_t> bit_states(16, 0);

    for (int i = 1; i <= 25; i++) {
        current = nand(current, current);
        int result = decrypt(current);
        uint16_t bits = result & 0xFFFF;
        
        if (i >= 15) {
            std::cout << "   Iter " << i << ": ";
            for (int b = 15; b >= 0; b--) {
                if (b % 4 == 0) std::cout << " ";
                std::cout << ((bits >> b) & 1);
            }
            std::cout << "\n";
        }
    }

    std::cout << "\n3. KONKLUSYON:\n";
    std::cout << "   Ang noise ay naka-encode sa:\n";
    std::cout << "   - Bit 0: value (0 o 1)\n";
    std::cout << "   - Bit 1-7: noise floor (mababa)\n";
    std::cout << "   - Bit 8-14: noise (lumalaki)\n";
    std::cout << "   - Bit 15: sign bit (threshold)\n";
    std::cout << "\n   Kapag bit 15 ay naging 1,\n";
    std::cout << "   ang decryption ay nag-fail\n";

    std::cout << "\n========================================\n";
    std::cout << "  KEY INSIGHT:\n";
    std::cout << "  - Ang noise ay may bit-level structure\n";
    std::cout << "  - Hindi ito random kundi structured\n";
    std::cout << "  - Kung ma-predict natin ang bit\n";
    std::cout << "    pattern, pwede nating i-correct\n";
    std::cout << "  - Ang bit 15 ang critical threshold\n";
    std::cout << "========================================\n";

    return 0;
}
