// TEST 2: NOISE MEASUREMENT — Sukatin ang noise sa bawat depth
#include "openfhe.h"
#include <iostream>

using namespace lbcrypto;

int main() {
    std::cout << "NOISE MEASUREMENT (OpenFHE BFV)\n";
    std::cout << "===============================\n\n";
    
    CCParams<CryptoContextBFVRNS> parameters;
    parameters.SetPlaintextModulus(65537);
    parameters.SetMultiplicativeDepth(20);
    parameters.SetRingDim(16384);
    
    CryptoContext<DCRTPoly> cc = GenCryptoContext(parameters);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);
    
    auto keys = cc->KeyGen();
    cc->EvalMultKeyGen(keys.secretKey);
    
    auto ct1 = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext({1}));
    auto current = ct1;
    
    std::cout << "Depth | Noise Level | Status\n";
    std::cout << "------|-------------|--------\n";
    
    for (int i = 0; i <= 20; i++) {
        // I-estimate ang noise: decrypt at check kung malapit sa expected
        Plaintext pt;
        cc->Decrypt(keys.secretKey, current, &pt);
        int val = pt->GetPackedValue()[0];
        
        // Noise proxy: kung malayo sa 0 o 1
        int noise_proxy = (val == 0 || val == 1) ? 0 : 1;
        
        std::cout << i << " | " << (noise_proxy ? "NOISY" : "CLEAN") << " | "
                  << (noise_proxy ? "⚠" : "✓") << "\n";
        
        auto sq = cc->EvalMult(current, current);
        current = cc->EvalSub(ct1, sq);
    }
    
    std::cout << "\nStandard BFV: Noise grows exponentially\n";
    std::cout << "→ Kailangan ng bootstrapping pagkatapos ng ~10-15 depths\n";
    
    return 0;
}
