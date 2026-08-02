// Test: Multi-stage φ↔ψ cascade for 99.9% noise reduction
// Theory: Each conversion reduces noise ~70%, cascade compounds
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
    std::cout <<   "  ║   99.9% Noise Reduction via Multi-Stage Cascade           ║\n";
    std::cout <<   "  ║   How many φ↔ψ conversions to reach 99.9%?               ║\n";
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
    
    // Create INITIAL NOISE — simulate a deep computation
    std::cout << "  Creating initial noise (20 multiplications)...\n";
    auto ct_noisy = cc->Encrypt(kp.publicKey, pt1);
    for (int i = 0; i < 20; i++) {
        ct_noisy = cc->EvalMult(ct_noisy, ct_mul);
    }
    double noise_start = std::abs(get_val(cc, kp, ct_noisy) - 1.0);
    std::cout << "  Initial noise: " << std::scientific << noise_start << "\n\n";
    
    // Now apply cascade of φ↔ψ conversions
    // Strategy: start in φ-reality, convert back and forth
    PE state = {ct_noisy, cc->Encrypt(kp.publicKey, pt1)}; // (noisy, 1.0) — φ-reality
    bool in_phi = true;
    
    std::cout << "  Stage  Reality  Value              Error          Reduction  Cumulative\n";
    std::cout << "  ----------------------------------------------------------------------\n";
    
    double current_noise = noise_start;
    double cumulative_reduction = 0;
    
    for (int stage = 0; stage < 12; stage++) {
        // Measure in current reality
        double val;
        if (in_phi) {
            val = get_val(cc, kp, state.a);
        } else {
            val = get_val(cc, kp, state.b);
        }
        double error = std::abs(val - 1.0);
        double stage_reduction = (current_noise - error) / current_noise * 100;
        cumulative_reduction = (noise_start - error) / noise_start * 100;
        
        std::cout << "  " << std::setw(3) << stage
             << "     " << (in_phi ? "φ" : "ψ") << "      "
             << std::fixed << std::setprecision(10) << val
             << "  " << std::scientific << std::setprecision(1) << error
             << "  " << std::fixed << std::setprecision(1) << stage_reduction << "%"
             << "      " << std::fixed << std::setprecision(1) << cumulative_reduction << "%\n";
        
        current_noise = error;
        
        // Check if we've reached 99.9%
        if (cumulative_reduction >= 99.9) {
            std::cout << "\n  🏆 99.9% NOISE REDUCTION ACHIEVED at stage " << stage << "!\n";
            break;
        }
        
        // Convert to other reality
        if (in_phi) {
            state = mulY(cc, state);       // φ → ψ
        } else {
            state = mulY_inv(cc, state);   // ψ → φ
        }
        in_phi = !in_phi;
    }
    
    // === BONUS: Add clean_reverse into the cascade ===
    std::cout << "\n  === BONUS: Cascade with clean_reverse (58% φ-reset) ===\n";
    
    // Reset state
    auto ct_noisy2 = cc->Encrypt(kp.publicKey, pt1);
    for (int i = 0; i < 20; i++) ct_noisy2 = cc->EvalMult(ct_noisy2, ct_mul);
    double noise2_start = std::abs(get_val(cc, kp, ct_noisy2) - 1.0);
    
    PE state2 = {ct_noisy2, cc->Encrypt(kp.publicKey, pt1)};
    
    // Apply clean_reverse (2a-b, -a+b) then mulY cascade
    auto a2 = cc->EvalAdd(state2.a, state2.a);
    state2.a = cc->EvalSub(a2, state2.b);
    state2.b = cc->EvalSub(state2.b, state2.a); // This is clean_reverse
    // Actually let's do it properly:
    // clean_rev(a,b) = (2a-b, -a+b)
    auto two_a = cc->EvalAdd(ct_noisy2, ct_noisy2);
    auto rev_a = cc->EvalSub(two_a, cc->Encrypt(kp.publicKey, pt1));
    auto rev_b = cc->EvalSub(cc->Encrypt(kp.publicKey, pt1), ct_noisy2);
    state2 = {rev_a, rev_b};
    
    // Now do 3 conversions
    in_phi = true;
    for (int s = 0; s < 3; s++) {
        if (in_phi) state2 = mulY(cc, state2);
        else state2 = mulY_inv(cc, state2);
        in_phi = !in_phi;
    }
    
    double val2 = (in_phi) ? get_val(cc, kp, state2.a) : get_val(cc, kp, state2.b);
    double error2 = std::abs(val2 - 1.0);
    double reduction2 = (noise2_start - error2) / noise2_start * 100;
    
    std::cout << "  After clean_reverse + 3 conversions:\n";
    std::cout << "    Initial noise: " << std::scientific << noise2_start << "\n";
    std::cout << "    Final error:   " << std::scientific << error2 << "\n";
    std::cout << "    Total reduction: " << std::fixed << std::setprecision(1) << reduction2 << "%\n\n";
    
    return 0;
}
