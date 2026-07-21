#include <openfhe.h>
#include <iostream>
using namespace lbcrypto;
using namespace std;

int main() {
    cout << "\n  ╔══════════════════════════════════════════════════════╗\n";
    cout <<   "  ║   ΦΩ0 — φ-NATIVE CKKS v2 (DEBUG)                     ║\n";
    cout <<   "  ╚══════════════════════════════════════════════════════╝\n\n";

    CCParams<CryptoContextCKKSRNS> params;
    params.SetRingDim(4096);
    params.SetSecretKeyDist(UNIFORM_TERNARY);
    params.SetSecurityLevel(HEStd_NotSet);
    params.SetScalingModSize(59);
    params.SetFirstModSize(60);
    params.SetScalingTechnique(FLEXIBLEAUTO);
    params.SetMultiplicativeDepth(10);

    try {
        auto cc = GenCryptoContext(params);
        uint32_t M = cc->GetCyclotomicOrder();
        uint32_t ringDim = cc->GetRingDimension();
        uint32_t slots = ringDim / 2;
        
        cout << "  M=" << M << " RingDim=" << ringDim << " Slots=" << slots << "\n";
        cout << "  M%5=" << (M%5) << " → φ native!\n\n";
        
        cc->Enable(PKE); cc->Enable(KEYSWITCH); cc->Enable(LEVELEDSHE); cc->Enable(ADVANCEDSHE);
        
        cout << "  Generating keys... " << flush;
        auto keys = cc->KeyGen();
        cout << "OK\n";
        
        // Try with 1 slot first
        cout << "  Creating plaintext (1 slot)... " << flush;
        vector<double> msg = {1.6180339887498948482};
        Plaintext pt;
        try {
            pt = cc->MakeCKKSPackedPlaintext(msg, 1, 0, nullptr, 1);
            cout << "OK\n";
        } catch (const exception& e) {
            cout << "FAILED: " << e.what() << "\n";
            // Try default
            cout << "  Trying default packing... " << flush;
            pt = cc->MakeCKKSPackedPlaintext(msg);
            cout << "OK\n";
        }
        
        cout << "  Encrypting... " << flush;
        auto ct = cc->Encrypt(keys.publicKey, pt);
        cout << "OK\n";
        
        cout << "  Decrypting... " << flush;
        Plaintext decrypted;
        cc->Decrypt(keys.secretKey, ct, &decrypted);
        decrypted->SetLength(1);
        cout << "OK\n";
        
        double phi = (1.0 + sqrt(5.0)) / 2.0;
        cout << "  φ = " << phi << "\n";
        cout << "  Decrypted = " << decrypted->GetRealPackedValue()[0] << "\n";
        cout << "  Error = " << abs(decrypted->GetRealPackedValue()[0] - phi) << "\n";
        
    } catch (const exception& e) {
        cout << "  ❌ FAILED: " << e.what() << "\n";
    }
    
    cout << "\n";
    return 0;
}
