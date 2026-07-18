#include <openfhe.h>
#include <iostream>
#include <vector>
using namespace lbcrypto;
using namespace std;

const int64_t ETERNAL_KEY = 0xDEADBEEFCAFE1234;
const int W = 5;

int64_t mod(int64_t v, int64_t m) { return ((v%m)+m)%m; }

int main() {
    int64_t modulus = 1073643521;
    
    CCParams<CryptoContextBFVRNS> params;
    params.SetMultiplicativeDepth(30);
    params.SetPlaintextModulus(modulus);
    params.SetRingDim(4096);
    params.SetSecurityLevel(HEStd_NotSet);
    auto cc = GenCryptoContext(params);
    cc->Enable(PKE);cc->Enable(KEYSWITCH);cc->Enable(LEVELEDSHE);cc->Enable(ADVANCEDSHE);
    auto keys = cc->KeyGen();cc->EvalMultKeyGen(keys.secretKey);
    
    auto enc=[&](int64_t v){return cc->Encrypt(keys.publicKey,cc->MakePackedPlaintext(vector<int64_t>{mod(v,modulus)}));};
    auto dec=[&](const Ciphertext<DCRTPoly>& ct){Plaintext pt;cc->Decrypt(keys.secretKey,ct,&pt);return pt->GetPackedValue()[0];};
    
    // Test with WRONG key
    int64_t guard_key = 0;
    bool eternal_ok = (guard_key == ETERNAL_KEY);
    cout << "Guard key: " << guard_key << " | ETERNAL_KEY: " << ETERNAL_KEY << endl;
    cout << "eternal_ok: " << (eternal_ok ? "TRUE" : "FALSE") << endl;
    
    vector<Ciphertext<DCRTPoly>> state(W);
    if(eternal_ok) {
        cout << "BRANCH: correct key" << endl;
        state[0]=enc(1);
    } else {
        cout << "BRANCH: wrong key — GARBAGE" << endl;
        state[0]=enc(999999);
    }
    cout << "state[0] decrypts to: " << dec(state[0]) << endl;
    
    return 0;
}
