// Clean noise test: Compare error with vs without φ↔ψ conversions
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
    std::cout <<   "  ║   Clean Noise Test: φ↔ψ Conversion Effect                ║\n";
    std::cout <<   "  ║   Compare error: Mults only vs Mults + Conversions       ║\n";
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
    auto pt0 = cc->MakeCKKSPackedPlaintext(std::vector<double>{0.0});
    
    // === PATH A: 15 EvalMults, NO conversions ===
    std::cout << "  === PATH A: 15 EvalMults, NO conversions ===\n";
    auto ctA = cc->Encrypt(kp.publicKey, pt1);
    for (int i = 0; i < 15; i++) {
        ctA = cc->EvalMult(ctA, ct_mul);
    }
    double errA = std::abs(get_val(cc, kp, ctA) - 1.0);
    std::cout << "  Error after 15 mults: " << std::scientific << errA << "\n\n";
    
    // === PATH B: 15 EvalMults WITH conversions every 3 mults ===
    std::cout << "  === PATH B: 15 EvalMults, convert every 3 mults ===\n";
    PE stateB = {cc->Encrypt(kp.publicKey, pt1), cc->Encrypt(kp.publicKey, pt0)};
    bool in_phi = true;
    
    for (int block = 0; block < 5; block++) {
        // 3 multiplications
        for (int m = 0; m < 3; m++) {
            if (in_phi) {
                stateB.a = cc->EvalMult(stateB.a, ct_mul);
            } else {
                stateB.b = cc->EvalMult(stateB.b, ct_mul);
            }
        }
        // Convert
        if (in_phi) {
            stateB = mulY(cc, stateB);       // φ → ψ
        } else {
            stateB = mulY_inv(cc, stateB);   // ψ → φ
        }
        in_phi = !in_phi;
    }
    double valB = (in_phi) ? get_val(cc, kp, stateB.a) : get_val(cc, kp, stateB.b);
    double errB = std::abs(valB - 1.0);
    std::cout << "  Error after 15 mults + conversions: " << std::scientific << errB << "\n";
    std::cout << "  Error ratio (B/A): " << std::fixed << std::setprecision(2) << errB/errA << "x\n\n";
    
    // === PATH C: Start with noise, pure conversions, measure decay ===
    std::cout << "  === PATH C: Pure conversions on noisy state ===\n";
    // Create noisy state
    auto ct_noisy = cc->Encrypt(kp.publicKey, pt1);
    for (int i = 0; i < 10; i++) ct_noisy = cc->EvalMult(ct_noisy, ct_mul);
    double err_start = std::abs(get_val(cc, kp, ct_noisy) - 1.0);
    
    PE stateC = {ct_noisy, cc->Encrypt(kp.publicKey, pt1)};
    bool c_in_phi = true;
    
    std::cout << "  Starting error: " << std::scientific << err_start << "\n";
    std::cout << "  Conv   Active val   Error        vs start\n";
    std::cout << "  ------------------------------------------\n";
    
    for (int c = 0; c < 8; c++) {
        double active_val;
        if (c_in_phi) {
            active_val = get_val(cc, kp, stateC.a);
        } else {
            active_val = get_val(cc, kp, stateC.b);
        }
        double err = std::abs(active_val - 1.0);
        
        std::cout << "  " << std::setw(3) << c
             << "     " << (c_in_phi ? "φ" : "ψ") << "         "
             << std::fixed << std::setprecision(10) << active_val
             << "  " << std::scientific << std::setprecision(1) << err
             << "  " << std::fixed << std::setprecision(2) << err/err_start << "x\n";
        
        if (c_in_phi) {
            stateC = mulY(cc, stateC);       // φ → ψ
        } else {
            stateC = mulY_inv(cc, stateC);   // ψ → φ
        }
        c_in_phi = !c_in_phi;
    }
    
    std::cout << "\n";
    return 0;
}
