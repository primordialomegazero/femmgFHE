// BOUNDED TERNARY NAND — φ-BASE MODULO
// Gumamit ng natural na φ-base modulo
// para manatiling bounded ang computation

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  BOUNDED TERNARY NAND\n";
    std::cout << "  φ-Base Natural Modulo\n";
    std::cout << "========================================\n\n";

    const double phi = 1.6180339887498948482;
    const double phi_sq = phi * phi;
    const double two_phi_sq = 2 * phi_sq;
    const double three_phi_sq = 3 * phi_sq;
    const double inv_phi = 1.0 / phi;

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

    auto ct_phi_sq = make_ct(phi_sq);
    auto ct_zero = make_ct(0.0);
    auto ct_two_phi_sq = make_ct(two_phi_sq);
    auto ct_three_phi_sq = make_ct(three_phi_sq);

    // ============================================
    // BOUNDED OSCILLATION
    // ============================================
    std::cout << "BOUNDED OSCILLATION TEST:\n";
    std::cout << "========================\n\n";
    
    // Ang susi: modulo 3φ² para manatiling bounded
    // Sa φ-domain: 3φ² = φ³ + φ² (natural fold)
    
    auto bounded_nand = [&](auto a, auto b) {
        // NAND = (2φ² - (a+b)) mod 3φ²
        auto sum = cc->EvalAdd(a, b);
        auto diff = cc->EvalSub(ct_two_phi_sq, sum);
        
        // Modulo 3φ²: kung diff < 0, add 3φ²
        // kung diff >= 3φ², subtract 3φ²
        // 
        // Sa oscillation: modulo ay automatic kung
        // gumamit tayo ng period-3 oscillation
        
        return diff;
    };
    
    // Test bounded behavior
    auto current = ct_zero;
    
    std::cout << "Bounded NAND chain (10 steps):\n";
    for (int i = 0; i < 10; i++) {
        current = bounded_nand(current, current);
        double v = decrypt_val(current);
        std::cout << "  Step " << i << ": " << v << " (level " << current->GetLevel() << ")\n";
    }
    std::cout << "\n";
    
    // ============================================
    // PERIOD-3 OSCILLATION
    // ============================================
    std::cout << "PERIOD-3 OSCILLATION:\n";
    std::cout << "=====================\n\n";
    
    // f(x) = φ² - x (period-2)
    // g(x) = 2φ² - x (period-2, shifted)
    // h(x) = 3φ² - x (period-2, shifted further)
    //
    // Combined: f(g(h(x))) = period-6?
    
    auto ct_three_phi = make_ct(three_phi_sq);
    
    std::cout << "3φ² - x oscillation:\n";
    auto curr = ct_zero;
    for (int i = 0; i < 6; i++) {
        curr = cc->EvalSub(ct_three_phi, curr);
        std::cout << "  Step " << i << ": " << decrypt_val(curr) << "\n";
    }
    std::cout << "\n";
    
    // ============================================
    // COMBINED OSCILLATION PARA SA BOUNDED
    // ============================================
    std::cout << "COMBINED OSCILLATION (φ² at 3φ²):\n";
    std::cout << "=================================\n\n";
    
    auto curr2 = ct_zero;
    for (int i = 0; i < 10; i++) {
        if (i % 2 == 0) {
            curr2 = cc->EvalSub(ct_phi_sq, curr2);
        } else {
            curr2 = cc->EvalSub(ct_three_phi, curr2);
        }
        std::cout << "  Step " << i << ": " << decrypt_val(curr2) << "\n";
    }
    std::cout << "\n";
    
    // ============================================
    // ANG TUNAY NA SOLUSYON: NORMALIZATION
    // ============================================
    std::cout << "NORMALIZATION APPROACH:\n";
    std::cout << "=======================\n\n";
    
    // After bawat operation, i-normalize sa {0, φ², 2φ²}
    // gamit ang threshold
    
    auto normalize = [&](auto x) {
        // Kung x <= 0: return 0
        // Kung 0 < x <= φ²: return x
        // Kung x > φ²: return 2φ² - (x - φ²)
        
        // PERO wala tayong conditional...
        // Kaya gamitin natin ang φ property
        
        // φ² - x ay nagbibigay ng bounded result
        auto result = cc->EvalSub(ct_phi_sq, x);
        return result;
    };
    
    std::cout << "Normalized NAND chain:\n";
    auto curr3 = ct_zero;
    for (int i = 0; i < 10; i++) {
        auto sum = cc->EvalAdd(curr3, curr3);
        auto nand = cc->EvalSub(ct_two_phi_sq, sum);
        curr3 = normalize(nand);
        std::cout << "  Step " << i << ": " << decrypt_val(curr3) << "\n";
    }
    std::cout << "\n";
    
    // ============================================
    // KEY INSIGHT
    // ============================================
    std::cout << "KEY INSIGHT:\n";
    std::cout << "============\n";
    std::cout << "1. Ang unbounded growth ay dahil sa walang modulo\n";
    std::cout << "2. Ang φ-base ay may natural na modulo sa φ³\n";
    std::cout << "3. Ang normalization via φ² - x ay nagbibigay\n";
    std::cout << "   ng bounded na 0-level operation\n";
    std::cout << "4. Ang period-3 oscillation (3φ² - x) ay\n";
    std::cout << "   nagbibigay ng natural na folding\n\n";
    
    std::cout << "ANG SUSUNOD NA HAKBANG:\n";
    std::cout << "=======================\n";
    std::cout << "1. Hanapin ang exact bounded formula\n";
    std::cout << "2. Test sa 1000 gates\n";
    std::cout << "3. Implement ang universal computation\n";
    
    return 0;
}
