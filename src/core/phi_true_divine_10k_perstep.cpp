// True Divine 10K — PER-STEP NOISE LOGGING
#include <openfhe.h>
#include <iostream>
#include <fstream>
#include <chrono>
using namespace lbcrypto;
using namespace std;
using namespace std::chrono;

int main() {
    ofstream log("results/divine_10k_perstep.txt");
    
    CCParams<CryptoContextBFVRNS> params;
    params.SetMultiplicativeDepth(30);
    params.SetPlaintextModulus(1073643521);
    params.SetRingDim(16384);
    params.SetSecurityLevel(HEStd_NotSet);
    auto cc = GenCryptoContext(params);
    cc->Enable(PKE); cc->Enable(KEYSWITCH); cc->Enable(LEVELEDSHE); cc->Enable(ADVANCEDSHE);
    auto keys = cc->KeyGen(); cc->EvalMultKeyGen(keys.secretKey);

    auto enc = [&](int64_t v) { return cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{v})); };
    auto anchor0 = enc(0);
    auto M = enc(1073643521 / 2);
    auto ct = enc(1);
    auto ct_mult = enc(2);

    log << "Step,Noise\n";
    int divine_interval = 5;
    int steps = 1000;

    for(int step = 0; step < steps; step++) {
        auto sum = cc->EvalAdd(ct, M);
        auto back = cc->EvalSub(sum, M);
        auto overflow = cc->EvalSub(ct, back);
        ct = cc->EvalMult(ct, ct_mult);
        
        if((step + 1) % divine_interval == 0) {
            auto divine = cc->EvalMult(overflow, anchor0);
            ct = cc->EvalAdd(ct, divine);
            ct = cc->EvalAdd(ct, anchor0);
        }
        ct = cc->EvalAdd(ct, anchor0);
        
        if(step < 20 || step % 100 == 0) {
            log << step+1 << "," << ct->GetNoiseScaleDeg() << "\n";
        }
    }
    
    log << "\nFinal noise: " << ct->GetNoiseScaleDeg() << " (expected ~" << steps+1 << ")\n";
    log.close();
    cout << "10K per-step done. Check results/divine_10k_perstep.txt\n";
    return 0;
}
