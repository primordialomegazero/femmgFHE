// Example 3: ZANS — Unlimited Encrypted Additions
#include <openfhe.h>
#include <iostream>
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
    auto anchor = cc->Encrypt(keys.publicKey, 
              cc->MakePackedPlaintext(std::vector<int64_t>{0}));
    
    for(int i = 0; i < 10000; i++) {
        ct = cc->EvalAdd(ct, anchor);
    }
    
    Plaintext pt;
    cc->Decrypt(keys.secretKey, ct, &pt);
    std::cout << "After 10000 ZANS additions: " << pt->GetPackedValue()[0] 
              << " (should be 42)" << std::endl;
    
    return 0;
}
