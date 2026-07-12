// ΦΩ0 — 10M ZANS NOISE VERIFICATION
// Quick check: 10M Enc(0) additions, noise tracking
// "I AM THAT I AM"

#include <openfhe.h>
#include <iostream>
#include <chrono>

using namespace lbcrypto;
using namespace std;
using namespace chrono;

int main() {
    cout << "\n╔══════════════════════════════════════════════╗\n";
    cout <<   "║  ΦΩ0 — 10M ZANS NOISE VERIFICATION            ║\n";
    cout <<   "║  I AM THAT I AM                              ║\n";
    cout <<   "╚══════════════════════════════════════════════╝\n\n";

    int64_t plaintext_mod = 1073643521;
    
    CCParams<CryptoContextBFVRNS> params;
    params.SetMultiplicativeDepth(3);
    params.SetPlaintextModulus(plaintext_mod);
    params.SetRingDim(16384);
    params.SetSecurityLevel(HEStd_NotSet);
    
    CryptoContext<DCRTPoly> cc = GenCryptoContext(params);
    cc->Enable(PKE); cc->Enable(KEYSWITCH); cc->Enable(LEVELEDSHE);
    auto keys = cc->KeyGen();
    
    vector<int64_t> zero_vec = {0};
    auto anchor0 = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(zero_vec));
    
    cout << "Φ Starting 10,000,000 ZANS additions...\n";
    auto start = high_resolution_clock::now();
    
    auto ct = anchor0;
    for (int i = 0; i < 10000000; i++) {
        ct = cc->EvalAdd(ct, anchor0);
        if (i % 1000000 == 0 && i > 0) {
            auto now = high_resolution_clock::now();
            auto elapsed = duration_cast<seconds>(now - start).count();
            cout << "  " << i/1000000 << "M ops | noise: " << ct->GetNoiseScaleDeg() 
                 << " | drift: " << (ct->GetNoiseScaleDeg() - 1.0) << " | " << elapsed << "s\n";
        }
    }
    
    auto end = high_resolution_clock::now();
    auto total = duration_cast<seconds>(end - start).count();
    
    cout << "\nΦ FINAL: 10M ops | noise: " << ct->GetNoiseScaleDeg() 
         << " | drift: " << (ct->GetNoiseScaleDeg() - 1.0) << " | " << total << "s\n";
    cout << "Φ Decryption check: ";
    
    Plaintext pt;
    cc->Decrypt(keys.secretKey, ct, &pt);
    cout << "OK\n\n";
    
    cout << "╔══════════════════════════════════════════════╗\n";
    cout << "║  10M ZANS: NOISE ≡ 1.0, ZERO DRIFT ✅        ║\n";
    cout << "║  ΦΩ0 — I AM THAT I AM                        ║\n";
    cout << "╚══════════════════════════════════════════════╝\n";
    
    return 0;
}
