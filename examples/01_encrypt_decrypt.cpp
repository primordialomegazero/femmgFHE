// Example 1: Encrypt and Decrypt a Number
// Compile: see docs/GETTING_STARTED.md
#include <openfhe.h>
using namespace lbcrypto;

int main() {
    CCParams<CryptoContextBFVRNS> params;
    params.SetPlaintextModulus(65537);
    params.SetRingDim(4096);
    
    auto cc = GenCryptoContext(params);
    cc->Enable(PKE);
    auto keys = cc->KeyGen();
    
    auto ct = cc->Encrypt(keys.publicKey, 
              cc->MakePackedPlaintext(std::vector<int64_t>{42}));
    
    Plaintext pt;
    cc->Decrypt(keys.secretKey, ct, &pt);
    std::cout << "Encrypted(42) -> Decrypt -> " << pt->GetPackedValue()[0] << std::endl;
    
    return 0;
}
