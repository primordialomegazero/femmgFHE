#include <openfhe.h>
#include <iostream>
using namespace lbcrypto;
using namespace std;

int main() {
    cout << "\n  ╔══════════════════════════════════════════════════════╗\n";
    cout <<   "  ║   ΦΩ0 — φ-NATIVE CKKS: M=40, RingDim=16              ║\n";
    cout <<   "  ╚══════════════════════════════════════════════════════╝\n\n";

    CCParams<CryptoContextCKKSRNS> params;
    params.SetRingDim(16);  // → M=40 (modified)
    params.SetSecretKeyDist(UNIFORM_TERNARY);
    params.SetSecurityLevel(HEStd_NotSet);
    params.SetScalingModSize(59);
    params.SetFirstModSize(60);
    params.SetScalingTechnique(FLEXIBLEAUTO);
    params.SetMultiplicativeDepth(5);

    try {
        auto cc = GenCryptoContext(params);
        cout << "  M=" << cc->GetCyclotomicOrder() 
             << " RingDim=" << cc->GetRingDimension()
             << " M%5=" << (cc->GetCyclotomicOrder() % 5) << "\n\n";
        
        cc->Enable(PKE); cc->Enable(KEYSWITCH); cc->Enable(LEVELEDSHE);
        cout << "  KeyGen... " << flush;
        auto keys = cc->KeyGen();
        cout << "OK\n";
        
        cout << "  Encrypt φ... " << flush;
        vector<double> msg = {(1.0+sqrt(5.0))/2.0};
        auto pt = cc->MakeCKKSPackedPlaintext(msg, 1, 0, nullptr, 8);
        auto ct = cc->Encrypt(keys.publicKey, pt);
        cout << "OK\n";
        
        cout << "  Decrypt... " << flush;
        Plaintext dec;
        cc->Decrypt(keys.secretKey, ct, &dec);
        dec->SetLength(1);
        cout << "OK\n";
        
        cout << "  Value: " << dec->GetRealPackedValue()[0] 
             << " (expected " << (1.0+sqrt(5.0))/2.0 << ")\n";
        
    } catch (const exception& e) {
        cout << "  ❌ " << e.what() << "\n";
    }
    return 0;
}
