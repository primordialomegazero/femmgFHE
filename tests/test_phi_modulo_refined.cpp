// NATURAL MODULO REFINED — TAMANG CYCLING
// Ayusin ang modulo para sa {0, φ², 2φ², 3φ²} cycle

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  NATURAL MODULO REFINED\n";
    std::cout << "  Correct 4-State Cycling\n";
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

    // ============================================
    // NATURAL MODULO STRATEGIES
    // ============================================
    
    std::cout << "MODULO STRATEGIES:\n";
    std::cout << "==================\n\n";
    
    // Strategy 1: Period-4 oscillation
    // 0 → φ² → 2φ² → -φ² → 0
    auto modulo_period4 = [&](auto x) {
        // Subtract 4φ² kung masyadong malaki
        // Add 4φ² kung negative
        // Ito ay 0-level approximation
        return x;  // Placeholder - kailangan ng refinement
    };
    
    // Strategy 2: Two-stage oscillation
    // Stage 1: Modulo 2φ²
    // Stage 2: Modulo φ²
    auto modulo_2stage = [&](auto x) {
        // First: 2φ² - x (period-2 sa {0, 2φ²})
        auto stage1 = cc->EvalSub(make_ct(two_phi_sq), x);
        // Second: φ² - stage1 (period-2 sa {0, φ²})
        auto stage2 = cc->EvalSub(make_ct(phi_sq), stage1);
        return stage2;
    };
    
    // Strategy 3: Alternating offset
    // Use φ² - x para sa even steps
    // Use 2φ² - x para sa odd steps
    auto modulo_alternating = [&](auto x, int step) {
        if (step % 2 == 0) {
            return cc->EvalSub(make_ct(phi_sq), x);
        } else {
            return cc->EvalSub(make_ct(two_phi_sq), x);
        }
    };
    
    // Test strategies
    std::cout << "TESTING MODULO STRATEGIES:\n";
    std::cout << "===========================\n\n";
    
    auto test_modulo = [&](const char* name, auto fn, int steps = 8) {
        std::cout << name << ":\n  ";
        auto current = make_ct(0.0);
        
        for (int i = 0; i < steps; i++) {
            current = fn(current, i);
            double v = decrypt_val(current);
            std::cout << v << " ";
        }
        std::cout << "\n\n";
    };
    
    // Test 2-stage modulo
    std::cout << "2-Stage Modulo (φ² at 2φ²):\n  ";
    auto current_2stage = make_ct(0.0);
    for (int i = 0; i < 8; i++) {
        current_2stage = modulo_2stage(current_2stage);
        std::cout << decrypt_val(current_2stage) << " ";
    }
    std::cout << "\n\n";
    
    // Test alternating modulo
    std::cout << "Alternating Modulo:\n  ";
    auto current_alt = make_ct(0.0);
    for (int i = 0; i < 8; i++) {
        current_alt = modulo_alternating(current_alt, i);
        std::cout << decrypt_val(current_alt) << " ";
    }
    std::cout << "\n\n";
    
    // ============================================
    // REFINED COUNTER WITH CORRECT MODULO
    // ============================================
    std::cout << "REFINED COUNTER (0-15):\n";
    std::cout << "=======================\n\n";
    
    auto counter = make_ct(0.0);
    auto ct_phi_sq = make_ct(phi_sq);
    auto ct_four_phi_sq = make_ct(four_phi_sq);
    
    std::cout << "Using period-4 natural cycle:\n";
    for (int i = 0; i < 16; i++) {
        // Increment
        counter = cc->EvalAdd(counter, ct_phi_sq);
        
        // Natural modulo via period-4
        // If counter > 3φ²: subtract 4φ²
        // (Ito ay 0-level approximation)
        double current_val = decrypt_val(counter);
        
        if (current_val > three_phi_sq) {
            counter = cc->EvalSub(counter, ct_four_phi_sq);
        }
        
        double v = decrypt_val(counter);
        std::cout << "  Count " << i << ": " << v;
        
        if (std::abs(v) < 0.01) std::cout << " [0]";
        else if (std::abs(v - phi_sq) < 0.01) std::cout << " [1]";
        else if (std::abs(v - two_phi_sq) < 0.01) std::cout << " [2]";
        else if (std::abs(v - three_phi_sq) < 0.01) std::cout << " [3]";
        else std::cout << " [OTHER]";
        
        std::cout << " level=" << counter->GetLevel() << "\n";
    }
    
    std::cout << "\n========================================\n";
    std::cout << "  KEY INSIGHT:\n";
    std::cout << "  Ang natural modulo ay kailangan ng\n";
    std::cout << "  conditional subtraction (0-level)\n";
    std::cout << "  para sa tamang cycling\n";
    std::cout << "========================================\n";

    return 0;
}
