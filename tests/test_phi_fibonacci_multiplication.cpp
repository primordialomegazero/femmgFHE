// FIBONACCI MULTIPLICATION — 0-LEVEL
// φⁿ = F(n)·φ + F(n-1) = natural multiplication!

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  FIBONACCI MULTIPLICATION — 0-LEVEL\n";
    std::cout << "  φ-Power Natural Products\n";
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
    // FIBONACCI MULTIPLICATION PATTERNS
    // ============================================
    std::cout << "FIBONACCI MULTIPLICATION PATTERNS:\n";
    std::cout << "==================================\n\n";
    
    // φ² × φ² = φ⁴
    std::cout << "1. φ² × φ² = φ⁴ = 3φ + 2\n";
    std::cout << "   " << phi_sq << " × " << phi_sq << " = " << phi_sq * phi_sq << "\n";
    std::cout << "   3φ + 2 = " << 3*phi + 2 << "\n\n";
    
    // φ × φ² = φ³
    std::cout << "2. φ × φ² = φ³ = 2φ + 1\n";
    std::cout << "   " << phi << " × " << phi_sq << " = " << phi * phi_sq << "\n";
    std::cout << "   2φ + 1 = " << 2*phi + 1 << "\n\n";
    
    // Fibonacci multiplication via φ-powers
    std::cout << "3. F(n) × F(m) via φ-powers:\n";
    std::cout << "   F(n) = (φⁿ - (-1/φ)ⁿ)/√5\n";
    std::cout << "   F(n)×F(m) = (φⁿ⁺ᵐ + ...)/5\n\n";
    
    // ============================================
    // THRESHOLD VIA φ-POWERS
    // ============================================
    std::cout << "THRESHOLD VIA φ-POWERS:\n";
    std::cout << "========================\n\n";
    
    // Ang φ-powers ay natural na thresholds:
    // φ¹ = 1.618 (between 0 and φ²)
    // φ² = 2.618 (between φ and φ³)
    // φ³ = 4.236 (between φ² and φ⁴)
    
    std::cout << "φ¹ = " << phi << " (threshold 1)\n";
    std::cout << "φ² = " << phi_sq << " (threshold 2)\n";
    std::cout << "φ³ = " << phi_cu << " (threshold 3)\n";
    std::cout << "φ⁴ = " << phi_qu << " (threshold 4)\n\n";
    
    // ============================================
    // THE KEY: MULTIPLICATION AS ADDITION OF LOGS
    // ============================================
    std::cout << "MULTIPLICATION AS ADDITION:\n";
    std::cout << "===========================\n\n";
    
    // Sa φ-base, multiplication = addition ng exponents!
    // φᵃ × φᵇ = φᵃ⁺ᵇ
    // 
    // Kung naka-encode ang value bilang exponent:
    // value = φ^exponent
    // multiplication = addition ng exponents
    // 
    // At addition ay 0-level!
    
    std::cout << "Kung value = φ^e:\n";
    std::cout << "  multiply(a,b) = φ^(e_a + e_b)\n";
    std::cout << "  = ADDITION ng exponents = 0-LEVEL!\n\n";
    
    // ============================================
    // PRACTICAL TEST: EXPONENTIAL ENCODING
    // ============================================
    std::cout << "EXPONENTIAL ENCODING TEST:\n";
    std::cout << "===========================\n\n";
    
    // I-encode ang 0 bilang φ⁰ = 1
    // I-encode ang 1 bilang φ¹ = φ
    // I-encode ang 2 bilang φ² = φ²
    // I-encode ang 3 bilang φ³ = φ³
    
    // Para sa NAND:
    // NAND(a,b) = 2 - (a+b)
    // Sa exponents: φ^(2 - (e_a + e_b))
    // = φ² / (φ^e_a × φ^e_b)
    // = φ² × φ^(-e_a) × φ^(-e_b)
    // = φ^(2 - e_a - e_b)
    
    // Ang 2 - e_a - e_b ay ADDITION/SUBTRACTION ng exponents!
    // 0-LEVEL!
    
    std::cout << "NAND via exponents:\n";
    std::cout << "  e_result = 2 - e_a - e_b\n";
    std::cout << "  result = φ^e_result\n\n";
    
    // Test: NAND(0,0) kung saan 0 = φ⁰ = 1
    // e_a = 0, e_b = 0
    // e_result = 2 - 0 - 0 = 2
    // result = φ² = 2.618 (true sa ternary)
    
    // Test: NAND(1,1) kung saan 1 = φ¹ = φ
    // e_a = 1, e_b = 1
    // e_result = 2 - 1 - 1 = 0
    // result = φ⁰ = 1 (false, pero 1 hindi 0!)
    
    std::cout << "PROBLEMA: φ⁰ = 1, hindi 0\n";
    std::cout << "Kailangan ng offset para sa false\n\n";
    
    // ============================================
    // ANG TUNAY NA BREAKTHROUGH: LOGARITHMIC DOMAIN
    // ============================================
    std::cout << "LOGARITHMIC DOMAIN:\n";
    std::cout << "===================\n\n";
    
    // Sa logarithmic domain:
    // false = -∞ (o very negative)
    // true = 0 (o φ⁰)
    // 
    // NAND(a,b) = NOT(a AND b)
    // Sa log domain: AND = max(a,b)
    // NOT(x) = -x
    // NAND(a,b) = -max(a,b) = min(-a,-b)
    
    std::cout << "Log-domain NAND:\n";
    std::cout << "  AND = max(a,b)\n";
    std::cout << "  NOT = negation\n";
    std::cout << "  NAND = -max(a,b) = min(-a,-b)\n\n";
    
    std::cout << "PERO max at min ay kailangan ng comparison...\n";
    std::cout << "NA KAILANGAN NG THRESHOLD!\n\n";
    
    // ============================================
    // FINAL INSIGHT
    // ============================================
    std::cout << "FINAL INSIGHT:\n";
    std::cout << "==============\n";
    std::cout << "1. Multiplication = addition sa φ-exponent domain\n";
    std::cout << "2. Ito ay 0-level!\n";
    std::cout << "3. Ang threshold ay natural sa φ-powers\n";
    std::cout << "4. Ang NAND ay kaya sa logarithmic domain\n";
    std::cout << "5. PERO kailangan ng max/min operation\n\n";
    
    std::cout << "SUSUNOD: Hanapin ang 0-level max/min\n";
    std::cout << "gamit ang φ-threshold\n";
    
    return 0;
}
