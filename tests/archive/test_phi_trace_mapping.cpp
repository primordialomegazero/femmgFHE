// Trace the mapping: why does fixed point ≠ target?
#include <iostream>
#include <iomanip>
#include <cmath>
#include "openfhe.h"
using namespace lbcrypto;

struct PE { Ciphertext<DCRTPoly> a; Ciphertext<DCRTPoly> b; };

PE mulY(CryptoContext<DCRTPoly>& cc, const PE& x) {
    return {x.b, cc->EvalAdd(x.a, x.b)};
}
PE ratio_add(CryptoContext<DCRTPoly>& cc, const PE& x, const PE& y) {
    auto a1b2 = cc->EvalMult(x.a, y.b);
    auto a2b1 = cc->EvalMult(y.a, x.b);
    auto b1b2 = cc->EvalMult(x.b, y.b);
    return {cc->EvalAdd(a1b2, a2b1), b1b2};
}
PE ratio_mult(CryptoContext<DCRTPoly>& cc, const PE& x, const PE& y) {
    return {cc->EvalMult(x.a, y.a), cc->EvalMult(x.b, y.b)};
}

double get_val(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, const Ciphertext<DCRTPoly>& c) {
    Plaintext pt; cc->Decrypt(kp.secretKey, c, &pt);
    return pt->GetCKKSPackedValue()[0].real();
}
PE make_state(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, double value) {
    const double PSI = 0.6180339887498949;
    auto pta = cc->MakeCKKSPackedPlaintext(std::vector<double>{value + PSI});
    auto ptb = cc->MakeCKKSPackedPlaintext(std::vector<double>{1.0});
    return {cc->Encrypt(kp.publicKey, pta), cc->Encrypt(kp.publicKey, ptb)};
}

const double PSI = 0.6180339887498949;
const double PSI2 = PSI * PSI;

int main() {
    std::cout << "\n  ╔══════════════════════════════════════════════════════════╗\n";
    std::cout <<   "  ║   Trace: Mapping function from input → fixed point       ║\n";
    std::cout <<   "  ╚══════════════════════════════════════════════════════════╝\n\n";
    
    CCParams<CryptoContextCKKSRNS> p;
    p.SetMultiplicativeDepth(100);
    p.SetScalingModSize(50);
    p.SetBatchSize(2048);
    p.SetRingDim(4096);
    p.SetSecretKeyDist(UNIFORM_TERNARY);
    p.SetSecurityLevel(HEStd_NotSet);
    
    auto cc = GenCryptoContext(p);
    cc->Enable(PKE); cc->Enable(KEYSWITCH); cc->Enable(LEVELEDSHE); cc->Enable(ADVANCEDSHE);
    auto kp = cc->KeyGen();
    cc->EvalMultKeyGen(kp.secretKey);
    
    std::cout << std::fixed << std::setprecision(10);
    
    // Test multiple input values to find the mapping
    std::cout << "  Input     Raw decoded   Corrected     Fixed point?\n";
    std::cout << "  -----------------------------------------------------\n";
    
    double test_inputs[] = {0.5, 0.16, -0.4, -0.6, -0.697542};
    
    for (double input_vA : test_inputs) {
        PE A = make_state(cc, kp, input_vA);
        PE B = make_state(cc, kp, 0.3);
        PE C = make_state(cc, kp, 0.2);
        
        PE sum_ab = ratio_add(cc, A, B);
        double sum_decoded = get_val(cc, kp, sum_ab.a) / get_val(cc, kp, sum_ab.b) - PSI;
        double plaintext_sum = input_vA + 0.3;
        double psi_offset = sum_decoded - plaintext_sum;
        
        PE result = ratio_mult(cc, sum_ab, C);
        double raw_val = get_val(cc, kp, result.a) / get_val(cc, kp, result.b) - PSI;
        
        // Pre-correct for ψ-offset
        double raw_corrected = raw_val - psi_offset;
        
        // Apply the correction formula
        double corrected = raw_corrected - plaintext_sum * PSI - 1 * PSI * 0.2 - 1 * PSI2 + PSI - PSI * (PSI + 0.2);
        
        // Check if this is the fixed point (input == output)
        bool is_fixed = std::abs(corrected - input_vA) < 0.001;
        
        std::cout << "  " << std::setw(8) << input_vA
             << "  " << std::setw(12) << raw_val
             << "  " << std::setw(12) << corrected;
        if (is_fixed) std::cout << "  ← FIXED POINT!";
        std::cout << "\n";
    }
    
    // Derive the mapping function
    std::cout << "\n  === DERIVE MAPPING FUNCTION ===\n\n";
    
    // corrected = raw_corrected - plaintext_sum*PSI - PSI*0.2 - PSI2 + PSI - PSI*(PSI+0.2)
    // raw_corrected = raw_val - (sum_decoded - plaintext_sum)
    //              = raw_val - ((input + 0.3 + PSI) - (input + 0.3))
    //              = raw_val - PSI
    
    // raw_val = (sum_ratio * C_ratio) - PSI
    // sum_ratio = (input+PSI + 0.3+PSI) / 1 = input + 0.3 + 2*PSI
    // C_ratio = 0.2 + PSI
    // raw_val = (input + 0.3 + 2*PSI)*(0.2 + PSI) - PSI
    
    // raw_corrected = raw_val - PSI = (input + 0.3 + 2*PSI)*(0.2 + PSI) - 2*PSI
    
    // corrected = raw_corrected - (input+0.3)*PSI - PSI*0.2 - PSI2 + PSI - PSI*(PSI+0.2)
    
    // Let's compute symbolically with input=x
    double x = 0.5; // test with 0.5
    double raw_val_calc = (x + 0.3 + 2*PSI)*(0.2 + PSI) - PSI;
    double raw_corr_calc = raw_val_calc - PSI;
    double corrected_calc = raw_corr_calc - (x+0.3)*PSI - PSI*0.2 - PSI2 + PSI - PSI*(PSI+0.2);
    
    std::cout << "  For input x, the output f(x) = :\n";
    std::cout << "  f(x) = (x + 0.3 + 2ψ)(0.2 + ψ) - 2ψ - (x+0.3)ψ - 0.2ψ - ψ² + ψ - ψ(ψ+0.2)\n\n";
    
    // Simplify
    std::cout << "  Let's expand:\n";
    std::cout << "  f(x) = (x + 0.3 + 2ψ)(0.2 + ψ) - 2ψ - xψ - 0.3ψ - 0.2ψ - ψ² + ψ - ψ² - 0.2ψ\n";
    std::cout << "  f(x) = x(0.2+ψ) + (0.3+2ψ)(0.2+ψ) - xψ - 0.5ψ - 2ψ² + ψ - 0.2ψ\n";
    std::cout << "  f(x) = x(0.2+ψ-ψ) + (0.3+2ψ)(0.2+ψ) - 0.7ψ - 2ψ² + ψ\n";
    std::cout << "  f(x) = 0.2x + (0.3+2ψ)(0.2+ψ) - 0.7ψ - 2ψ² + ψ\n\n";
    
    double constant_term = (0.3+2*PSI)*(0.2+PSI) - 0.7*PSI - 2*PSI2 + PSI;
    std::cout << "  f(x) = 0.2x + " << constant_term << "\n";
    std::cout << "  f(x) = 0.2x + " << constant_term << "\n\n";
    
    // Fixed point: f(x) = x
    // 0.2x + constant = x
    // constant = 0.8x
    // x = constant / 0.8
    double fixed_point = constant_term / 0.8;
    std::cout << "  Fixed point: x = " << constant_term << " / 0.8 = " << fixed_point << "\n";
    std::cout << "  Expected fixed point from experiment: -0.697542\n\n";
    
    return 0;
}
