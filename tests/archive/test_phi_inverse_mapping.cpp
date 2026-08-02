// Compute f(x) for many x values, then derive f⁻¹
#include <iostream>
#include <iomanip>
#include <cmath>
#include "openfhe.h"
using namespace lbcrypto;

struct PE { Ciphertext<DCRTPoly> a; Ciphertext<DCRTPoly> b; };

PE mulY(CryptoContext<DCRTPoly>& cc, const PE& x) {
    return {x.b, cc->EvalAdd(x.a, x.b)};
}
PE ratio_add(CryptoContext<DCRTPoly>& cc, const PE& x, const PE& y) {
    auto a1b2 = cc->EvalMult(x.a, y.b);
    auto a2b1 = cc->EvalMult(y.a, x.b);
    auto b1b2 = cc->EvalMult(x.b, y.b);
    return {cc->EvalAdd(a1b2, a2b1), b1b2};
}
PE ratio_mult(CryptoContext<DCRTPoly>& cc, const PE& x, const PE& y) {
    return {cc->EvalMult(x.a, y.a), cc->EvalMult(x.b, y.b)};
}

double get_val(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, const Ciphertext<DCRTPoly>& c) {
    Plaintext pt; cc->Decrypt(kp.secretKey, c, &pt);
    return pt->GetCKKSPackedValue()[0].real();
}
PE make_state(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, double value) {
    const double PSI = 0.6180339887498949;
    auto pta = cc->MakeCKKSPackedPlaintext(std::vector<double>{value + PSI});
    auto ptb = cc->MakeCKKSPackedPlaintext(std::vector<double>{1.0});
    return {cc->Encrypt(kp.publicKey, pta), cc->Encrypt(kp.publicKey, ptb)};
}

const double PSI = 0.6180339887498949;
const double PSI2 = PSI * PSI;

// The actual function that maps input → corrected output
double f(double x, CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp) {
    PE A = make_state(cc, kp, x);
    PE B = make_state(cc, kp, 0.3);
    PE C = make_state(cc, kp, 0.2);
    
    PE sum_ab = ratio_add(cc, A, B);
    double sum_decoded = get_val(cc, kp, sum_ab.a) / get_val(cc, kp, sum_ab.b) - PSI;
    double plaintext_sum = x + 0.3;
    double psi_offset = sum_decoded - plaintext_sum;
    
    PE result = ratio_mult(cc, sum_ab, C);
    double raw_val = get_val(cc, kp, result.a) / get_val(cc, kp, result.b) - PSI;
    double raw_corrected = raw_val - psi_offset;
    
    return raw_corrected - plaintext_sum * PSI - 1 * PSI * 0.2 - 1 * PSI2 + PSI - PSI * (PSI + 0.2);
}

int main() {
    std::cout << "\n  ╔══════════════════════════════════════════════════════════╗\n";
    std::cout <<   "  ║   Compute f(x) for many x, find inverse mapping          ║\n";
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
    
    std::cout << std::fixed << std::setprecision(10);
    std::cout << "  Input(x)    Output f(x)   f(x)-x       Type\n";
    std::cout << "  -------------------------------------------------\n";
    
    double fixed_point = -0.697542; // from experiment
    double prev_diff = 0;
    
    for (double x = -2.0; x <= 2.0; x += 0.1) {
        double fx = f(x, cc, kp);
        double diff = fx - x;
        
        std::cout << "  " << std::setw(8) << x
             << "  " << std::setw(12) << fx
             << "  " << std::setw(12) << diff;
        
        if (std::abs(diff) < 0.001) std::cout << "  ← FIXED POINT!";
        if (x > fixed_point - 0.01 && x < fixed_point + 0.01) std::cout << "  ← CONFIRMED";
        
        std::cout << "\n";
        prev_diff = diff;
    }
    
    // Now find the inverse: for target=0.16, what input gives f(x)=0.16?
    std::cout << "\n  === FIND INVERSE: f(x) = 0.16 ===\n";
    
    // Newton's method: x_{n+1} = x_n - (f(x_n) - target) / f'(x_n)
    // f'(x) ≈ 0.2 (from earlier analysis)
    double target = 0.16;
    double x_guess = target; // start with target as guess
    double fprime = 0.2; // approximate derivative
    
    for (int iter = 0; iter < 20; iter++) {
        double fx_val = f(x_guess, cc, kp);
        double error = fx_val - target;
        x_guess = x_guess - error / fprime;
        
        if (iter < 5 || iter == 19) {
            std::cout << "  Iter " << iter << ": x=" << x_guess << " f(x)=" << fx_val << " error=" << error << "\n";
        }
        
        if (std::abs(error) < 1e-10) break;
    }
    
    std::cout << "\n  Inverse: f⁻¹(" << target << ") = " << x_guess << "\n";
    std::cout << "  Verify: f(" << x_guess << ") = " << f(x_guess, cc, kp) << "\n\n";
    
    return 0;
}
