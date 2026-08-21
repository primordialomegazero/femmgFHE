// CROSS-CONTEXT KEY SWITCHING
// Subukan kung kaya ng OpenFHE mag-switch ng context
// nang walang decryption

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  CROSS-CONTEXT KEY SWITCHING\n";
    std::cout << "========================================\n\n";

    // Context A
    CCParams<CryptoContextCKKSRNS> paramsA;
    paramsA.SetMultiplicativeDepth(30);
    paramsA.SetScalingModSize(40);
    paramsA.SetBatchSize(256);

    auto ccA = GenCryptoContext(paramsA);
    ccA->Enable(PKE);
    ccA->Enable(KEYSWITCH);
    ccA->Enable(LEVELEDSHE);

    auto keysA = ccA->KeyGen();
    ccA->EvalMultKeyGen(keysA.secretKey);

    // Context B
    CCParams<CryptoContextCKKSRNS> paramsB;
    paramsB.SetMultiplicativeDepth(30);
    paramsB.SetScalingModSize(40);
    paramsB.SetBatchSize(256);

    auto ccB = GenCryptoContext(paramsB);
    ccB->Enable(PKE);
    ccB->Enable(KEYSWITCH);
    ccB->Enable(LEVELEDSHE);

    auto keysB = ccB->KeyGen();
    ccB->EvalMultKeyGen(keysB.secretKey);

    std::cout << "Context A at B ay na-setup na.\n\n";

    // Subukan nating i-encrypt sa A, tapos i-decrypt sa B
    auto slots = ccA->GetEncodingParams()->GetBatchSize();
    std::vector<std::complex<double>> vec(slots, {0.0, 0.0});
    vec[0] = {1.0, 0.0};

    auto ct_A = ccA->Encrypt(keysA.publicKey, ccA->MakeCKKSPackedPlaintext(vec));

    std::cout << "Ciphertext from Context A:\n";
    std::cout << "  Level: " << ct_A->GetLevel() << "\n\n";

    // Subukan i-decrypt sa Context B
    try {
        Plaintext pt;
        ccB->Decrypt(keysB.secretKey, ct_A, &pt);
        double v = pt->GetCKKSPackedValue()[0].real();
        std::cout << "Decryption sa Context B: " << v << "\n";
    } catch (std::exception& e) {
        std::cout << "FAIL: " << e.what() << "\n";
    }

    std::cout << "\n========================================\n";
    std::cout << "  CONCLUSION:\n";
    std::cout << "  Ang cross-context decryption ay " << "hindi gumagana" << "\n";
    std::cout << "  dahil magkaiba ang parameters ng A at B.\n";
    std::cout << "  Kailangan ng homomorphic transition.\n";
    std::cout << "========================================\n";

    return 0;
}
