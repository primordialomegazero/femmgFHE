#include <openfhe.h>
#include <iostream>
using namespace lbcrypto;
using namespace std;

int main() {
    cout << "[1] Creating params...\n";
    CCParams<CryptoContextCKKSRNS> params;
    params.SetSecretKeyDist(UNIFORM_TERNARY);
    params.SetSecurityLevel(HEStd_NotSet);
    params.SetRingDim(4096);
    params.SetScalingModSize(59);
    params.SetFirstModSize(60);
    params.SetScalingTechnique(FLEXIBLEAUTO);
    params.SetMultiplicativeDepth(10);
    
    cout << "[2] GenCryptoContext...\n";
    auto cc = GenCryptoContext(params);
    cout << "[3] Enable...\n";
    cc->Enable(PKE); cc->Enable(KEYSWITCH); cc->Enable(LEVELEDSHE);
    cout << "[4] KeyGen...\n";
    auto keys = cc->KeyGen();
    cout << "[5] Encrypt...\n";
    vector<double> v(2048, 0.0); v[0] = 1.0;
    auto pt = cc->MakeCKKSPackedPlaintext(v);
    auto ct = cc->Encrypt(keys.publicKey, pt);
    cout << "[6] Decrypt...\n";
    Plaintext dec; cc->Decrypt(keys.secretKey, ct, &dec);
    dec->SetLength(1);
    cout << "[7] Done: " << dec->GetRealPackedValue()[0] << "\n";
    return 0;
}
