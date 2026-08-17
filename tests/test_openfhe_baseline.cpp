#include "openfhe.h"
#include <iostream>

using namespace lbcrypto;

int main() {
    std::cout << "OPENFHE BASELINE TEST\n";
    std::cout << "====================\n\n";

    // Setup CKKS parameters
    CCParams<CryptoContextCKKSRNS> parameters;
    parameters.SetMultiplicativeDepth(3);
    parameters.SetScalingModSize(50);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(parameters);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);

    auto keyPair = cc->KeyGen();
    cc->EvalMultKeyGen(keyPair.secretKey);

    std::vector<double> input = {1.0, 2.0, 3.0, 4.0};
    Plaintext ptxt = cc->MakeCKKSPackedPlaintext(input);
    auto ctxt = cc->Encrypt(keyPair.publicKey, ptxt);

    Plaintext result;
    cc->Decrypt(keyPair.secretKey, ctxt, &result);
    result->SetLength(4);

    std::cout << "Decrypted: ";
    for (int i = 0; i < 4; i++) {
        std::cout << result->GetCKKSPackedValue()[i] << " ";
    }
    std::cout << "\n";
    std::cout << "Expected: 1 2 3 4\n\n";

    std::cout << "OPENFHE ay gumagana! ✓\n";

    return 0;
}
