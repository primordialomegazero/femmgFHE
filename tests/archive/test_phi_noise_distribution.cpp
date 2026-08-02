// Test: Noise distribution across φ and ψ realities
// Theory: ψ-reality contracts noise, φ-reality expands it
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
    std::cout <<   "  ║   Noise Distribution: φ vs ψ Reality                     ║\n";
    std::cout <<   "  ║   φ expands (×1.618), ψ contracts (×0.618)               ║\n";
    std::cout <<   "  ╚══════════════════════════════════════════════════════════╝\n\n";
    
    CCParams<CryptoContextCKKSRNS> p;
    p.SetMultiplicativeDepth(30);
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
    
    // === Test: Start in φ-reality (a=1, b=0) vs ψ-reality (a=0, b=1) ===
    std::cout << "  === Noise accumulation: φ-reality vs ψ-reality ===\n";
    std::cout << "  Multiply by 1.0, then evolve 5 Fibonacci steps\n\n";
    
    // φ-reality: (1, 0) — pure φ
    PE state_phi = {cc->Encrypt(kp.publicKey, pt1), cc->Encrypt(kp.publicKey, pt1)};
    state_phi.b = cc->Encrypt(kp.publicKey, cc->MakeCKKSPackedPlaintext(std::vector<double>{0.0}));
    
    // ψ-reality: (0, 1) — pure ψ  
    PE state_psi = {cc->Encrypt(kp.publicKey, pt1), cc->Encrypt(kp.publicKey, pt1)};
    state_psi.a = cc->Encrypt(kp.publicKey, cc->MakeCKKSPackedPlaintext(std::vector<double>{0.0}));
    
    // Add noise by multiplying
    state_phi.a = cc->EvalMult(state_phi.a, ct_mul);
    state_psi.b = cc->EvalMult(state_psi.b, ct_mul);
    
    std::cout << "  Step  φ-a        φ-b        φ-err      ψ-a        ψ-b        ψ-err\n";
    std::cout << "  -------------------------------------------------------------------\n";
    
    for (int step = 0; step <= 10; step++) {
        double phi_a = get_val(cc, kp, state_phi.a);
        double phi_b = get_val(cc, kp, state_phi.b);
        double phi_err = std::abs(phi_a - 1.0) + std::abs(phi_b);
        
        double psi_a = get_val(cc, kp, state_psi.a);
        double psi_b = get_val(cc, kp, state_psi.b);
        double psi_err = std::abs(psi_a) + std::abs(psi_b - 1.0);
        
        if (step % 2 == 0 || step == 10) {
            std::cout << "  " << std::setw(3) << step
                 << "  " << std::fixed << std::setprecision(4) << phi_a
                 << "  " << std::fixed << std::setprecision(4) << phi_b
                 << "  " << std::scientific << std::setprecision(1) << phi_err
                 << "  " << std::fixed << std::setprecision(4) << psi_a
                 << "  " << std::fixed << std::setprecision(4) << psi_b
                 << "  " << std::scientific << std::setprecision(1) << psi_err
                 << "\n";
        }
        
        // Evolve both
        if (step < 10) {
            state_phi = mulY(cc, state_phi);
            state_psi = mulY(cc, state_psi);
        }
    }
    
    // === Test: Compute in ψ-reality, then convert to φ ===
    std::cout << "\n  === Compute in ψ, convert to φ ===\n";
    std::cout << "  Do 3 mults in ψ-reality, then mulY to φ-reality\n\n";
    
    PE comp_psi = {cc->Encrypt(kp.publicKey, pt1), cc->Encrypt(kp.publicKey, pt1)};
    comp_psi.a = cc->Encrypt(kp.publicKey, cc->MakeCKKSPackedPlaintext(std::vector<double>{0.0}));
    
    // Do multiplications in ψ-reality
    for (int i = 0; i < 3; i++) {
        comp_psi.b = cc->EvalMult(comp_psi.b, ct_mul);
    }
    
    double before_a = get_val(cc, kp, comp_psi.a);
    double before_b = get_val(cc, kp, comp_psi.b);
    double before_err = std::abs(before_b - 1.0);
    
    // Convert to φ-reality
    comp_psi = mulY(cc, comp_psi);
    
    double after_a = get_val(cc, kp, comp_psi.a);
    double after_b = get_val(cc, kp, comp_psi.b);
    double after_err = std::abs(after_a - 1.0);
    
    std::cout << "  Before convert: a=" << std::fixed << std::setprecision(6) << before_a
              << " b=" << before_b << " err=" << std::scientific << before_err << "\n";
    std::cout << "  After convert:  a=" << std::fixed << std::setprecision(6) << after_a
              << " b=" << after_b << " err=" << std::scientific << after_err << "\n";
    std::cout << "  Error change: " << std::fixed << std::setprecision(1) 
              << (after_err - before_err) / before_err * 100 << "%\n\n";
    
    return 0;
}
