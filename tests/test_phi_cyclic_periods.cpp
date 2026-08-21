// φ-CYCLIC PERIODS — HANAPIN ANG MAS MAHABANG CYCLE
// Subukan natin ang iba't ibang φ-based transformations
// para makakita ng period-3, period-4, o period-5

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  φ-CYCLIC PERIODS\n";
    std::cout << "  Beyond Period-2\n";
    std::cout << "========================================\n\n";

    const double phi = 1.6180339887498948482;
    const double phi_sq = phi * phi;
    const double phi_cu = phi * phi * phi;

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
    // CYCLIC SHIFT MAPS
    // ============================================
    
    std::cout << "CYCLIC SHIFT SEARCH:\n";
    std::cout << "====================\n\n";
    
    // Map 1: x → (x + φ²) mod φ³ (cyclic addition)
    auto cyclic_add = [&](auto x) {
        auto added = cc->EvalAdd(x, make_ct(phi_sq));
        // Modulo via oscillation: (φ³ - x) gives 0-level modulo
        return cc->EvalSub(make_ct(phi_cu), added);
    };
    
    // Map 2: x → φ² - x - φ (offset oscillation)
    auto offset_osc = [&](auto x) {
        auto temp = cc->EvalSub(make_ct(phi_sq), x);
        return cc->EvalSub(temp, make_ct(phi));
    };
    
    // Map 3: x → 2φ² - x - φ (double offset)
    auto double_offset = [&](auto x) {
        auto temp = cc->EvalSub(make_ct(2*phi_sq), x);
        return cc->EvalSub(temp, make_ct(phi));
    };
    
    // Map 4: x → φ³ - x - φ² (higher order)
    auto higher_osc = [&](auto x) {
        auto temp = cc->EvalSub(make_ct(phi_cu), x);
        return cc->EvalSub(temp, make_ct(phi_sq));
    };
    
    // Test each map for periodicity
    auto test_map = [&](const char* name, auto map_fn, int max_steps = 15) {
        std::cout << name << ":\n  ";
        auto current = make_ct(0.0);
        std::vector<double> values;
        
        for (int i = 0; i < max_steps; i++) {
            current = map_fn(current);
            double v = decrypt_val(current);
            values.push_back(v);
            std::cout << v << " ";
            
            // Check for period
            if (i > 0 && std::abs(v - values[0]) < 0.01) {
                std::cout << "\n  → Period " << (i+1) << " found!\n\n";
                return;
            }
        }
        std::cout << "\n  → No period in " << max_steps << " steps\n\n";
    };
    
    test_map("Map 1: (x+φ²) mod φ³", cyclic_add);
    test_map("Map 2: φ² - x - φ", offset_osc);
    test_map("Map 3: 2φ² - x - φ", double_offset);
    test_map("Map 4: φ³ - x - φ²", higher_osc);
    
    std::cout << "========================================\n";
    std::cout << "  KEY: Ang map na may period > 2\n";
    std::cout << "  ay pwedeng mag-encode ng more states\n";
    std::cout << "========================================\n";

    return 0;
}
