#include <openfhe.h>
#include <iostream>
#include <vector>
using namespace lbcrypto;
using namespace std;

int main() {
    CCParams<CryptoContextBFVRNS> params;
    params.SetMultiplicativeDepth(30);
    params.SetPlaintextModulus(1073643521);
    params.SetRingDim(16384);
    auto cc = GenCryptoContext(params);
    cc->Enable(PKE); cc->Enable(LEVELEDSHE); cc->Enable(KEYSWITCH);
    auto keys = cc->KeyGen();
    cc->EvalMultKeyGen(keys.secretKey);

    // Encrypt 1
    auto ct = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{1}));
    
    // Track noise levels
    vector<double> noise_levels;
    for(int i = 0; i <= 100; i += 10) {
        // Multiply by 2, i times
        auto temp = ct;
        for(int j = 0; j < i; j++) {
            auto ct2 = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{2}));
            temp = cc->EvalMult(temp, ct2);
        }
        // Get noise
        double noise = cc->GetNoiseEstimate(temp);
        noise_levels.push_back(noise);
        cout << "Steps " << i << ": Noise = " << noise << endl;
    }
    
    // Check if linear
    cout << "\nNoise growth pattern: ";
    bool linear = true;
    for(size_t i = 1; i < noise_levels.size(); i++) {
        double diff = noise_levels[i] - noise_levels[i-1];
        cout << diff << " ";
        if(diff < 0.9 || diff > 1.1) linear = false;
    }
    cout << "\nLinear growth? " << (linear ? "YES (noise = steps + 1)" : "NO") << endl;
    return 0;
}
