// MIXED φ-BASE REPRESENTATION — BREAKING ASSUMPTIONS
// value = a·φ + b kung saan a,b ∈ {0,1}
// Modulo ay automatic sa φ-base!

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  MIXED φ-BASE REPRESENTATION\n";
    std::cout << "  Breaking Traditional Assumptions\n";
    std::cout << "========================================\n\n";

    const double phi = 1.6180339887498948482;
    const double phi_sq = phi * phi;
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

    // IMPORTANTE: Declare lahat ng ciphertexts!
    auto ct_phi_sq = make_ct(phi_sq);
    auto ct_neg_phi_sq = make_ct(-phi_sq);
    auto ct_half_phi_sq = make_ct(phi_sq / 2.0);
    auto ct_zero = make_ct(0.0);

    // ============================================
    // THEORY: MIXED φ-BASE
    // ============================================
    std::cout << "THEORY:\n";
    std::cout << "=======\n\n";
    
    std::cout << "Sa φ-base, ang bawat value ay may 2 components:\n";
    std::cout << "  value = a·φ + b\n";
    std::cout << "  kung saan a,b ∈ {0,1}\n\n";
    
    std::cout << "0 = 0·φ + 0\n";
    std::cout << "1 = 0·φ + 1\n";
    std::cout << "φ = 1·φ + 0\n";
    std::cout << "φ² = 1·φ + 1\n\n";
    
    std::cout << "Ang addition sa φ-base ay automatic carry:\n";
    std::cout << "  φ + φ = 2φ = φ² + φ⁻¹\n";
    std::cout << "  (φ + φ) = (1·φ + 0) + (1·φ + 0) = 2φ + 0\n";
    std::cout << "  = φ² + φ⁻¹ = (1·φ + 1) + (0·φ + 1/φ)\n\n";
    
    std::cout << "KEY: Ang φ-base ay may NATURAL na modulo!\n";
    std::cout << "  2φ = φ² + φ⁻¹ (auto-fold)\n";
    std::cout << "  3φ = φ² + φ (auto-carry)\n\n";
    
    // ============================================
    // TEST: XOR SA MIXED φ-BASE
    // ============================================
    std::cout << "XOR SA MIXED φ-BASE:\n";
    std::cout << "====================\n\n";
    
    std::cout << "XOR(φ², φ²) sa mixed base:\n";
    std::cout << "  φ² = φ + 1\n";
    std::cout << "  φ² + φ² = 2φ + 2\n";
    std::cout << "  = 2φ + 2\n";
    std::cout << "  = φ² + φ⁻¹ + 2 (auto-fold!)\n";
    std::cout << "  = φ + 1 + φ - 1 + 2 (since φ⁻¹ = φ - 1)\n";
    std::cout << "  = 2φ + 2\n\n";
    
    // ============================================
    // THE REAL BREAKTHROUGH: NEGATIVE φ OSCILLATION
    // ============================================
    std::cout << "NEGATIVE φ OSCILLATION:\n";
    std::cout << "=======================\n\n";
    
    std::cout << "f(x) = -φ² - x:\n";
    std::cout << "  f(0) = -φ²\n";
    std::cout << "  f(-φ²) = 0\n";
    std::cout << "  f(φ²) = -2φ²\n";
    std::cout << "  f(-2φ²) = φ²\n\n";
    
    std::cout << "COMBINED OSCILLATION:\n";
    std::cout << "  g(x) = (φ² - x) + (-φ² - x) = -2x\n";
    std::cout << "  Ito ay LINEAR (walang folding)!\n\n";
    
    std::cout << "PERO kung alternating:\n";
    std::cout << "  Step 1: φ² - x\n";
    std::cout << "  Step 2: -φ² - x\n";
    std::cout << "  Step 3: φ² - x\n";
    std::cout << "  Step 4: -φ² - x\n\n";
    
    // Test alternating oscillation
    std::cout << "ALTERNATING OSCILLATION TEST:\n";
    std::cout << "=============================\n";
    
    auto curr = ct_zero;
    for (int i = 0; i < 8; i++) {
        if (i % 2 == 0) {
            curr = cc->EvalSub(ct_phi_sq, curr);
        } else {
            curr = cc->EvalSub(ct_neg_phi_sq, curr);
        }
        double v = decrypt_val(curr);
        std::cout << "  Step " << i+1 << ": " << v;
        // I-classify ang value
        if (std::abs(v) < 0.001) std::cout << " (ZERO)";
        else if (std::abs(v - phi_sq) < 0.001) std::cout << " (+φ²)";
        else if (std::abs(v + phi_sq) < 0.001) std::cout << " (-φ²)";
        else if (std::abs(v - 2*phi_sq) < 0.001) std::cout << " (+2φ²)";
        else if (std::abs(v + 2*phi_sq) < 0.001) std::cout << " (-2φ²)";
        else std::cout << " (OTHER)";
        std::cout << " [level " << curr->GetLevel() << "]\n";
    }
    std::cout << "\n";
    
    // ============================================
    // THE ULTIMATE TRICK: φ²/2 OSCILLATION
    // ============================================
    std::cout << "φ²/2 OSCILLATION:\n";
    std::cout << "=================\n\n";
    
    std::cout << "f(x) = φ²/2 - x:\n";
    std::cout << "  f(0) = φ²/2\n";
    std::cout << "  f(φ²/2) = 0\n";
    std::cout << "  f(φ²) = -φ²/2\n";
    std::cout << "  f(-φ²/2) = φ²\n\n";
    
    auto curr2 = ct_zero;
    for (int i = 0; i < 6; i++) {
        curr2 = cc->EvalSub(ct_half_phi_sq, curr2);
        double v = decrypt_val(curr2);
        std::cout << "  Step " << i+1 << ": " << v;
        if (std::abs(v) < 0.001) std::cout << " (ZERO)";
        else if (std::abs(v - phi_sq/2) < 0.001) std::cout << " (+φ²/2)";
        else if (std::abs(v + phi_sq/2) < 0.001) std::cout << " (-φ²/2)";
        else if (std::abs(v - phi_sq) < 0.001) std::cout << " (+φ²)";
        else if (std::abs(v + phi_sq) < 0.001) std::cout << " (-φ²)";
        else std::cout << " (OTHER)";
        std::cout << " [level " << curr2->GetLevel() << "]\n";
    }
    std::cout << "\n";
    
    // ============================================
    // BEHAVIOR ANALYSIS
    // ============================================
    std::cout << "BEHAVIOR ANALYSIS:\n";
    std::cout << "==================\n\n";
    
    std::cout << "1. Standard oscillation (φ² - x): period-2\n";
    std::cout << "   States: {0, φ²}\n\n";
    
    std::cout << "2. Negative oscillation (-φ² - x): period-2\n";
    std::cout << "   States: {0, -φ²}\n\n";
    
    std::cout << "3. Alternating: period-4\n";
    std::cout << "   States: {0, φ², -φ², 2φ², ...}\n\n";
    
    std::cout << "4. Half oscillation (φ²/2 - x): period-4\n";
    std::cout << "   States: {0, φ²/2, -φ²/2, φ², ...}\n\n";
    
    std::cout << "KEY INSIGHT:\n";
    std::cout << "============\n";
    std::cout << "Ang half oscillation ay nagbibigay ng\n";
    std::cout << "MAS MARAMING states na maaaring magamit\n";
    std::cout << "para sa finer threshold detection.\n";
    std::cout << "Ang alternating oscillation ay nagbibigay\n";
    std::cout << "ng asymmetric states na maaaring magamit\n";
    std::cout << "para sa sign detection.\n";
    
    return 0;
}
