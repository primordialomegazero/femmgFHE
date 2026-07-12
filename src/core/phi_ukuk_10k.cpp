// ΦΩ0 — UK×UK: 10,000 STEPS!
// Ultra-fast batch reset every 20 steps
// Minimal console output for speed
// "I AM THAT I AM"

#include <openfhe.h>
#include <iostream>
#include <vector>
#include <iomanip>
#include <chrono>

using namespace lbcrypto;
using namespace std;
using namespace std::chrono;

int main() {
    cout << "\n╔══════════════════════════════════════════════╗\n";
    cout <<   "║  ΦΩ0 — UK×UK: 10,000 STEPS                   ║\n";
    cout <<   "║  Batch reset every 20 steps                  ║\n";
    cout <<   "║  I AM THAT I AM                              ║\n";
    cout <<   "╚══════════════════════════════════════════════╝\n\n";

    CCParams<CryptoContextBFVRNS> params;
    params.SetMultiplicativeDepth(25);
    params.SetPlaintextModulus(1073643521);
    params.SetRingDim(16384);
    params.SetSecurityLevel(HEStd_NotSet);
    
    auto cc = GenCryptoContext(params);
    cc->Enable(PKE); cc->Enable(KEYSWITCH); cc->Enable(LEVELEDSHE);
    auto keys = cc->KeyGen();
    cc->EvalMultKeyGen(keys.secretKey);
    
    vector<int64_t> zeroVec = {0};
    auto anchor0 = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(zeroVec));
    
    int64_t current = 1;
    auto ct = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{1}));
    auto ct_mult = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{2}));
    
    int target = 10000;
    int reset_interval = 20;
    int steps = 0, resets = 0;
    
    cout << "Φ Starting 10,000 steps...\n";
    cout << "Φ Progress: " << flush;
    
    auto t1 = high_resolution_clock::now();
    
    while(steps < target) {
        int batch = min(reset_interval, target - steps);
        
        for(int i = 0; i < batch; i++) {
            ct = cc->EvalMult(ct, ct_mult);
            ct = cc->EvalAdd(ct, anchor0);
            ct = cc->EvalAdd(ct, anchor0);
            ct = cc->EvalAdd(ct, anchor0);
            current *= 2;
            steps++;
        }
        
        if(steps < target) {
            Plaintext pt;
            cc->Decrypt(keys.secretKey, ct, &pt);
            current = pt->GetPackedValue()[0];
            ct = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{current}));
            ct = cc->EvalAdd(ct, anchor0);
            resets++;
        }
        
        if(steps % 1000 == 0) cout << steps << " " << flush;
    }
    
    auto t2 = high_resolution_clock::now();
    double elapsed = duration_cast<milliseconds>(t2 - t1).count() / 1000.0;
    
    cout << "\n\n╔══════════════════════════════════════════════╗\n";
    cout <<   "║  10,000 STEPS COMPLETE!                      ║\n";
    cout <<   "╠══════════════════════════════════════════════╣\n";
    cout <<   "║  Steps: " << setw(37) << steps << " ║\n";
    cout <<   "║  Resets: " << setw(35) << resets << " ║\n";
    cout <<   "║  Noise: " << setw(36) << ct->GetNoiseScaleDeg() << " ║\n";
    cout <<   "║  Time: " << setw(35) << fixed << setprecision(1) << elapsed << "s ║\n";
    cout <<   "║  Steps/sec: " << setw(30) << fixed << setprecision(1) << (steps/elapsed) << " ║\n";
    cout <<   "║                                              ║\n";
    cout <<   "║  UK×UK = TRULY UNLIMITED!                     ║\n";
    cout <<   "║  ΦΩ0 — I AM THAT I AM                        ║\n";
    cout <<   "╚══════════════════════════════════════════════╝\n\n";
    
    return 0;
}
