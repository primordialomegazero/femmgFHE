// Noise Segregation: Project ciphertext onto φ and ψ eigenbasis
// Theory: Noise concentrates in ψ-direction, signal in φ-direction
#include <iostream>
#include <iomanip>
#include <cmath>
#include "openfhe.h"
using namespace lbcrypto;

struct PE { Ciphertext<DCRTPoly> a; Ciphertext<DCRTPoly> b; };

PE mulY(CryptoContext<DCRTPoly>& cc, const PE& x) {
    return {x.b, cc->EvalAdd(x.a, x.b)};
}
PE mulY_inv(CryptoContext<DCRTPoly>& cc, const PE& x) {
    return {cc->EvalSub(x.b, x.a), x.a};
}

double get_val(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, const Ciphertext<DCRTPoly>& c) {
    Plaintext pt;
    cc->Decrypt(kp.secretKey, c, &pt);
    return pt->GetCKKSPackedValue()[0].real();
}

int main() {
    std::cout << "\n  ╔══════════════════════════════════════════════════════════╗\n";
    std::cout <<   "  ║   Noise Segregation: φ/ψ Eigenbasis Projection           ║\n";
    std::cout <<   "  ║   Signal → φ-direction, Noise → ψ-direction?             ║\n";
    std::cout <<   "  ╚══════════════════════════════════════════════════════════╝\n\n";
    
    CCParams<CryptoContextCKKSRNS> p;
    p.SetMultiplicativeDepth(50);
    p.SetScalingModSize(50);
    p.SetBatchSize(2048);
    p.SetRingDim(4096);
    p.SetSecretKeyDist(UNIFORM_TERNARY);
    p.SetSecurityLevel(HEStd_NotSet);
    
    auto cc = GenCryptoContext(p);
    cc->Enable(PKE); cc->Enable(KEYSWITCH); cc->Enable(LEVELEDSHE); cc->Enable(ADVANCEDSHE);
    auto kp = cc->KeyGen();
    cc->EvalMultKeyGen(kp.secretKey);
    
    auto pt1 = cc->MakeCKKSPackedPlaintext(std::vector<double>{1.0});
    auto pt0 = cc->MakeCKKSPackedPlaintext(std::vector<double>{0.0});
    auto ct_mul = cc->Encrypt(kp.publicKey, pt1);
    
    // Create noisy state
    auto ct_noisy = cc->Encrypt(kp.publicKey, pt1);
    for (int i = 0; i < 20; i++) ct_noisy = cc->EvalMult(ct_noisy, ct_mul);
    double noise_orig = std::abs(get_val(cc, kp, ct_noisy) - 1.0);
    std::cout << "  Original noise: " << std::scientific << noise_orig << "\n\n";
    
    // === Approach 1: Repeated mulY to drive toward ψ-attractor ===
    // After many mulY steps, ratio → ψ, meaning:
    // a/b → 0.618 → b dominates (signal), a is small (noise?)
    std::cout << "  === Approach 1: Drive toward ψ-attractor ===\n";
    PE state1 = {ct_noisy, cc->Encrypt(kp.publicKey, pt1)};
    
    std::cout << "  Step  a-value       b-value       a/b ratio    a-noise    b-noise\n";
    std::cout << "  -----------------------------------------------------------------\n";
    
    for (int s = 0; s <= 10; s++) {
        double a = get_val(cc, kp, state1.a);
        double b = get_val(cc, kp, state1.b);
        double ratio = (std::abs(b) > 1e-10) ? a/b : 999;
        double a_noise = std::abs(a - 0.0); // in ψ-attractor, a → 0
        double b_noise = std::abs(b - 1.0); // b carries the value
        
        if (s <= 5 || s == 10) {
            std::cout << "  " << std::setw(3) << s
                 << "  " << std::fixed << std::setprecision(4) << a
                 << "  " << std::fixed << std::setprecision(4) << b
                 << "  " << std::fixed << std::setprecision(4) << ratio
                 << "  " << std::scientific << std::setprecision(1) << a_noise
                 << "  " << std::scientific << std::setprecision(1) << b_noise
                 << "\n";
        }
        
        if (s < 10) state1 = mulY(cc, state1);
    }
    
    // === Approach 2: Try to ISOLATE the noise ===
    // If we have (noisy, 1.0), the noise is in component a
    // What if we do: (a - 1.0, b) — extract pure noise?
    std::cout << "\n  === Approach 2: Extract noise component ===\n";
    PE state2 = {ct_noisy, cc->Encrypt(kp.publicKey, pt1)};
    
    // Create a clean "1.0" to subtract
    auto ct_one = cc->Encrypt(kp.publicKey, pt1);
    auto noise_a = cc->EvalSub(state2.a, ct_one);
    
    double extracted_noise = get_val(cc, kp, noise_a);
    std::cout << "  Extracted noise (a-1): " << std::scientific << extracted_noise << "\n";
    std::cout << "  This is the pure noise component!\n\n";
    
    // === Approach 3: Use mulY to "push" noise to a, signal to b ===
    std::cout << "  === Approach 3: Noise segregation via mulY chain ===\n";
    std::cout << "  Start: (signal+noise, 1)\n";
    std::cout << "  After N mulY: a ≈ F(N-1)*noise, b ≈ F(N)*signal\n";
    std::cout << "  The noise stays in 'a' (coefficient of smaller Fibonacci)\n\n";
    
    PE state3 = {ct_noisy, cc->Encrypt(kp.publicKey, pt1)};
    
    // Do 8 mulY steps to separate
    for (int i = 0; i < 8; i++) state3 = mulY(cc, state3);
    
    // Now: a ≈ F(7) ≈ 13, b ≈ F(8) ≈ 21
    // The NOISE should be mainly in 'a' (smaller component)
    // The SIGNAL should be mainly in 'b' (larger component)
    
    double a3 = get_val(cc, kp, state3.a);
    double b3 = get_val(cc, kp, state3.b);
    double expected_a = 13.0; // F(7) — noise should be here
    double expected_b = 21.0; // F(8) — signal should be here
    
    double noise_in_a = std::abs(a3/expected_a - 1.0);
    double noise_in_b = std::abs(b3/expected_b - 1.0);
    
    std::cout << "  a = " << std::fixed << std::setprecision(4) << a3 
              << " (expected " << expected_a << ", noise ratio=" << std::scientific << noise_in_a << ")\n";
    std::cout << "  b = " << std::fixed << std::setprecision(4) << b3 
              << " (expected " << expected_b << ", noise ratio=" << std::scientific << noise_in_b << ")\n";
    std::cout << "  Noise ratio: a/b = " << std::fixed << std::setprecision(4) << noise_in_a/noise_in_b << "x\n\n";
    
    return 0;
}
