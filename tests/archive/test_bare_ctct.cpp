#include <openfhe.h>
#include <iostream>
#include <iomanip>
#include <vector>
#include <cmath>
using namespace lbcrypto;
using namespace std;

int main() {
    CCParams<CryptoContextCKKSRNS> params;
    params.SetSecretKeyDist(UNIFORM_TERNARY);
    params.SetSecurityLevel(HEStd_NotSet);
    params.SetRingDim(4096);
    params.SetScalingModSize(59);
    params.SetFirstModSize(60);
    params.SetScalingTechnique(FLEXIBLEAUTO);
    params.SetMultiplicativeDepth(100);
    auto cc = GenCryptoContext(params);
    cc->Enable(PKE); cc->Enable(KEYSWITCH); cc->Enable(LEVELEDSHE); cc->Enable(ADVANCEDSHE);
    auto keys = cc->KeyGen();
    cc->EvalMultKeyGen(keys.secretKey);
    
    auto enc = [&](double v) {
        vector<double> vec(2048, 0.0); vec[0] = v;
        return cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(vec));
    };
    auto dec = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext pt; cc->Decrypt(keys.secretKey, ct, &pt);
        pt->SetLength(2048); return pt->GetRealPackedValue()[0];
    };

    struct PE { Ciphertext<DCRTPoly> a, b; };
    
    auto ctct = [&](const PE& x, const PE& y) -> PE {
        auto ac = cc->EvalMult(x.a, y.a);
        auto bd = cc->EvalMult(x.b, y.b);
        auto ad = cc->EvalMult(x.a, y.b);
        auto bc = cc->EvalMult(x.b, y.a);
        return {cc->EvalAdd(ac, bd), cc->EvalAdd(cc->EvalAdd(ad, bc), bd)};
    };

    PE state = {enc(1.0), enc(0.0)};
    PE mult = {enc(1.01), enc(0.0)};

    cout << "Bare CTxCT chain (no boost):\n";
    for (int i = 0; i <= 50; i += 10) {
        double val = dec(state.a) + dec(state.b) * 1.618;
        double exp = pow(1.01, i);
        cout << i << ": " << val << " (exp " << exp << ") err=" << abs(val-exp)/exp << "\n";
        if (i < 50) for (int m = 0; m < 10; m++) state = ctct(state, mult);
    }
    return 0;
}
