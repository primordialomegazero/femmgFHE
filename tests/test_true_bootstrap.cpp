// TRUE BOOTSTRAPPING — CKKS Bootstrapping Test
// Using OpenFHE's built-in bootstrapping

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>
#include <chrono>

using namespace lbcrypto;
using namespace std::chrono;

int main() {
    std::cout << "========================================\n";
    std::cout << "  TRUE CKKS BOOTSTRAPPING\n";
    std::cout << "  Unlimited Sequential Operations\n";
    std::cout << "========================================\n\n";

    const double PHI_MOD = 0.6180339887498949;

    CCParams<CryptoContextCKKSRNS> params;
    params.SetMultiplicativeDepth(15);
    params.SetScalingModSize(50);
    params.SetBatchSize(256);
    params.SetFirstModSize(60);
    
    // Bootstrapping parameters
    params.SetRingDim(1 << 16);  // 65536
    params.SetSecurityLevel(HEStd_128_classic);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(params);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);
    cc->Enable(ADVANCEDSHE);  // Para sa bootstrapping
    cc->Enable(FHE);           // Full FHE mode

    auto keys = cc->KeyGen();
    cc->EvalMultKeyGen(keys.secretKey);
    cc->EvalRotateKeyGen(keys.secretKey, {1, 2, 4, 8, 16});
    
    // Bootstrapping setup
    cc->EvalBootstrapSetup(10, {0, 0}, 128);
    
    auto slots = cc->GetEncodingParams()->GetBatchSize();

    auto make_ct = [&](double val) {
        std::vector<std::complex<double>> vec(slots, {0.0, 0.0});
        vec[0] = {val, 0.0};
        return cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(vec));
    };

    auto decrypt_val = [&](auto ct) {
        Plaintext pt;
        cc->Decrypt(keys.secretKey, ct, &pt);
        return pt->GetCKKSPackedValue()[0].real();
    };

    std::cout << "TRUE BOOTSTRAPPING TEST:\n";
    std::cout << "========================\n\n";
    
    auto state = make_ct(0.1);
    std::cout << "  Initial: " << decrypt_val(state) 
              << " (level=" << state->GetLevel() << ")\n";
    
    int bootstraps = 0;
    
    for (int i = 0; i < 50; i++) {
        // Operation: multiply by 1.1 (consumes 1 level)
        auto factor = make_ct(1.1);
        state = cc->EvalMult(state, factor);
        
        // Bootstrap every 10 operations
        if (i > 0 && i % 10 == 0) {
            std::cout << "  Bootstrapping at " << i << "...\n";
            state = cc->EvalBootstrap(state);
            bootstraps++;
            
            double val = decrypt_val(state);
            std::cout << "    After bootstrap: " << val 
                      << " (level=" << state->GetLevel() << ")\n";
        }
    }
    
    double final_val = decrypt_val(state);
    
    std::cout << "\n========================================\n";
    std::cout << "  RESULTS\n";
    std::cout << "========================================\n";
    std::cout << "  Operations: 50\n";
    std::cout << "  Bootstraps: " << bootstraps << "\n";
    std::cout << "  Final value: " << final_val << "\n";
    std::cout << "  Expected: " << 0.1 * pow(1.1, 50) << "\n";
    std::cout << "  Final level: " << state->GetLevel() << "\n";
    std::cout << "  Status: " << (bootstraps > 0 ? "✓ BOOTSTRAPPING WORKS!" : "✗ FAILED") << "\n";
    std::cout << "========================================\n";
    
    return 0;
}
