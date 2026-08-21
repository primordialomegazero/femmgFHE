// SAME MODULUS CROSS-CONTEXT
// Dalawang contexts na may parehong parameters
// Subukan kung kaya ng key switching ang transition

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  SAME MODULUS CROSS-CONTEXT\n";
    std::cout << "========================================\n\n";

    // Pareho ang parameters
    CCParams<CryptoContextCKKSRNS> params;
    params.SetMultiplicativeDepth(30);
    params.SetScalingModSize(40);
    params.SetBatchSize(256);

    auto ccA = GenCryptoContext(params);
    ccA->Enable(PKE);
    ccA->Enable(KEYSWITCH);
    ccA->Enable(LEVELEDSHE);

    auto ccB = GenCryptoContext(params);
    ccB->Enable(PKE);
    ccB->Enable(KEYSWITCH);
    ccB->Enable(LEVELEDSHE);

    auto keysA = ccA->KeyGen();
    ccA->EvalMultKeyGen(keysA.secretKey);

    auto keysB = ccB->KeyGen();
    ccB->EvalMultKeyGen(keysB.secretKey);

    std::cout << "Same parameters para sa A at B.\n\n";

    // Encrypt sa A
    auto slots = ccA->GetEncodingParams()->GetBatchSize();
    std::vector<std::complex<double>> vec(slots, {0.0, 0.0});
    vec[0] = {1.0, 0.0};

    auto ct_A = ccA->Encrypt(keysA.publicKey, ccA->MakeCKKSPackedPlaintext(vec));

    std::cout << "Ciphertext from A, level=" << ct_A->GetLevel() << "\n";

    // Subukan i-decrypt sa B
    try {
        Plaintext pt;
        ccB->Decrypt(keysB.secretKey, ct_A, &pt);
        double v = pt->GetCKKSPackedValue()[0].real();
        std::cout << "Decryption sa B: " << v << "\n";
    } catch (std::exception& e) {
        std::cout << "FAIL: " << e.what() << "\n";
    }

    return 0;
}
