// TRACE ERASURE VIA MULTI-SLOT INTERFERENCE
// Ang interference sa pagitan ng slots ay nagbe-blur
// ng phase difference

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  TRACE ERASURE VIA INTERFERENCE\n";
    std::cout << "  Multi-Slot Phase Blurring\n";
    std::cout << "========================================\n\n";

    const double phi = 1.6180339887498948482;
    const double phi_sq = phi * phi;
    const double two_phi_sq = 2 * phi_sq;

    CCParams<CryptoContextCKKSRNS> params;
    params.SetMultiplicativeDepth(5);
    params.SetScalingModSize(50);
    params.SetBatchSize(256);
    params.SetFirstModSize(60);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(params);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);

    auto keys = cc->KeyGen();
    cc->EvalMultKeyGen(keys.secretKey);
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

    // Gumamit ng slot 0 at slot 1 bilang magkaibang inputs
    // Pagkatapos ay i-interfere ang mga ito
    
    std::vector<std::complex<double>> vec(slots, {0.0, 0.0});
    vec[0] = {0.0, 0.0};        // Input A sa slot 0
    vec[1] = {phi_sq, 0.0};     // Input B sa slot 1
    
    auto state = cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(vec));

    std::cout << "INTERFERENCE-BASED ERASURE:\n";
    std::cout << "===========================\n\n";

    // I-rotate at i-add para sa interference
    for (int step = 0; step < 20; step++) {
        // Paikutin ang slots para sa interference
        state = cc->EvalRotate(state, 1);
        
        Plaintext pt;
        cc->Decrypt(keys.secretKey, state, &pt);
        auto vals = pt->GetCKKSPackedValue();
        
        double v0 = vals[0].real();
        double v1 = vals[1].real();
        
        if (step < 10) {
            std::cout << "  Step " << step << ": slot0=" << v0 
                      << " slot1=" << v1
                      << " diff=" << std::abs(v0 - v1) << "\n";
        }
    }

    return 0;
}
