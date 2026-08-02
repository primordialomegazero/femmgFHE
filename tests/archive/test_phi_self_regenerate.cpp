// Self-Regenerating Noise Reduction via Golden Ratio Breathing
// Cycle: φ → ψ → φ → ψ → φ, noise trapped in ψ, signal regenerated in φ
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

// Forward clean: (a+b, a+2b) — drives ψ toward attractor
PE clean_fwd(CryptoContext<DCRTPoly>& cc, const PE& x) {
    auto sum = cc->EvalAdd(x.a, x.b);
    return {sum, cc->EvalAdd(x.a, sum)};
}

// Reverse clean: (2a-b, -a+b) — shrinks φ by 58%
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
    std::cout <<   "  ║   Self-Regenerating Noise Reduction                       ║\n";
    std::cout <<   "  ║   Golden Ratio Breathing Cycle                            ║\n";
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
    
    // Create INITIAL NOISE — simulate heavy computation
    auto ct_noisy = cc->Encrypt(kp.publicKey, pt1);
    auto ct_mul = cc->Encrypt(kp.publicKey, pt1);
    for (int i = 0; i < 20; i++) ct_noisy = cc->EvalMult(ct_noisy, ct_mul);
    
    double noise_initial = std::abs(get_val(cc, kp, ct_noisy) - 1.0);
    std::cout << "  Initial noise (20 mults): " << std::scientific << noise_initial << "\n\n";
    
    // ===== BREATHING CYCLE =====
    // Start: (noisy, 1.0) — φ-reality, signal in a
    PE state = {ct_noisy, cc->Encrypt(kp.publicKey, pt1)};
    
    std::cout << "  Cycle  Operation        a-val       b-val       Noise(|a-1|)  Reduction\n";
    std::cout << "  ----------------------------------------------------------------------\n";
    
    double current_noise = noise_initial;
    
    // Measure initial
    double a0 = get_val(cc, kp, state.a);
    double n0 = std::abs(a0 - 1.0);
    std::cout << "  start  —               " << std::fixed << std::setprecision(4) << a0
              << "  " << std::fixed << std::setprecision(4) << get_val(cc, kp, state.b)
              << "  " << std::scientific << std::setprecision(1) << n0
              << "  —\n";
    
    // Cycle 1: REVERSE CLEAN (shrink φ by 58%)
    state = clean_rev(cc, state);
    double a1 = get_val(cc, kp, state.a);
    double n1 = std::abs(a1 - 1.0);
    double r1 = (current_noise > 1e-30) ? (current_noise - n1)/current_noise*100 : 0;
    std::cout << "    1    clean_rev        " << std::fixed << std::setprecision(4) << a1
              << "  " << std::fixed << std::setprecision(4) << get_val(cc, kp, state.b)
              << "  " << std::scientific << std::setprecision(1) << n1
              << "  " << std::fixed << std::setprecision(1) << r1 << "%\n";
    current_noise = n1;
    
    // Cycle 2: FORWARD CLEAN (drive ψ toward attractor)
    state = clean_fwd(cc, state);
    double a2 = get_val(cc, kp, state.a);
    double n2 = std::abs(a2 - 1.0);
    double r2 = (current_noise > 1e-30) ? (current_noise - n2)/current_noise*100 : 0;
    std::cout << "    2    clean_fwd        " << std::fixed << std::setprecision(4) << a2
              << "  " << std::fixed << std::setprecision(4) << get_val(cc, kp, state.b)
              << "  " << std::scientific << std::setprecision(1) << n2
              << "  " << std::fixed << std::setprecision(1) << r2 << "%\n";
    current_noise = n2;
    
    // Cycle 3: MULY (shift reality)
    state = mulY(cc, state);
    double a3 = get_val(cc, kp, state.a);
    double n3 = std::abs(a3 - 1.0);
    double r3 = (current_noise > 1e-30) ? (current_noise - n3)/current_noise*100 : 0;
    std::cout << "    3    mulY (φ→ψ)       " << std::fixed << std::setprecision(4) << a3
              << "  " << std::fixed << std::setprecision(4) << get_val(cc, kp, state.b)
              << "  " << std::scientific << std::setprecision(1) << n3
              << "  " << std::fixed << std::setprecision(1) << r3 << "%\n";
    current_noise = n3;
    
    // Cycle 4: MULY_INV (shift back)
    state = mulY_inv(cc, state);
    double a4 = get_val(cc, kp, state.a);
    double n4 = std::abs(a4 - 1.0);
    double r4 = (current_noise > 1e-30) ? (current_noise - n4)/current_noise*100 : 0;
    std::cout << "    4    mulY_inv (ψ→φ)   " << std::fixed << std::setprecision(4) << a4
              << "  " << std::fixed << std::setprecision(4) << get_val(cc, kp, state.b)
              << "  " << std::scientific << std::setprecision(1) << n4
              << "  " << std::fixed << std::setprecision(1) << r4 << "%\n";
    current_noise = n4;
    
    // Cycle 5: CLEAN_REV again
    state = clean_rev(cc, state);
    double a5 = get_val(cc, kp, state.a);
    double n5 = std::abs(a5 - 1.0);
    double r5 = (current_noise > 1e-30) ? (current_noise - n5)/current_noise*100 : 0;
    std::cout << "    5    clean_rev        " << std::fixed << std::setprecision(4) << a5
              << "  " << std::fixed << std::setprecision(4) << get_val(cc, kp, state.b)
              << "  " << std::scientific << std::setprecision(1) << n5
              << "  " << std::fixed << std::setprecision(1) << r5 << "%\n";
    current_noise = n5;
    
    // Cycle 6: CLEAN_FWD again
    state = clean_fwd(cc, state);
    double a6 = get_val(cc, kp, state.a);
    double n6 = std::abs(a6 - 1.0);
    double r6 = (current_noise > 1e-30) ? (current_noise - n6)/current_noise*100 : 0;
    std::cout << "    6    clean_fwd        " << std::fixed << std::setprecision(4) << a6
              << "  " << std::fixed << std::setprecision(4) << get_val(cc, kp, state.b)
              << "  " << std::scientific << std::setprecision(1) << n6
              << "  " << std::fixed << std::setprecision(1) << r6 << "%\n";
    
    double total_reduction = (noise_initial - n6) / noise_initial * 100;
    std::cout << "\n  Total reduction: " << std::fixed << std::setprecision(1) << total_reduction << "%\n\n";
    
    return 0;
}
