// EVAL SWITCHKEYS BRIDGE
// Gumamit ng EvalSwitchKeys para lumipat ng key
// nang hindi nagde-decrypt

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  EVAL SWITCHKEYS BRIDGE\n";
    std::cout << "========================================\n\n";

    // Isang context lang
    CCParams<CryptoContextCKKSRNS> params;
    params.SetMultiplicativeDepth(30);
    params.SetScalingModSize(40);
    params.SetBatchSize(256);

    auto cc = GenCryptoContext(params);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);

    // Generate dalawang key pairs
    auto keysA = cc->KeyGen();
    auto keysB = cc->KeyGen();

    cc->EvalMultKeyGen(keysA.secretKey);

    // Generate switching key mula A papuntang B
    std::cout << "Generating switching key A→B...\n";
    auto switchKeyAB = cc->KeySwitchGen(keysA.secretKey, keysB.secretKey);
    std::cout << "Switching key A→B na-generate.\n\n";

    auto slots = cc->GetEncodingParams()->GetBatchSize();
    std::vector<std::complex<double>> vec(slots, {0.0, 0.0});
    vec[0] = {1.0, 0.0};

    // Encrypt gamit ang keysA
    auto ct_A = cc->Encrypt(keysA.publicKey, cc->MakeCKKSPackedPlaintext(vec));
    std::cout << "Ciphertext encrypted with keysA:\n";
    std::cout << "  Level: " << ct_A->GetLevel() << "\n\n";

    // Subukan i-decrypt gamit ang keysA
    Plaintext ptA;
    cc->Decrypt(keysA.secretKey, ct_A, &ptA);
    double vA = ptA->GetCKKSPackedValue()[0].real();
    std::cout << "Decryption with keysA: " << vA << "\n";

    // Subukan i-decrypt gamit ang keysB
    try {
        Plaintext ptB;
        cc->Decrypt(keysB.secretKey, ct_A, &ptB);
        double vB = ptB->GetCKKSPackedValue()[0].real();
        std::cout << "Decryption with keysB: " << vB << "\n";
    } catch (std::exception& e) {
        std::cout << "Decryption with keysB FAIL: " << e.what() << "\n";
    }

    // Subukan i-switch mula A papuntang B
    std::cout << "\nSwitching from keysA to keysB...\n";
    try {
        auto ct_B = cc->EvalSwitchKeys(ct_A, switchKeyAB);
        std::cout << "Switch successful!\n";

        Plaintext ptSwitched;
        cc->Decrypt(keysB.secretKey, ct_B, &ptSwitched);
        double vSwitched = ptSwitched->GetCKKSPackedValue()[0].real();
        std::cout << "Decryption after switch: " << vSwitched << "\n";
    } catch (std::exception& e) {
        std::cout << "Switch FAIL: " << e.what() << "\n";
    }

    return 0;
}
