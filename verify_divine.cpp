#include <openfhe/pke/openfhe.h>
#include <iostream>
using namespace lbcrypto;
using namespace std;

int main() {
    cout << "=== VERIFY DIVINE: 7 * 2^10 ===" << endl;
    
    CCParams<CryptoContextBFVRNS> params;
    params.SetMultiplicativeDepth(30);
    params.SetPlaintextModulus(1073643521);
    params.SetRingDim(16384);
    params.SetSecurityLevel(HEStd_NotSet);
    
    auto cc = GenCryptoContext(params);
    cc->Enable(PKE);
    cc->Enable(LEVELEDSHE);
    cc->Enable(KEYSWITCH);
    
    auto keys = cc->KeyGen();
    cc->EvalMultKeyGen(keys.secretKey);
    
    // Encrypt 7
    vector<int64_t> v = {7};
    auto ct = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(v));
    
    // Multiply by 2, 10 times (2^10 = 1024)
    for(int i = 0; i < 10; i++) {
        auto ct2 = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{2}));
        ct = cc->EvalMult(ct, ct2);
    }
    
    // Decrypt and check
    Plaintext result;
    cc->Decrypt(keys.secretKey, ct, &result);
    int64_t val = result->GetPackedValue()[0];
    
    cout << "7 * 2^10 = " << val << endl;
    cout << "Expected: 7168" << endl;
    cout << "VERIFIED: " << (val == 7168 ? "✅ YES" : "❌ NO") << endl;
    
    return 0;
}
