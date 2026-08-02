// Test: Does ring swap reduce ciphertext noise?
// Measure decryption error before and after ring swap
#include <iostream>
#include <iomanip>
#include <cmath>
#include <vector>
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
    std::cout <<   "  ║   Noise Refresh Experiment                               ║\n";
    std::cout <<   "  ║   Measure decryption error with/without ring swap        ║\n";
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
    
    // === TEST 1: Noise growth WITHOUT ring swap ===
    std::cout << "  === TEST 1: Noise growth WITHOUT ring swap ===\n";
    std::cout << "  Multiply by 1.0 repeatedly (value stays 1.0, noise grows)\n\n";
    
    auto pt1 = cc->MakeCKKSPackedPlaintext(std::vector<double>{1.0});
    auto ct = cc->Encrypt(kp.publicKey, pt1);
    auto ct_mult = cc->Encrypt(kp.publicKey, pt1);
    
    std::cout << "  Mults   Decrypted Value    Error from 1.0    Level\n";
    std::cout << "  ---------------------------------------------------\n";
    
    for (int i = 0; i <= 25; i += 5) {
        Plaintext result;
        cc->Decrypt(kp.secretKey, ct, &result);
        double val = result->GetCKKSPackedValue()[0].real();
        double err = std::abs(val - 1.0);
        int lvl = ct->GetLevel();
        
        std::cout << "  " << std::setw(3) << i
             << "    " << std::fixed << std::setprecision(10) << val
             << "      " << std::scientific << std::setprecision(2) << err
             << "        " << lvl << "\n";
        
        // Do 5 more multiplications
        for (int j = 0; j < 5 && i + j < 25; j++) {
            ct = cc->EvalMult(ct, ct_mult);
        }
    }
    
    // === TEST 2: Noise growth WITH ring swap ===
    std::cout << "\n  === TEST 2: Noise growth WITH ring swap every 5 mults ===\n";
    std::cout << "  Multiply by 1.0, ring swap (mulY then mulY_inv) every 5 mults\n\n";
    
    auto pt_a = cc->MakeCKKSPackedPlaintext(std::vector<double>{1.0});
    auto pt_b = cc->MakeCKKSPackedPlaintext(std::vector<double>{0.0});
    PE state = {cc->Encrypt(kp.publicKey, pt_a), cc->Encrypt(kp.publicKey, pt_b)};
    auto ct_mul = cc->Encrypt(kp.publicKey, pt1);
    
    std::cout << "  Mults   a-value          b-value          Error from 1.0    Level(a)\n";
    std::cout << "  -------------------------------------------------------------------\n";
    
    for (int i = 0; i <= 25; i += 5) {
        double a_val = get_val(cc, kp, state.a);
        double b_val = get_val(cc, kp, state.b);
        double err = std::abs(a_val - 1.0);
        int lvl = state.a->GetLevel();
        
        std::cout << "  " << std::setw(3) << i
             << "    " << std::fixed << std::setprecision(10) << a_val
             << "  " << std::fixed << std::setprecision(10) << b_val
             << "      " << std::scientific << std::setprecision(2) << err
             << "        " << lvl << "\n";
        
        // Do 5 multiplications on component a
        for (int j = 0; j < 5 && i + j < 25; j++) {
            state.a = cc->EvalMult(state.a, ct_mul);
        }
        
        // Ring swap: mulY then mulY_inv (back to original)
        if (i + 5 < 25) {
            state = mulY(cc, state);
            state = mulY_inv(cc, state);
        }
    }
    
    // === TEST 3: Ring swap as NOISE FILTER? ===
    std::cout << "\n  === TEST 3: Ring swap effect on noisy state ===\n";
    std::cout << "  Start with 10 multiplications (noisy), then ring swap, check error\n\n";
    
    auto ct_noisy = cc->Encrypt(kp.publicKey, pt1);
    for (int i = 0; i < 10; i++) {
        ct_noisy = cc->EvalMult(ct_noisy, ct_mult);
    }
    
    Plaintext before_pt;
    cc->Decrypt(kp.secretKey, ct_noisy, &before_pt);
    double before_val = before_pt->GetCKKSPackedValue()[0].real();
    double before_err = std::abs(before_val - 1.0);
    
    // Put noisy ct into PE state and ring swap
    PE noisy_state = {ct_noisy, cc->Encrypt(kp.publicKey, pt1)}; // a=noisy, b=1.0
    noisy_state = mulY(cc, noisy_state);       // (b, a+b)
    noisy_state = mulY_inv(cc, noisy_state);    // back to original orientation
    
    double after_a = get_val(cc, kp, noisy_state.a);
    double after_b = get_val(cc, kp, noisy_state.b);
    double after_err = std::abs(after_a - 1.0);
    
    std::cout << "  Before ring swap: value=" << std::fixed << std::setprecision(10) << before_val;
    std::cout << "  error=" << std::scientific << before_err << "\n";
    std::cout << "  After ring swap:  a=" << std::fixed << std::setprecision(10) << after_a;
    std::cout << "  b=" << after_b << "\n";
    std::cout << "  Error after swap: " << std::scientific << after_err << "\n";
    std::cout << "  Error reduction:  " << std::fixed << std::setprecision(2) 
              << (before_err - after_err) / before_err * 100 << "%\n\n";
    
    return 0;
}
