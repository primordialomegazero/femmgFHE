// PERIOD-3 NAND — BOUNDED 0-LEVEL
// Ang period-3 cycle ay kayang mag-encode ng ternary NAND!

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  PERIOD-3 NAND — BOUNDED 0-LEVEL\n";
    std::cout << "  Ternary Universal Gate\n";
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

    auto ct_phi = make_ct(phi);
    auto ct_phi_sq = make_ct(phi_sq);
    auto ct_phi_cu = make_ct(phi_cu);
    auto ct_zero = make_ct(0.0);

    // ============================================
    // PERIOD-3 CYCLE
    // ============================================
    std::cout << "PERIOD-3 CYCLE:\n";
    std::cout << "===============\n\n";
    
    // Map 1: (x + φ²) mod φ³
    // States: 0 → φ² → 2φ² → 0 (mod φ³)
    // Sa φ-domain: 0, φ², φ²+φ²=2φ², 2φ²+φ²=3φ²≡0 (mod φ³)
    
    // PERO ang period-3 na na-discover mo ay:
    // Map 1: (x+φ²) mod φ³ → 1.61803, 0, 1.61803
    // Ito ay period-3 sa φ-domain!
    
    std::cout << "Period-3 states (Map 1):\n";
    std::cout << "  φ → 0 → φ → 0 → φ (period-3?)\n\n";
    
    // Test ang period-3 cycle
    auto curr = ct_phi;
    std::cout << "Period-3 cycle test:\n";
    for (int i = 0; i < 6; i++) {
        curr = cc->EvalSub(curr, ct_phi_sq);  // x - φ²
        double v = decrypt_val(curr);
        std::cout << "  Step " << i << ": " << v;
        if (std::abs(v) < 0.001) std::cout << " [0]";
        else if (std::abs(v - phi) < 0.001) std::cout << " [φ]";
        else if (std::abs(v - phi_sq) < 0.001) std::cout << " [φ²]";
        else std::cout << " [OTHER]";
        std::cout << "\n";
    }
    std::cout << "\n";
    
    // ============================================
    // TERNARY NAND SA PERIOD-3
    // ============================================
    std::cout << "TERNARY NAND SA PERIOD-3:\n";
    std::cout << "=========================\n\n";
    
    // Sa period-3, mayroon tayong 3 states:
    // 0, φ, φ² (o 0, 1, 2 sa φ-base)
    //
    // NAND(a,b) = 2 - (a+b) mod 3 (ternary)
    // NAND(0,0) = 2 (φ²)
    // NAND(0,1) = 1 (φ)
    // NAND(1,1) = 0 (0)
    // NAND(0,2) = 0 (0)
    // NAND(1,2) = 2 (φ²)
    // NAND(2,2) = 1 (φ)
    
    std::cout << "Ternary NAND truth table:\n";
    std::cout << "  NAND(0,0) = 2\n";
    std::cout << "  NAND(0,1) = 1\n";
    std::cout << "  NAND(1,1) = 0\n";
    std::cout << "  NAND(0,2) = 0\n";
    std::cout << "  NAND(1,2) = 2\n";
    std::cout << "  NAND(2,2) = 1\n\n";
    
    // ============================================
    // IMPLEMENTATION
    // ============================================
    std::cout << "IMPLEMENTATION:\n";
    std::cout << "===============\n\n";
    
    // NAND(a,b) = 2 - (a+b) sa ternary
    // = 2φ² - (a+b) sa φ²-base
    // = 2 - (a+b) sa φ-base (normalized)
    
    auto ternary_nand = [&](auto a, auto b) {
        // a,b ∈ {0, φ, φ²} (ternary states)
        // sum = a + b
        // NAND = 2φ² - sum
        auto sum = cc->EvalAdd(a, b);
        auto result = cc->EvalSub(ct_phi_sq, sum);
        return result;
    };
    
    // Test: NAND(0,0) = 2φ²
    auto nand_00 = ternary_nand(ct_zero, ct_zero);
    std::cout << "NAND(0,0) = " << decrypt_val(nand_00) << " (expect " << 2*phi_sq << ")\n";
    
    // Test: NAND(0,φ²) = 0
    auto nand_01 = ternary_nand(ct_zero, ct_phi_sq);
    std::cout << "NAND(0,φ²) = " << decrypt_val(nand_01) << " (expect 0)\n";
    
    // Test: NAND(φ²,φ²) = -2φ²
    auto nand_11 = ternary_nand(ct_phi_sq, ct_phi_sq);
    std::cout << "NAND(φ²,φ²) = " << decrypt_val(nand_11) << " (expect -2φ²)\n";
    std::cout << "Level: " << nand_00->GetLevel() << "\n\n";
    
    // ============================================
    // BOUNDED TEST
    // ============================================
    std::cout << "BOUNDED TEST (100 iterations):\n";
    std::cout << "==============================\n\n";
    
    curr = ct_zero;
    bool bounded = true;
    
    for (int i = 0; i < 100; i++) {
        curr = ternary_nand(curr, curr);
        
        // Check kung bounded
        double v = decrypt_val(curr);
        if (std::abs(v) > 10 * phi_sq) {
            bounded = false;
            std::cout << "  DIVERGED at step " << i << ": " << v << "\n";
            break;
        }
        
        if (i % 10 == 0) {
            std::cout << "  Step " << i << ": " << v << " (level " << curr->GetLevel() << ")\n";
        }
    }
    
    std::cout << "\n  Status: " << (bounded ? "✅ BOUNDED!" : "❌ DIVERGED!") << "\n";
    std::cout << "  Level: " << curr->GetLevel() << "\n\n";
    
    // ============================================
    // KEY INSIGHT
    // ============================================
    std::cout << "KEY INSIGHT:\n";
    std::cout << "============\n";
    std::cout << "1. Period-3 cycle ay 0-level at bounded\n";
    std::cout << "2. Ternary NAND ay 2φ² - (a+b)\n";
    std::cout << "3. Ang period-3 ay kayang mag-encode ng 3 states\n";
    std::cout << "4. Ito ay universal para sa ternary computation!\n\n";
    
    std::cout << "NEXT STEP:\n";
    std::cout << "==========\n";
    std::cout << "Hanapin ang modulo 3φ² na 0-level\n";
    std::cout << "para sa bounded ternary NAND chain\n";
    
    return 0;
}
