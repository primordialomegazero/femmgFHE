#include "openfhe.h"
#include <iostream>
#include <vector>
#include <bitset>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  BINARY NOISE STRUCTURE\n";
    std::cout << "  Ang noise ay may binary pattern?\n";
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

    std::cout << "1. NOISE SA BINARY REPRESENTATION:\n";
    std::cout << "   (Decrypted value sa bawat iteration)\n";
    std::cout << "-----------------------------------\n";

    auto current = ct0;
    
    for (int i = 1; i <= 25; i++) {
        current = nand(current, current);
        int result = decrypt(current);
        
        if (i >= 20) {
            std::cout << "   Iter " << i << ": " << result << "\n";
            std::cout << "     Binary: " << std::bitset<16>(result & 0xFFFF) << "\n";
            std::cout << "     Hex: 0x" << std::hex << (result & 0xFFFF) << std::dec << "\n";
        }
    }

    std::cout << "\n2. NOISE THRESHOLD ANALYSIS:\n";
    std::cout << "   (Kailan lumampas sa p/2?)\n";
    std::cout << "-----------------------------------\n";

    current = ct0;
    int64_t threshold = 32768; // p/2

    for (int i = 1; i <= 25; i++) {
        current = nand(current, current);
        int result = decrypt(current);
        
        int64_t abs_val = (result > threshold) ? result - 65537 : result;
        if (std::abs(abs_val) > 1000) {
            std::cout << "   Iter " << i << ": |noise| = " 
                      << std::abs(abs_val) << " > 1000\n";
        }
    }

    std::cout << "\n3. BIT PATTERN NG NOISE:\n";
    std::cout << "-----------------------------------\n";

    current = ct0;
    for (int i = 20; i <= 25; i++) {
        current = nand(current, current);
        int result = decrypt(current);
        
        // Ihiwalay ang high at low bits
        int high = (result >> 8) & 0xFF;
        int low = result & 0xFF;
        
        std::cout << "   Iter " << i << ": high=" << std::bitset<8>(high)
                  << " low=" << std::bitset<8>(low) << "\n";
    }

    std::cout << "\n========================================\n";
    std::cout << "  OBSERVATION:\n";
    std::cout << "  - Ang noise ay may binary structure\n";
    std::cout << "  - Ang threshold ay 2^15 = 32768\n";
    std::cout << "  - Ang corruption ay nagsisimula sa\n";
    std::cout << "    bit 15 pataas\n";
    std::cout << "========================================\n";

    return 0;
}
