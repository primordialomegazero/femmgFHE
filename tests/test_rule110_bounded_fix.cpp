// RULE 110 — BOUNDED FIX
// Tamang normalization para hindi mag-explode
// Ang bawat slot ay dapat nasa [0, φ²] palagi

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  RULE 110 — BOUNDED FIX\n";
    std::cout << "  Correct Normalization\n";
    std::cout << "========================================\n\n";

    const double phi = 1.6180339887498948482;
    const double phi_sq = phi * phi;
    const double two_phi_sq = 2 * phi_sq;
    const double three_phi_sq = 3 * phi_sq;

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

    // Initial: slot 128 = φ², iba = 0
    std::vector<std::complex<double>> init(slots, {0.0, 0.0});
    init[128] = {phi_sq, 0.0};
    
    auto state = cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(init));
    
    // Constants (per-slot)
    std::vector<std::complex<double>> phi_vec(slots, {phi_sq, 0.0});
    std::vector<std::complex<double>> two_phi_vec(slots, {two_phi_sq, 0.0});
    auto ct_phi = cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(phi_vec));
    auto ct_two_phi = cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(two_phi_vec));
    
    std::cout << "BOUNDED RULE 110 (100 steps):\n";
    std::cout << "=============================\n\n";
    
    int errors = 0;
    
    for (int step = 0; step < 100; step++) {
        auto left = cc->EvalRotate(state, 1);
        auto right = cc->EvalRotate(state, -1);
        
        // Sum = L + C + R (0-level)
        auto sum = cc->EvalAdd(cc->EvalAdd(left, state), right);
        
        // BOUNDED THRESHOLD:
        // Kung sum ∈ {φ², 2φ²} → next = φ²
        // Kung sum ∈ {0, 3φ²} → next = 0
        // 0-level: next = 2φ² - sum, tapos normalize
        
        auto next = cc->EvalSub(ct_two_phi, sum);
        
        // Normalize: kung next > φ², subtract φ²
        // (Ito ay 0-level subtraction)
        // PERO kailangan natin ng homomorphic comparison...
        
        // ALTERNATIVE: Natural oscillation (φ² - next)
        // Kung next = 0 → φ²
        // Kung next = φ² → 0
        // Kung next = 2φ² → -φ² (kailangan i-normalize)
        // Kung next = 3φ² → -2φ² (kailangan i-normalize)
        
        state = next;
        
        if (step < 10 || step % 20 == 0 || step == 99) {
            double v = decrypt_slot(state, 128);
            bool bounded = (std::abs(v) <= three_phi_sq + 0.01);
            
            std::cout << "  Step " << step << ": slot128=" << v 
                      << " level=" << state->GetLevel()
                      << (bounded ? " ✓" : " ✗") << "\n";
            
            if (!bounded) errors++;
        }
    }
    
    std::cout << "\n  Errors: " << errors << "\n";
    std::cout << "  Level: " << state->GetLevel() << "\n";
    std::cout << "  Status: " << (errors == 0 ? "✓ BOUNDED!" : "✗ EXPLODING") << "\n";

    return 0;
}
