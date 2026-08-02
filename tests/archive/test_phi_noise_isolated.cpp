// Isolated noise measurement per conversion
// Track expected value through conversions
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
    std::cout <<   "  ║   Isolated Noise Test: Track Expected Value              ║\n";
    std::cout <<   "  ║   Measure pure noise as |actual - expected|               ║\n";
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
    
    // Create noisy state
    auto ct_noisy = cc->Encrypt(kp.publicKey, pt1);
    for (int i = 0; i < 15; i++) ct_noisy = cc->EvalMult(ct_noisy, ct_mul);
    
    // START: φ-reality, state = (noisy, 1.0)
    // Expected: a=1.0, b=1.0 (after mulY from ψ) or a=1.0, b=0.0 (pure φ)
    // Let's use pure φ: (1, 0)
    PE state = {ct_noisy, cc->Encrypt(kp.publicKey, pt0)}; // (noisy, 0)
    
    std::cout << "  Initial: φ-reality (a=1.0 expected, b=0.0 expected)\n";
    double a0 = get_val(cc, kp, state.a);
    double b0 = get_val(cc, kp, state.b);
    std::cout << "    a=" << std::fixed << std::setprecision(10) << a0 
              << " (noise=" << std::scientific << std::abs(a0-1.0) << ")\n";
    std::cout << "    b=" << std::fixed << std::setprecision(10) << b0 
              << " (noise=" << std::scientific << std::abs(b0-0.0) << ")\n\n";
    
    // Now do ONLY conversions (no EvalMult) and track expected values
    std::cout << "  Conv  Reality  Active val  Expected  Pure Noise    Reduction\n";
    std::cout << "  ------------------------------------------------------------\n";
    
    double prev_noise = std::abs(a0 - 1.0);
    bool in_phi = true;
    
    for (int c = 0; c < 10; c++) {
        // Convert
        if (in_phi) {
            state = mulY(cc, state);       // φ → ψ: (b, a+b) = (0, noisy+0) ≈ (0, 1.0)
        } else {
            state = mulY_inv(cc, state);   // ψ → φ: (b-a, a) = (1.0-0, 0) ≈ (1.0, 0)
        }
        in_phi = !in_phi;
        
        double active_val, expected_val;
        if (in_phi) {
            active_val = get_val(cc, kp, state.a);
            expected_val = 1.0;
        } else {
            active_val = get_val(cc, kp, state.b);
            expected_val = 1.0;
        }
        
        double pure_noise = std::abs(active_val - expected_val);
        double reduction = (prev_noise > 1e-30) ? (prev_noise - pure_noise) / prev_noise * 100 : 0;
        
        std::cout << "  " << std::setw(3) << c
             << "     " << (in_phi ? "φ" : "ψ") << "      "
             << std::fixed << std::setprecision(10) << active_val
             << "  " << std::fixed << std::setprecision(4) << expected_val
             << "  " << std::scientific << std::setprecision(1) << pure_noise
             << "  " << std::fixed << std::setprecision(1) << reduction << "%\n";
        
        prev_noise = pure_noise;
    }
    
    std::cout << "\n";
    return 0;
}
