// Example 2: Add Two Encrypted Numbers
#include <openfhe.h>
using namespace lbcrypto;

int main() {
    CCParams<CryptoContextBFVRNS> params;
    params.SetPlaintextModulus(65537);
    params.SetRingDim(4096);
    
    auto cc = GenCryptoContext(params);
    cc->Enable(PKE);
    auto keys = cc->KeyGen();
    
    auto ct1 = cc->Encrypt(keys.publicKey, 
              cc->MakePackedPlaintext(std::vector<int64_t>{10}));
    auto ct2 = cc->Encrypt(keys.publicKey, 
              cc->MakePackedPlaintext(std::vector<int64_t>{20}));
    
    auto ct_sum = cc->EvalAdd(ct1, ct2);
    
    Plaintext pt;
    cc->Decrypt(keys.secretKey, ct_sum, &pt);
    std::cout << "10 + 20 = " << pt->GetPackedValue()[0] << std::endl;
    
    return 0;
}
