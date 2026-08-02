// Correct noise measurement: track Fibonacci expected values
#include <iostream>
#include <iomanip>
#include <cmath>
#include "openfhe.h"
using namespace lbcrypto;

struct PE { Ciphertext<DCRTPoly> a; Ciphertext<DCRTPoly> b; };

PE mulY(CryptoContext<DCRTPoly>& cc, const PE& x) {
    return {x.b, cc->EvalAdd(x.a, x.b)};
}

double get_val(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, const Ciphertext<DCRTPoly>& c) {
    Plaintext pt;
    cc->Decrypt(kp.secretKey, c, &pt);
    return pt->GetCKKSPackedValue()[0].real();
}

// Fibonacci sequence
long long fib(int n) {
    if (n <= 1) return n;
    long long a = 0, b = 1;
    for (int i = 2; i <= n; i++) {
        long long t = a + b; a = b; b = t;
    }
    return b;
}

int main() {
    std::cout << "\n  ╔══════════════════════════════════════════════════════════╗\n";
    std::cout <<   "  ║   Correct Noise: Track Fibonacci Expected Values         ║\n";
    std::cout <<   "  ║   Noise = |actual - Fibonacci(expected)|                 ║\n";
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
    
    // Create noisy state
    auto ct_noisy = cc->Encrypt(kp.publicKey, pt1);
    auto ct_mul = cc->Encrypt(kp.publicKey, pt1);
    for (int i = 0; i < 15; i++) ct_noisy = cc->EvalMult(ct_noisy, ct_mul);
    
    double noise_start = std::abs(get_val(cc, kp, ct_noisy) - 1.0);
    std::cout << "  Initial noise (15 mults): " << std::scientific << noise_start << "\n\n";
    
    // Start in ψ-reality: (0, noisy)
    PE state = {cc->Encrypt(kp.publicKey, pt0), ct_noisy};
    // Current state: a=0, b≈1 (with noise)
    
    std::cout << "  Step  Expected(b)  Actual(b)     Pure Noise    Contraction\n";
    std::cout << "  ---------------------------------------------------------\n";
    
    double prev_noise = std::abs(get_val(cc, kp, state.b) - 1.0); // b should be F(2)=1
    
    for (int step = 0; step < 8; step++) {
        double actual_b = get_val(cc, kp, state.b);
        long long expected_b = fib(step + 2); // b follows Fibonacci: F(2), F(3), F(4)...
        double pure_noise = std::abs(actual_b - expected_b);
        double contraction = (prev_noise > 1e-30) ? pure_noise / prev_noise : 0;
        
        std::cout << "  " << std::setw(3) << step
             << "  " << std::setw(10) << expected_b
             << "  " << std::fixed << std::setprecision(10) << actual_b
             << "  " << std::scientific << std::setprecision(1) << pure_noise
             << "  " << std::fixed << std::setprecision(4) << contraction << "x\n";
        
        prev_noise = pure_noise;
        
        if (step < 7) state = mulY(cc, state);
    }
    
    std::cout << "\n  (If contraction < 1.0, noise is shrinking in ψ-reality!)\n\n";
    return 0;
}
