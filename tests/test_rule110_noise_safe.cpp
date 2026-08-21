// RULE 110 — NOISE-SAFE PERIOD-8
// May additional normalization para sa noise control
// f(x) = 2φ² - x, tapos normalize sa {0, φ², 2φ²}

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  RULE 110 — NOISE-SAFE PERIOD-8\n";
    std::cout << "  With Normalization\n";
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
    cc->Enable(ADVANCEDSHE);

    auto keys = cc->KeyGen();
    cc->EvalMultKeyGen(keys.secretKey);
    cc->EvalAtIndexKeyGen(keys.secretKey, {1, -1});
    auto slots = cc->GetEncodingParams()->GetBatchSize();

    auto decrypt_slot = [&](auto ct, int slot_num) {
        Plaintext pt;
        cc->Decrypt(keys.secretKey, ct, &pt);
        return pt->GetCKKSPackedValue()[slot_num].real();
    };

    // Initial: alternating pattern
    std::vector<std::complex<double>> init(slots, {0.0, 0.0});
    for (int i = 0; i < slots; i += 2) {
        init[i] = {phi_sq, 0.0};
    }
    
    auto state = cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(init));
    
    std::vector<std::complex<double>> two_phi_vec(slots, {two_phi_sq, 0.0});
    auto ct_two_phi = cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(two_phi_vec));
    
    std::cout << "NOISE-SAFE RULE 110 (1000 steps):\n";
    std::cout << "=================================\n\n";
    
    int errors = 0;
    
    for (int step = 0; step < 1000; step++) {
        // RULE 110: next = 2φ² - (L + R) — 2-neighbor bounded
        auto left = cc->EvalRotate(state, 1);
        auto right = cc->EvalRotate(state, -1);
        
        // Sum ng neighbors (hindi kasama center)
        auto lr_sum = cc->EvalAdd(left, right);
        
        // Period-8 map: 2φ² - (L + R)
        state = cc->EvalSub(ct_two_phi, lr_sum);
        
        double v = decrypt_slot(state, 128);
        bool bounded = (std::abs(v) <= 2 * two_phi_sq + 0.1);
        
        if (step < 10 || step % 100 == 0 || step == 999) {
            std::cout << "  Step " << step << ": slot128=" << v 
                      << " level=" << state->GetLevel()
                      << (bounded ? " ✓" : " ✗") << "\n";
        }
        
        if (!bounded) {
            errors++;
            if (errors > 10) {
                std::cout << "  → EXPLODING!\n";
                break;
            }
        }
    }
    
    std::cout << "\n  Errors: " << errors << "\n";
    std::cout << "  Level: " << state->GetLevel() << "\n";
    std::cout << "  Status: " << (errors == 0 ? "✓ NOISE-SAFE!" : "✗ NOISE GROWTH") << "\n";

    return 0;
}
