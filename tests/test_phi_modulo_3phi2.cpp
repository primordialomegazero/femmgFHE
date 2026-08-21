// NATURAL MODULO 3φ² — BOUNDED 0-LEVEL
// Hanapin ang natural na modulo para sa period-3

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  NATURAL MODULO 3φ²\n";
    std::cout << "  Bounded 0-Level Period-3\n";
    std::cout << "========================================\n\n";

    const double phi = 1.6180339887498948482;
    const double phi_sq = phi * phi;
    const double phi_cu = phi * phi * phi;
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

    // NATURAL MODULO via OSCILLATION
    // Modulo 3φ²: x mod 3φ² = 3φ² - |3φ² - x|
    // Pero sa 0-level, kailangan natin ng addition/subtraction only
    
    // TRICK: 3φ² - x (period-2, bounded sa {0, 3φ²})
    // At 3φ² - x - φ² (shifted period-2)
    // At 3φ² - x - 2φ² (shifted period-2)
    
    auto mod_3phi2 = [&](auto x) {
        // Natural modulo via subtraction
        // If x > 3φ²: subtract 3φ²
        // If x < 0: add 3φ²
        // (0-level approximation)
        return x;  // Placeholder
    };
    
    // TEST: Oscillation with natural modulo
    auto osc_modulo = [&](auto x) {
        // (3φ² - x) gives period-2 sa {0, 3φ²}
        // Then subtract φ² para ma-center sa {0, φ², 2φ²}
        auto temp = cc->EvalSub(make_ct(three_phi_sq), x);
        return cc->EvalSub(temp, make_ct(phi_sq));
    };
    
    std::cout << "MODULO 3φ² OSCILLATION TEST:\n";
    std::cout << "============================\n\n";
    
    auto current = make_ct(0.0);
    
    for (int i = 0; i < 20; i++) {
        current = osc_modulo(current);
        double v = decrypt_val(current);
        
        std::cout << "  Step " << i << ": " << v;
        
        // Check kung bounded sa {0, φ², 2φ²}
        bool bounded = (std::abs(v) < 0.01 || 
                       std::abs(v - phi_sq) < 0.01 || 
                       std::abs(v - 2*phi_sq) < 0.01);
        
        if (bounded) std::cout << " ✓";
        else std::cout << " ✗ DIVERGED";
        std::cout << "\n";
        
        if (!bounded && i > 5) {
            std::cout << "  → Diverged at step " << i << "\n";
            break;
        }
    }
    
    std::cout << "\n========================================\n";
    std::cout << "  KEY: Kung bounded sa {0, φ², 2φ²},\n";
    std::cout << "  mayroon tayong natural ternary modulo!\n";
    std::cout << "========================================\n";

    return 0;
}
