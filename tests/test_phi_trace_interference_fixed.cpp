// TRACE ERASURE VIA INTERFERENCE — FIXED
// May rotation keys na

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

    CCParams<CryptoContextCKKSRNS> params;
    params.SetMultiplicativeDepth(5);
    params.SetScalingModSize(50);
    params.SetBatchSize(256);
    params.SetFirstModSize(60);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(params);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);
    cc->Enable(ADVANCEDSHE);

    auto keys = cc->KeyGen();
    cc->EvalMultKeyGen(keys.secretKey);
    
    // Generate rotation keys
    std::cout << "Generating rotation keys...\n";
    cc->EvalAtIndexKeyGen(keys.secretKey, {1, -1, 2, -2, 4, -4, 8, -8});
    std::cout << "Rotation keys generated!\n\n";

    auto slots = cc->GetEncodingParams()->GetBatchSize();

    // Input A sa slot 0, Input B sa slot 1
    std::vector<std::complex<double>> vec(slots, {0.0, 0.0});
    vec[0] = {0.0, 0.0};        // Input A
    vec[1] = {phi_sq, 0.0};     // Input B

    auto state = cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(vec));

    auto decrypt_slot = [&](auto ct, int slot_num) {
        Plaintext pt;
        cc->Decrypt(keys.secretKey, ct, &pt);
        return pt->GetCKKSPackedValue()[slot_num].real();
    };

    std::cout << "INTERFERENCE TEST (20 steps):\n";
    std::cout << "=============================\n\n";

    for (int step = 0; step < 20; step++) {
        state = cc->EvalAtIndex(state, 1);
        
        double v0 = decrypt_slot(state, 0);
        double v1 = decrypt_slot(state, 1);
        double diff = std::abs(v0 - v1);
        
        if (step < 10) {
            std::cout << "  Step " << step << ": slot0=" << v0
                      << " slot1=" << v1
                      << " diff=" << diff << "\n";
        }
    }

    std::cout << "\n  Level: " << state->GetLevel() << "\n";
    std::cout << "  Status: Interference complete\n";

    return 0;
}
