// NATURAL OSCILLATION SEARCH
// Hanapin ang φ-domain oscillation na may
// natural na periodicity para sa universal gates
//
// ANG TARGET:
// Oscillation na nagbibigay ng:
// - 0-level (addition/subtraction only)
// - Natural modulo (bounded values)
// - Universal computation (Rule 110 o NAND)

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  NATURAL OSCILLATION SEARCH\n";
    std::cout << "  φ-Domain Periodicity\n";
    std::cout << "========================================\n\n";

    const double phi = 1.6180339887498948482;
    const double phi_sq = phi * phi;
    const double phi_cu = phi * phi * phi;
    const double phi_qu = phi * phi * phi * phi;

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
    // SEARCH: Iba't ibang φ-based oscillations
    // ============================================
    
    std::cout << "TESTING φ-BASED OSCILLATIONS:\n";
    std::cout << "=============================\n\n";
    
    // Test 1: φ² - x (period-2)
    auto osc_phi2 = [&](auto x) { return cc->EvalSub(make_ct(phi_sq), x); };
    
    // Test 2: φ³ - x (period-2, ibang scale)
    auto osc_phi3 = [&](auto x) { return cc->EvalSub(make_ct(phi_cu), x); };
    
    // Test 3: 2φ² - x (period-2, ibang offset)
    auto osc_2phi2 = [&](auto x) { return cc->EvalSub(make_ct(2*phi_sq), x); };
    
    // Test 4: φ² + φ - x (combination)
    auto osc_comb = [&](auto x) { return cc->EvalSub(make_ct(phi_sq + phi), x); };
    
    // Test 5: φ⁴ - x (higher power)
    auto osc_phi4 = [&](auto x) { return cc->EvalSub(make_ct(phi_qu), x); };
    
    auto current = make_ct(0.0);
    
    std::cout << "OSCILLATION CHAINS (10 steps each):\n\n";
    
    // Test φ² - x
    std::cout << "1. φ² - x:\n   ";
    current = make_ct(0.0);
    for (int i = 0; i < 10; i++) {
        current = osc_phi2(current);
        std::cout << decrypt_val(current) << " ";
    }
    std::cout << "\n\n";
    
    // Test φ³ - x
    std::cout << "2. φ³ - x:\n   ";
    current = make_ct(0.0);
    for (int i = 0; i < 10; i++) {
        current = osc_phi3(current);
        std::cout << decrypt_val(current) << " ";
    }
    std::cout << "\n\n";
    
    // Test 2φ² - x
    std::cout << "3. 2φ² - x:\n   ";
    current = make_ct(0.0);
    for (int i = 0; i < 10; i++) {
        current = osc_2phi2(current);
        std::cout << decrypt_val(current) << " ";
    }
    std::cout << "\n\n";
    
    // Test φ² + φ - x
    std::cout << "4. (φ²+φ) - x:\n   ";
    current = make_ct(0.0);
    for (int i = 0; i < 10; i++) {
        current = osc_comb(current);
        std::cout << decrypt_val(current) << " ";
    }
    std::cout << "\n\n";
    
    // Test φ⁴ - x
    std::cout << "5. φ⁴ - x:\n   ";
    current = make_ct(0.0);
    for (int i = 0; i < 10; i++) {
        current = osc_phi4(current);
        std::cout << decrypt_val(current) << " ";
    }
    std::cout << "\n\n";
    
    // ============================================
    // HANAPIN ANG NATURAL PERIOD
    // ============================================
    std::cout << "NATURAL PERIOD SEARCH:\n";
    std::cout << "=======================\n\n";
    
    // Test kung may period-3, period-4, o period-5
    auto test_period = [&](const char* name, auto osc_fn, int max_steps = 20) {
        std::cout << name << ":\n";
        auto curr = make_ct(0.0);
        std::vector<double> values;
        
        for (int i = 0; i < max_steps; i++) {
            curr = osc_fn(curr);
            double v = decrypt_val(curr);
            values.push_back(v);
            
            // Check for period
            if (i > 0 && std::abs(v - values[0]) < 0.01) {
                std::cout << "  Period " << (i+1) << " found!\n";
                for (int j = 0; j <= i; j++) {
                    std::cout << "    " << values[j] << "\n";
                }
                return i + 1;
            }
        }
        
        std::cout << "  No period found in " << max_steps << " steps\n";
        return 0;
    };
    
    // Test combinations na baka may natural period
    auto osc_phi2_phi3 = [&](auto x) {
        auto temp = cc->EvalSub(make_ct(phi_sq), x);
        return cc->EvalSub(make_ct(phi_cu), temp);
    };
    
    auto osc_phi2_2phi2 = [&](auto x) {
        auto temp = cc->EvalSub(make_ct(phi_sq), x);
        return cc->EvalSub(make_ct(2*phi_sq), temp);
    };
    
    test_period("φ²→φ³ combo", osc_phi2_phi3);
    test_period("φ²→2φ² combo", osc_phi2_2phi2);
    
    std::cout << "\n========================================\n";
    std::cout << "  KEY INSIGHT:\n";
    std::cout << "  Ang natural oscillation ay yung may\n";
    std::cout << "  period na pwedeng mag-encode ng logic\n";
    std::cout << "  nang walang multiplication\n";
    std::cout << "========================================\n";

    return 0;
}
