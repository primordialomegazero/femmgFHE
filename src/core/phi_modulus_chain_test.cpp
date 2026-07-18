#include <openfhe.h>
#include <iostream>
#include <iomanip>
using namespace lbcrypto;
using namespace std;
int main() {
    CCParams<CryptoContextBFVRNS> params;
    params.SetMultiplicativeDepth(30);
    params.SetPlaintextModulus(1073643521);
    params.SetRingDim(4096);
    params.SetSecurityLevel(HEStd_NotSet);
    auto cc = GenCryptoContext(params);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);
    auto keys = cc->KeyGen();
    cc->EvalMultKeyGen(keys.secretKey);
    
    auto ct = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{1}));
    auto two = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{2}));
    
    cout << "=== MODULUS CHAIN TRACKING ===" << endl;
    cout << "Initial level: " << ct->GetLevel() << endl;
    
    for (int i = 0; i < 35; i++) {
        ct = cc->EvalMult(ct, two);
        cout << "Step " << setw(2) << i+1 
             << " | Level: " << ct->GetLevel()
             << " | ScaleDeg: " << ct->GetNoiseScaleDeg()
             << endl;
    }
}
