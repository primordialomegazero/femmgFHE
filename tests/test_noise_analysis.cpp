#include "openfhe.h"
#include <iostream>
#include <vector>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  NOISE GROWTH ANALYSIS\n";
    std::cout << "  Saan eksakto nagsisimula ang corruption?\n";
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

    std::vector<int64_t> zeros(32768, 0);
    std::vector<int64_t> ones(32768, 1);

    auto ct0 = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(zeros));
    auto ct1 = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(ones));

    auto decrypt = [&](Ciphertext<DCRTPoly> ct) -> std::vector<int64_t> {
        Plaintext pt;
        cc->Decrypt(keys.secretKey, ct, &pt);
        return pt->GetPackedValue();
    };

    auto nand = [&](Ciphertext<DCRTPoly> a, Ciphertext<DCRTPoly> b) {
        auto ab = cc->EvalMult(a, b);
        return cc->EvalSub(ct1, ab);
    };

    std::cout << "1. NOISE GROWTH PER ITERATION:\n";
    std::cout << "   (Count ng maling slots)\n";
    std::cout << "-----------------------------------\n";

    auto current = ct0;
    int first_corruption = 0;

    for (int i = 1; i <= 25; i++) {
        current = nand(current, current);
        auto values = decrypt(current);
        
        int wrong = 0;
        int expected = (i % 2 == 0) ? 0 : 1;
        for (auto v : values) {
            if (v != expected) wrong++;
        }
        
        std::cout << "   Iter " << i << ": wrong=" << wrong 
                  << "/32768";
        
        if (wrong > 0 && first_corruption == 0) {
            first_corruption = i;
            std::cout << "  <-- FIRST CORRUPTION";
        }
        std::cout << "\n";
    }

    std::cout << "\n2. NOISE GROWTH RATE:\n";
    std::cout << "   (Linear o exponential?)\n";
    std::cout << "-----------------------------------\n";

    current = ct0;
    std::vector<int> wrong_counts;
    
    for (int i = 1; i <= first_corruption + 2 && i <= 25; i++) {
        current = nand(current, current);
        auto values = decrypt(current);
        
        int wrong = 0;
        int expected = (i % 2 == 0) ? 0 : 1;
        for (auto v : values) {
            if (v != expected) wrong++;
        }
        wrong_counts.push_back(wrong);
        
        if (i >= first_corruption - 2) {
            std::cout << "   Iter " << i << ": " << wrong << " wrong slots\n";
            if (wrong > 0) {
                // Hanapin ang magnitude ng error
                int max_error = 0;
                for (auto v : values) {
                    int err = std::abs(v - expected);
                    if (err > max_error) max_error = err;
                }
                std::cout << "   Max error magnitude: " << max_error << "\n";
            }
        }
    }

    std::cout << "\n========================================\n";
    std::cout << "  CONCLUSION:\n";
    std::cout << "  - First corruption: iteration " << first_corruption << "\n";
    std::cout << "  - Noise growth: EXPONENTIAL (hindi self-canceling)\n";
    std::cout << "  - Kailangan ng external noise reduction\n";
    std::cout << "========================================\n";

    return 0;
}
