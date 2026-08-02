// Extract and cancel noise using ψ-attractor as clean reference
// 1. Drive to ψ-attractor to get clean ratio
// 2. Compare with noisy original to extract noise
// 3. Subtract noise from original
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
    std::cout <<   "  ║   Noise Extraction via ψ-Attractor Reference             ║\n";
    std::cout <<   "  ║   Extract noise, then cancel it from original            ║\n";
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
    
    // Create HEAVILY noisy state
    auto ct_noisy = cc->Encrypt(kp.publicKey, pt1);
    for (int i = 0; i < 25; i++) ct_noisy = cc->EvalMult(ct_noisy, ct_mul);
    
    double noise_before = std::abs(get_val(cc, kp, ct_noisy) - 1.0);
    std::cout << "  Noise before: " << std::scientific << noise_before << "\n\n";
    
    // === METHOD: Use ψ-attractor as clean reference ===
    std::cout << "  === Step 1: Drive noisy state to ψ-attractor ===\n";
    PE state = {ct_noisy, cc->Encrypt(kp.publicKey, pt1)}; // (noisy, 1)
    
    // Drive toward ψ: after ~10 steps, ratio ≈ 0.618
    for (int i = 0; i < 10; i++) state = mulY(cc, state);
    
    double a_att = get_val(cc, kp, state.a);
    double b_att = get_val(cc, kp, state.b);
    double ratio = a_att / b_att;
    std::cout << "  After 10 mulY: a=" << std::fixed << std::setprecision(4) << a_att
              << " b=" << b_att << " ratio=" << std::fixed << std::setprecision(6) << ratio << "\n\n";
    
    // === Step 2: The noise is encoded in the deviation from expected ratio ===
    // Expected: a/b = ψ ≈ 0.618, but actual is different due to noise
    // Noise in ratio = actual_ratio - ψ
    double psi = 0.6180339887498949;
    double ratio_noise = ratio - psi;
    std::cout << "  === Step 2: Measure noise in ratio ===\n";
    std::cout << "  Ratio deviation from ψ: " << std::scientific << ratio_noise << "\n\n";
    
    // === Step 3: Correct the ratio back to ψ ===
    // The clean value should be: b * (1 - ψ) where b is the clean component
    // Actually the value v = b - a (since ratio = a/b ≈ ψ, v = b - ψb = b(1-ψ))
    double recovered_val = b_att - a_att; // b - a with ψ-attractor ratio
    std::cout << "  === Step 3: Recover clean value ===\n";
    std::cout << "  Recovered value (b-a): " << std::fixed << std::setprecision(10) << recovered_val << "\n";
    std::cout << "  Expected value: 1.0\n";
    std::cout << "  Error: " << std::scientific << std::abs(recovered_val - 1.0) << "\n\n";
    
    // === Step 4: Compare with original noise ===
    std::cout << "  === Step 4: Noise reduction ===\n";
    double noise_after = std::abs(recovered_val - 1.0);
    double reduction = (noise_before - noise_after) / noise_before * 100;
    std::cout << "  Before: " << std::scientific << noise_before << "\n";
    std::cout << "  After:  " << std::scientific << noise_after << "\n";
    std::cout << "  Reduction: " << std::fixed << std::setprecision(1) << reduction << "%\n\n";
    
    return 0;
}
