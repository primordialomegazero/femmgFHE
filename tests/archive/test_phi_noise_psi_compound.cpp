// Test: Stay in ψ-reality for multiple steps → compound noise contraction
// Theory: Each mulY step multiplies ψ-component by ψ ≈ 0.618
//         Multiple steps → noise × 0.618^N
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
    std::cout <<   "  ║   Compound Noise Contraction in ψ-Reality                ║\n";
    std::cout <<   "  ║   Stay in ψ for N steps → noise × 0.618^N?              ║\n";
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
    auto ct_mul = cc->Encrypt(kp.publicKey, pt1);
    auto pt0 = cc->MakeCKKSPackedPlaintext(std::vector<double>{0.0});
    
    // Create INITIAL NOISE
    auto ct_noisy = cc->Encrypt(kp.publicKey, pt1);
    for (int i = 0; i < 15; i++) ct_noisy = cc->EvalMult(ct_noisy, ct_mul);
    double noise_initial = std::abs(get_val(cc, kp, ct_noisy) - 1.0);
    std::cout << "  Initial noise (after 15 mults): " << std::scientific << noise_initial << "\n\n";
    
    // Test: Vary the number of ψ-steps before converting back to φ
    std::cout << "  N_ψ    Noise after      Reduction    Predicted (0.618^N)\n";
    std::cout << "  --------------------------------------------------------\n";
    
    for (int n_steps : {1, 2, 3, 5, 8, 13, 21}) {
        // Start in φ-reality with noisy state
        PE state = {ct_noisy, cc->Encrypt(kp.publicKey, pt0)}; // (noisy, 0)
        
        // Convert to ψ-reality
        state = mulY(cc, state);  // (0, noisy+0) = (0, noisy)
        // Now in ψ: a≈0, b≈noisy (expected b=1.0)
        
        // Stay in ψ for N steps
        for (int s = 0; s < n_steps; s++) {
            state = mulY(cc, state);  // Continue in ψ
        }
        // After N steps in ψ, state.b should have multiplied by 0.618^N
        
        // Convert back to φ-reality
        state = mulY_inv(cc, state);
        // Back in φ: a should be ~1.0
        
        double final_val = get_val(cc, kp, state.a);
        double noise_final = std::abs(final_val - 1.0);
        double reduction = (noise_initial - noise_final) / noise_initial * 100;
        double predicted = std::pow(0.618, n_steps) * noise_initial;
        
        std::cout << "  " << std::setw(3) << n_steps
             << "     " << std::scientific << std::setprecision(1) << noise_final
             << "      " << std::fixed << std::setprecision(1) << reduction << "%"
             << "       " << std::scientific << std::setprecision(1) << predicted
             << "\n";
    }
    
    // === TEST 2: Stay in ψ, measure each step ===
    std::cout << "\n  === Step-by-step noise in ψ-reality ===\n";
    PE state2 = {ct_noisy, cc->Encrypt(kp.publicKey, pt0)};
    state2 = mulY(cc, state2); // to ψ
    
    std::cout << "  Step  b-value       Noise(|b-1|)  Contraction\n";
    std::cout << "  ---------------------------------------------\n";
    
    double prev_noise = std::abs(get_val(cc, kp, state2.b) - 1.0);
    
    for (int s = 0; s < 10; s++) {
        double b_val = get_val(cc, kp, state2.b);
        double noise = std::abs(b_val - 1.0);
        double contraction = (prev_noise > 1e-30) ? noise / prev_noise : 0;
        
        std::cout << "  " << std::setw(3) << s
             << "  " << std::fixed << std::setprecision(10) << b_val
             << "  " << std::scientific << std::setprecision(1) << noise
             << "  " << std::fixed << std::setprecision(4) << contraction << "x\n";
        
        prev_noise = noise;
        state2 = mulY(cc, state2);
    }
    
    std::cout << "\n";
    return 0;
}
