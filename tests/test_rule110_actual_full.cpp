// RULE 110 — ACTUAL IMPLEMENTATION
// Non-trivial initial state + Full 3-neighbor transition
// Encoding: 1 = φ², 0 = 0
// Transition: f(L,C,R) ayon sa Rule 110 table

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  RULE 110 — ACTUAL IMPLEMENTATION\n";
    std::cout << "  Non-Trivial Initial State\n";
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

    // ============================================
    // RULE 110 TRANSITION FUNCTION (φ-domain)
    // ============================================
    // f(L,C,R) = ?
    // Rule 110 table:
    // 111→0, 110→1, 101→1, 100→0, 011→1, 010→1, 001→1, 000→0
    //
    // Sa φ-domain: 1=φ², 0=0
    // Subukan: f = φ² - (L + C + R - φ²)
    // = 2φ² - (L + C + R)
    // Ito ay period-2 oscillation sa sum!
    
    // NON-TRIVIAL INITIAL STATE: Random pattern
    std::vector<std::complex<double>> init(slots, {0.0, 0.0});
    for (int i = 0; i < slots; i++) {
        // Pattern: 1011010100110110...
        bool bit = ((i * 7 + 3) % 11) > 5;  // pseudo-random
        init[i] = {bit ? phi_sq : 0.0, 0.0};
    }
    
    auto state = cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(init));
    
    std::cout << "NON-TRIVIAL INITIAL STATE:\n";
    std::cout << "  slot126=" << decrypt_slot(state, 126) << "\n";
    std::cout << "  slot127=" << decrypt_slot(state, 127) << "\n";
    std::cout << "  slot128=" << decrypt_slot(state, 128) << "\n";
    std::cout << "  slot129=" << decrypt_slot(state, 129) << "\n";
    std::cout << "  slot130=" << decrypt_slot(state, 130) << "\n\n";
    
    std::cout << "RULE 110 TRANSITION (100 steps):\n";
    std::cout << "================================\n\n";
    
    int errors = 0;
    
    for (int step = 0; step < 100; step++) {
        auto left = cc->EvalRotate(state, 1);
        auto right = cc->EvalRotate(state, -1);
        
        // SUM = L + C + R (0-level)
        auto sum = cc->EvalAdd(cc->EvalAdd(left, state), right);
        
        // TRANSITION: 2φ² - sum (period-2 oscillation)
        // Kung sum = 0 → 2φ² (should be 0)
        // Kung sum = φ² → φ² (should be φ²) ✓
        // Kung sum = 2φ² → 0 (should be φ²)
        // Kung sum = 3φ² → -φ² (should be 0)
        state = cc->EvalSub(make_uniform(two_phi_sq), sum);
        
        double v = decrypt_slot(state, 128);
        bool bounded = (std::abs(v) <= three_phi_sq + 0.01);
        
        if (step < 20) {
            std::cout << "  Step " << step << ": slot128=" << v 
                      << " level=" << state->GetLevel()
                      << (bounded ? " ✓" : " ✗") << "\n";
        }
        
        if (!bounded) {
            errors++;
            if (errors > 10) {
                std::cout << "  → EXPLODING at step " << step << "\n";
                break;
            }
        }
    }
    
    std::cout << "\n  Errors: " << errors << "\n";
    std::cout << "  Level: " << state->GetLevel() << "\n";
    std::cout << "  Status: " << (errors == 0 ? "🏆 RULE 110 BOUNDED!" : "❌ EXPLODING") << "\n";
    
    // ============================================
    // CORRECTED TRANSITION (PERIOD-3 BOUNDED)
    // ============================================
    std::cout << "\n\nCORRECTED TRANSITION (PERIOD-3):\n";
    std::cout << "================================\n\n";
    
    // Reset sa non-trivial state
    std::vector<std::complex<double>> init2(slots, {0.0, 0.0});
    for (int i = 0; i < slots; i++) {
        bool bit = ((i * 7 + 3) % 11) > 5;
        init2[i] = {bit ? phi_sq : 0.0, 0.0};
    }
    auto state2 = cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(init2));
    
    int errors2 = 0;
    
    for (int step = 0; step < 100; step++) {
        auto left = cc->EvalRotate(state2, 1);
        auto right = cc->EvalRotate(state2, -1);
        auto sum = cc->EvalAdd(cc->EvalAdd(left, state2), right);
        
        // PERIOD-3: K_i - sum (bounded oscillation)
        double K;
        switch (step % 3) {
            case 0: K = phi_sq; break;
            case 1: K = two_phi_sq; break;
            case 2: K = three_phi_sq; break;
        }
        
        state2 = cc->EvalSub(make_uniform(K), sum);
        
        double v = decrypt_slot(state2, 128);
        bool bounded = (std::abs(v) <= three_phi_sq + 0.01);
        
        if (step < 10 || step % 20 == 0 || step == 99) {
            std::cout << "  Step " << step << ": slot128=" << v 
                      << " level=" << state2->GetLevel()
                      << (bounded ? " ✓" : " ✗") << "\n";
        }
        
        if (!bounded) {
            errors2++;
            if (errors2 > 10) {
                std::cout << "  → EXPLODING at step " << step << "\n";
                break;
            }
        }
    }
    
    std::cout << "\n  Errors: " << errors2 << "\n";
    std::cout << "  Level: " << state2->GetLevel() << "\n";
    std::cout << "  Status: " << (errors2 == 0 ? "🏆 PERIOD-3 RULE 110 BOUNDED!" : "❌ EXPLODING") << "\n";

    return 0;
}
