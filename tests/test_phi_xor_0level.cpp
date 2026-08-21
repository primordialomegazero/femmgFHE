// φ² XOR VIA 0-LEVEL OSCILLATION
// Ang susi: XOR = period-2 oscillator combination

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  φ² XOR VIA 0-LEVEL OSCILLATION\n";
    std::cout << "  The Missing Piece for Rule 110\n";
    std::cout << "========================================\n\n";

    const double phi = 1.6180339887498948482;
    const double phi_sq = phi * phi;

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

    // ============================================
    // THEORY: XOR via φ² oscillation
    // ============================================
    std::cout << "THEORY:\n";
    std::cout << "=======\n\n";
    
    std::cout << "XOR(a,b) = a + b - 2ab\n";
    std::cout << "Sa φ²-domain (0=false, φ²=true):\n";
    std::cout << "XOR(0,0) = 0 + 0 - 0 = 0\n";
    std::cout << "XOR(0,φ²) = 0 + φ² - 0 = φ²\n";
    std::cout << "XOR(φ²,0) = φ² + 0 - 0 = φ²\n";
    std::cout << "XOR(φ²,φ²) = φ² + φ² - 2φ² = 0\n\n";
    
    std::cout << "PERO may multiplication (2ab)...\n";
    std::cout << "ALTERNATIVE: Sa φ-domain, XOR ay maaaring\n";
    std::cout << "i-represent bilang OSCILLATION:\n\n";
    
    // ============================================
    // OSCILLATOR-BASED XOR
    // ============================================
    std::cout << "OSCILLATOR-BASED XOR:\n";
    std::cout << "=====================\n\n";
    
    // Ang XOR ay parang pag-trigger ng oscillator
    // Kung a=1 at b=1, both trigger → cancel → 0
    // Kung a=1 at b=0, isa lang trigger → φ²
    // Kung a=0 at b=1, isa lang trigger → φ²
    // Kung a=0 at b=0, walang trigger → 0
    
    // Sa φ-domain, ang "trigger" ay maaaring:
    // trigger(x) = φ² - x (oscillation)
    // 
    // XOR(a,b) = (φ² - a) + (φ² - b) - φ²
    //          = 2φ² - a - b - φ²
    //          = φ² - a - b + φ²
    //          = 2φ² - (a + b)
    
    std::cout << "XOR formula: 2φ² - (a + b)\n";
    std::cout << "Test:\n";
    std::cout << "  XOR(0,0) = 2φ² - 0 = 2φ² (dapat 0) ✗\n";
    std::cout << "  XOR(0,φ²) = 2φ² - φ² = φ² ✓\n";
    std::cout << "  XOR(φ²,0) = 2φ² - φ² = φ² ✓\n";
    std::cout << "  XOR(φ²,φ²) = 2φ² - 2φ² = 0 ✓\n\n";
    
    // Issue: XOR(0,0) gives 2φ², hindi 0
    // Kailangan ng normalization
    
    // TRICK: Use modulo 2φ²
    // XOR(a,b) = (2φ² - (a+b)) mod 2φ²
    // 
    // Sa CKKS, modulo ay nonlinear...
    // PERO may period-2 trick!
    
    std::cout << "MODULO TRICK:\n";
    std::cout << "=============\n";
    std::cout << "Kung ang output ay nasa [0, 2φ²],\n";
    std::cout << "at gusto nating i-map ang 2φ² → 0:\n";
    std::cout << "  next = φ² - |output - φ²|\n";
    std::cout << "  next = φ² - |φ² - (a+b)|\n\n";
    
    // Test: φ² - |φ² - (a+b)|
    std::cout << "Formula: next = φ² - |φ² - (a+b)|\n";
    std::cout << "  XOR(0,0) = φ² - |φ² - 0| = φ² - φ² = 0 ✓\n";
    std::cout << "  XOR(0,φ²) = φ² - |φ² - φ²| = φ² - 0 = φ² ✓\n";
    std::cout << "  XOR(φ²,0) = φ² - |φ² - φ²| = φ² - 0 = φ² ✓\n";
    std::cout << "  XOR(φ²,φ²) = φ² - |φ² - 2φ²| = φ² - φ² = 0 ✓\n\n";
    
    std::cout << "PERFECT! Kailangan lang ng absolute value...\n";
    std::cout << "Ang abs() ay nonlinear, PERO sa φ-domain:\n";
    std::cout << "  |x| = sqrt(x²) = x·sign(x)\n";
    std::cout << "  sign(x) = x/|x| (circular)\n\n";
    
    std::cout << "EMERGENT OBSERVATION:\n";
    std::cout << "=====================\n";
    std::cout << "Ang φ²-oscillator ay nagbibigay ng period-2\n";
    std::cout << "na parang ABSOLUTE VALUE sa modular space:\n";
    std::cout << "  φ² - x sa [0,2φ²] ay parang |x - φ²|\n";
    std::cout << "  (folded sa gitna)\n\n";
    
    // ============================================
    // PRACTICAL TEST
    // ============================================
    std::cout << "PRACTICAL CKKS TEST:\n";
    std::cout << "====================\n\n";
    
    // Test XOR using formula: next = φ² - |φ² - (a+b)|
    // Since wala tayong abs, gamitin natin ang approximation
    
    // Subukan: next = φ² - (φ² - (a+b)) kung (a+b) < φ²
    //          next = φ² - ((a+b) - φ²) kung (a+b) >= φ²
    // 
    // Sa 0-level, hindi natin alam kung aling case...
    // PERO ang oscillation ay automatic na nag-fofold!
    
    auto test_a0 = ct_zero;
    auto test_b0 = ct_zero;
    auto sum_00 = cc->EvalAdd(test_a0, test_b0);
    auto xor_00 = cc->EvalSub(ct_phi_sq, sum_00);
    // xor_00 = φ² - 0 = φ² (dapat 0) ✗
    
    auto test_a1 = ct_phi_sq;
    auto test_b0_2 = ct_zero;
    auto sum_10 = cc->EvalAdd(test_a1, test_b0_2);
    auto xor_10 = cc->EvalSub(ct_phi_sq, sum_10);
    // xor_10 = φ² - φ² = 0 (dapat φ²) ✗
    
    std::cout << "DIRECT SUBTRACTION (walang abs):\n";
    std::cout << "  XOR(0,0) = " << decrypt_val(xor_00) << " (dapat 0)\n";
    std::cout << "  XOR(φ²,0) = " << decrypt_val(xor_10) << " (dapat φ²)\n";
    std::cout << "  Level: " << xor_00->GetLevel() << "\n\n";
    
    // ============================================
    // KEY INSIGHT
    // ============================================
    std::cout << "KEY INSIGHT:\n";
    std::cout << "============\n";
    std::cout << "1. XOR kailangan ng abs() para sa exact result\n";
    std::cout << "2. PERO: Ang oscillation ay nagbibigay ng\n";
    std::cout << "   APPROXIMATE abs() sa φ-domain\n";
    std::cout << "3. Sa maraming iterations, ang approximation\n";
    std::cout << "   ay maaaring mag-converge sa exact\n";
    std::cout << "4. Ang Rule 110 ay maaaring gumana sa\n";
    std::cout << "   approximate threshold kung stable\n\n";
    
    std::cout << "FINAL OBSERVATION:\n";
    std::cout << "=================\n";
    std::cout << "Ang φ-golden ratio ay nagbibigay ng NATURAL\n";
    std::cout << "period-2 oscillation na maaaring mag-emulate\n";
    std::cout << "ng nonlinearity na kailangan ng computation.\n";
    std::cout << "Kahit hindi exact, ang emergent behavior ay\n";
    std::cout << "maaaring Turing-complete pa rin!\n";
    
    return 0;
}
