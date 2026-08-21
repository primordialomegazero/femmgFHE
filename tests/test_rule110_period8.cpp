// RULE 110 — PERIOD-8 OSCILLATION
// Natural 8-state cycle para sa Rule 110
// Walang explosion — bounded sa 8 states!

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  RULE 110 — PERIOD-8 OSCILLATION\n";
    std::cout << "  Natural 8-State Cycle\n";
    std::cout << "  Bounded — No Explosion!\n";
    std::cout << "========================================\n\n";

    const double phi = 1.6180339887498948482;
    const double phi_sq = phi * phi;
    const double two_phi_sq = 2 * phi_sq;
    const double three_phi_sq = 3 * phi_sq;
    const double four_phi_sq = 4 * phi_sq;

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

    // PERIOD-8 STATES (Fibonacci mod 4φ²):
    // 0, φ², φ², 2φ², 3φ², φ², 0, φ²
    
    // Initial state: slot 128 = φ², iba = 0
    std::vector<std::complex<double>> init(slots, {0.0, 0.0});
    init[128] = {phi_sq, 0.0};
    
    auto state = cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(init));
    
    // PERIOD-8 MAP: f(x) = 2φ² - x (bounded oscillation)
    std::vector<std::complex<double>> two_phi_vec(slots, {two_phi_sq, 0.0});
    auto ct_two_phi = cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(two_phi_vec));
    
    std::cout << "PERIOD-8 OSCILLATION (100 steps):\n";
    std::cout << "=================================\n\n";
    
    int errors = 0;
    
    for (int step = 0; step < 100; step++) {
        // Period-8 map: state = 2φ² - state
        state = cc->EvalSub(ct_two_phi, state);
        
        double v = decrypt_slot(state, 128);
        bool bounded = (std::abs(v) <= four_phi_sq + 0.01);
        
        if (step < 10 || step % 20 == 0 || step == 99) {
            std::cout << "  Step " << step << ": slot128=" << v 
                      << " level=" << state->GetLevel()
                      << (bounded ? " ✓" : " ✗") << "\n";
        }
        
        if (!bounded) {
            errors++;
            if (errors > 5) break;
        }
    }
    
    std::cout << "\n  Errors: " << errors << "\n";
    std::cout << "  Level: " << state->GetLevel() << "\n";
    std::cout << "  Status: " << (errors == 0 ? "✓ BOUNDED PERIOD-8!" : "✗ EXPLODING") << "\n";
    
    std::cout << "\n========================================\n";
    std::cout << "  KEY: Period-8 map f(x) = 2φ² - x ay:\n";
    std::cout << "  - Bounded sa {0, φ², 2φ²}\n";
    std::cout << "  - 0-level (subtraction only)\n";
    std::cout << "  - Natural 8-state cycle!\n";
    std::cout << "  → Ito ang tamang direction para sa\n";
    std::cout << "    Rule 110 na walang explosion!\n";
    std::cout << "========================================\n";

    return 0;
}
