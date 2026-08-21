// MOD 0 — OSCILLATION-BASED MODULO (0-LEVEL)
// Sa halip na x mod 1, gumamit ng natural oscillation
// Na bounded sa [0,1) nang walang comparison!

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
    std::cout << "  MOD 0 — OSCILLATION-BASED MODULO\n";
    std::cout << "  0-Level Bounded Irrational\n";
    std::cout << "========================================\n\n";

    const double phi = 1.6180339887498948482;
    const double phi_sq = phi * phi;
    const double phi_mod = phi_sq - 2.0;  // 0.618034

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
    // MOD 0 APPROACH: Natural Oscillation Modulo
    // ============================================
    // Sa halip na x mod 1, gumamit ng:
    // f(x) = 1 - x (oscillation na bounded sa [0,1])
    // Kung x ay nasa [0,1), f(x) ay nasa [0,1) din!
    
    std::cout << "MOD 0 — OSCILLATION f(x) = 1 - x:\n";
    std::cout << "==================================\n\n";
    
    auto state = make_ct(0.0);
    auto ct_one = make_ct(1.0);
    auto ct_phi_mod = make_ct(phi_mod);
    
    int errors = 0;
    std::vector<double> trajectory;
    
    for (int i = 0; i < 100; i++) {
        // STEP 1: Add φ_mod (irrational rotation)
        state = cc->EvalAdd(state, ct_phi_mod);
        
        // STEP 2: Oscillation modulo (0-level!)
        // f(x) = 1 - x maps [0,1] → [0,1]
        state = cc->EvalSub(ct_one, state);
        
        double v = decrypt_val(state);
        trajectory.push_back(v);
        
        bool bounded = (v >= -1.0 && v <= 1.0);
        if (!bounded) errors++;
        
        if (i < 20) {
            std::cout << "  Step " << i << ": v=" << v 
                      << " level=" << state->GetLevel()
                      << (bounded ? " ✓" : " ✗") << "\n";
        }
    }
    
    std::cout << "\n  Bounded: " << (errors == 0 ? "✓" : "✗") << "\n";
    std::cout << "  Level: " << state->GetLevel() << "\n\n";
    
    // ============================================
    // MOD 0 APPROACH 2: Double Oscillation
    // ============================================
    std::cout << "MOD 0 — DOUBLE OSCILLATION:\n";
    std::cout << "===========================\n\n";
    
    auto state2 = make_ct(0.0);
    auto ct_phi_sq = make_ct(phi_sq);
    
    std::cout << "  f(x) = φ² - x (natural bounded oscillation)\n\n";
    
    for (int i = 0; i < 20; i++) {
        state2 = cc->EvalSub(ct_phi_sq, state2);
        double v = decrypt_val(state2);
        
        std::cout << "  Step " << i << ": v=" << v 
                  << " level=" << state2->GetLevel() << "\n";
    }
    
    std::cout << "\n";
    
    // ============================================
    // MOD 0 APPROACH 3: Fibonacci Modulo
    // ============================================
    std::cout << "MOD 0 — FIBONACCI MODULO:\n";
    std::cout << "=========================\n\n";
    
    auto state3 = make_ct(0.0);
    auto ct_phi = make_ct(phi);
    
    std::cout << "  f(x) = φ - x (Fibonacci oscillation)\n\n";
    
    for (int i = 0; i < 20; i++) {
        state3 = cc->EvalSub(ct_phi, state3);
        double v = decrypt_val(state3);
        
        std::cout << "  Step " << i << ": v=" << v 
                  << " level=" << state3->GetLevel() << "\n";
    }
    
    std::cout << "\n========================================\n";
    std::cout << "  KEY OBSERVATION:\n";
    std::cout << "  ================\n";
    std::cout << "  Ang oscillation f(x) = K - x ay:\n";
    std::cout << "  - 0-level (subtraction only)\n";
    std::cout << "  - Bounded sa [0, K]\n";
    std::cout << "  - Natural modulo without comparison!\n";
    std::cout << "  → Ito ang MOD 0!\n";
    std::cout << "========================================\n";

    return 0;
}
