// RULE 110 — ROTATION + INTERFERENCE
// Pure rotation (bounded) + φ-interference (complex)
// Walang sum — kaya walang explosion!

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  RULE 110 — ROTATION + INTERFERENCE\n";
    std::cout << "  Bounded + Complex Dynamics\n";
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
    cc->EvalAtIndexKeyGen(keys.secretKey, {1, -1, 2, -2});
    auto slots = cc->GetEncodingParams()->GetBatchSize();

    auto decrypt_slot = [&](auto ct, int slot_num) {
        Plaintext pt;
        cc->Decrypt(keys.secretKey, ct, &pt);
        return pt->GetCKKSPackedValue()[slot_num].real();
    };

    auto make_uniform = [&](double val) {
        std::vector<std::complex<double>> vec(slots, {0.0, 0.0});
        for (int i = 0; i < slots; i++) vec[i] = {val, 0.0};
        return cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(vec));
    };

    // Initial: alternating pattern
    std::vector<std::complex<double>> init(slots, {0.0, 0.0});
    for (int i = 0; i < slots; i += 2) init[i] = {phi_sq, 0.0};
    auto state = cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(init));
    
    // ROTATION + OSCILLATION (walang sum!):
    // next = φ² - rotate(state) — bounded!
    // Kasi rotate lang, walang accumulation
    
    std::cout << "ROTATION-OSCILLATION (1000 steps):\n";
    std::cout << "==================================\n\n";
    
    int errors = 0;
    
    for (int step = 0; step < 1000; step++) {
        // Rotate left (phase shift)
        auto rotated = cc->EvalRotate(state, 1);
        
        // Oscillate: φ² - rotated (bounded!)
        state = cc->EvalSub(make_uniform(phi_sq), rotated);
        
        double v = decrypt_slot(state, 128);
        bool bounded = (std::abs(v) <= phi_sq + 0.01);
        
        if (step < 10 || step % 100 == 0 || step == 999) {
            std::cout << "  Step " << step << ": slot128=" << v 
                      << " level=" << state->GetLevel()
                      << (bounded ? " ✓" : " ✗") << "\n";
        }
        
        if (!bounded) {
            errors++;
            if (errors > 5) {
                std::cout << "  → EXPLODING!\n";
                break;
            }
        }
    }
    
    std::cout << "\n  Errors: " << errors << "\n";
    std::cout << "  Level: " << state->GetLevel() << "\n";
    std::cout << "  Status: " << (errors == 0 ? "🏆 ROTATION-OSCILLATION BOUNDED!" : "❌ UNBOUNDED") << "\n";

    return 0;
}
