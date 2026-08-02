// Noise Pump: Repeatedly push noise into ψ, pull signal into φ
// Measure noise decay in φ-component over multiple cycles
#include <iostream>
#include <iomanip>
#include <cmath>
#include "openfhe.h"
using namespace lbcrypto;

struct PE { Ciphertext<DCRTPoly> a; Ciphertext<DCRTPoly> b; };

PE clean_fwd(CryptoContext<DCRTPoly>& cc, const PE& x) {
    auto sum = cc->EvalAdd(x.a, x.b);
    return {sum, cc->EvalAdd(x.a, sum)};
}
PE clean_rev(CryptoContext<DCRTPoly>& cc, const PE& x) {
    auto a2 = cc->EvalAdd(x.a, x.a);
    return {cc->EvalSub(a2, x.b), cc->EvalSub(x.b, x.a)};
}

double get_val(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, const Ciphertext<DCRTPoly>& c) {
    Plaintext pt;
    cc->Decrypt(kp.secretKey, c, &pt);
    return pt->GetCKKSPackedValue()[0].real();
}

int main() {
    std::cout << "\n  ╔══════════════════════════════════════════════════════════╗\n";
    std::cout <<   "  ║   Noise Pump: Push→Pull Cycle                             ║\n";
    std::cout <<   "  ║   Push noise to ψ, fade via attractor, measure φ          ║\n";
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
    
    auto pt1 = cc->MakeCKKSPackedPlaintext(std::vector<double>{1.0});
    auto ct_mul = cc->Encrypt(kp.publicKey, pt1);
    
    // Create HEAVY noise — 30 multiplications
    auto ct_noisy = cc->Encrypt(kp.publicKey, pt1);
    for (int i = 0; i < 30; i++) ct_noisy = cc->EvalMult(ct_noisy, ct_mul);
    
    double noise_start = std::abs(get_val(cc, kp, ct_noisy) - 1.0);
    std::cout << "  Initial noise (30 mults): " << std::scientific << noise_start << "\n\n";
    
    // Start: φ-reality, (noisy, 0)
    PE state = {ct_noisy, cc->Encrypt(kp.publicKey, 
                cc->MakeCKKSPackedPlaintext(std::vector<double>{0.0}))};
    
    std::cout << "  Cycle  φ-value              φ-noise(|a-1|)  Reduction  Cumulative\n";
    std::cout << "  -----------------------------------------------------------------\n";
    
    double prev_noise = noise_start;
    
    for (int cycle = 0; cycle < 20; cycle++) {
        // Push: clean_rev pushes noise to ψ, pulls signal to φ
        state = clean_rev(cc, state);
        // Pull: clean_fwd drives ψ toward attractor (noise fades)
        state = clean_fwd(cc, state);
        
        // Measure noise in φ-component
        double a_val = get_val(cc, kp, state.a);
        double noise = std::abs(a_val - 1.0);
        double reduction = (prev_noise > 1e-30) ? (prev_noise - noise)/prev_noise*100 : 0;
        double cumulative = (noise_start > 1e-30) ? (noise_start - noise)/noise_start*100 : 0;
        
        if (cycle < 10 || cycle % 5 == 0) {
            std::cout << "  " << std::setw(3) << cycle
                 << "  " << std::fixed << std::setprecision(10) << a_val
                 << "  " << std::scientific << std::setprecision(1) << noise
                 << "  " << std::fixed << std::setprecision(1) << reduction << "%"
                 << "     " << std::fixed << std::setprecision(1) << cumulative << "%\n";
        }
        
        prev_noise = noise;
        
        if (cumulative >= 99.9) {
            std::cout << "\n  🏆 99.9% NOISE REDUCTION at cycle " << cycle << "!\n";
            break;
        }
    }
    
    std::cout << "\n";
    return 0;
}
