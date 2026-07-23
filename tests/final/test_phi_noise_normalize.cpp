// Noise extraction with Fibonacci normalization
// After N mulY steps, divide by F(N) to recover original value
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

long long fib(int n) {
    if (n <= 1) return n;
    long long a = 0, b = 1;
    for (int i = 2; i <= n; i++) { long long t = a+b; a = b; b = t; }
    return b;
}

int main() {
    std::cout << "\n  ╔══════════════════════════════════════════════════════════╗\n";
    std::cout <<   "  ║   Noise Extraction with Fibonacci Normalization          ║\n";
    std::cout <<   "  ║   Divide by F(N) to recover original value               ║\n";
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
    
    // Create noisy state
    auto ct_noisy = cc->Encrypt(kp.publicKey, pt1);
    for (int i = 0; i < 25; i++) ct_noisy = cc->EvalMult(ct_noisy, ct_mul);
    double noise_before = std::abs(get_val(cc, kp, ct_noisy) - 1.0);
    
    std::cout << "  Noise before: " << std::scientific << noise_before << "\n\n";
    
    // Drive toward ψ-attractor
    PE state = {ct_noisy, cc->Encrypt(kp.publicKey, pt1)};
    const int STEPS = 10;
    for (int i = 0; i < STEPS; i++) state = mulY(cc, state);
    
    double a_val = get_val(cc, kp, state.a);
    double b_val = get_val(cc, kp, state.b);
    
    // Starting from (1,1): after N steps, a = F(N+1), b = F(N+2)
    long long f_n1 = fib(STEPS + 1); // F(11) = 89
    long long f_n2 = fib(STEPS + 2); // F(12) = 144
    
    std::cout << "  After " << STEPS << " mulY steps:\n";
    std::cout << "  a = " << std::fixed << std::setprecision(4) << a_val << " (expected F(" << STEPS+1 << ") = " << f_n1 << ")\n";
    std::cout << "  b = " << std::fixed << std::setprecision(4) << b_val << " (expected F(" << STEPS+2 << ") = " << f_n2 << ")\n\n";
    
    // Recover original value by dividing by Fibonacci number
    double recovered = b_val / f_n2;
    double noise_after = std::abs(recovered - 1.0);
    
    std::cout << "  === Recovery by normalization ===\n";
    std::cout << "  Recovered = b / F(" << STEPS+2 << ") = " << std::fixed << std::setprecision(10) << recovered << "\n";
    std::cout << "  Noise after: " << std::scientific << noise_after << "\n";
    std::cout << "  Reduction: " << std::fixed << std::setprecision(1) << (noise_before-noise_after)/noise_before*100 << "%\n\n";
    
    // === METHOD 2: Use ratio directly ===
    // ratio = a/b → ψ, so value = b - a (if we started from (1,1))
    // Actually: (a,b) = (F(N)*orig, F(N+1)*orig) → orig = a/F(N) = b/F(N+1)
    double recovered2 = a_val / f_n1;
    double noise_after2 = std::abs(recovered2 - 1.0);
    
    std::cout << "  === Alternative: a / F(" << STEPS+1 << ") ===\n";
    std::cout << "  Recovered = " << std::fixed << std::setprecision(10) << recovered2 << "\n";
    std::cout << "  Noise after: " << std::scientific << noise_after2 << "\n";
    std::cout << "  Reduction: " << std::fixed << std::setprecision(1) << (noise_before-noise_after2)/noise_before*100 << "%\n\n";
    
    return 0;
}
