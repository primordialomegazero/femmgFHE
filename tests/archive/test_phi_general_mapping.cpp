// Derive general f(x, vB, vC) and f_inv(t, vB, vC)
#include <iostream>
#include <iomanip>
#include <cmath>
#include "openfhe.h"
using namespace lbcrypto;

struct PE { Ciphertext<DCRTPoly> a; Ciphertext<DCRTPoly> b; };
PE mulY(CryptoContext<DCRTPoly>& cc, const PE& x) { return {x.b, cc->EvalAdd(x.a, x.b)}; }
PE ratio_add(CryptoContext<DCRTPoly>& cc, const PE& x, const PE& y) {
    auto a1b2=cc->EvalMult(x.a,y.b), a2b1=cc->EvalMult(y.a,x.b), b1b2=cc->EvalMult(x.b,y.b);
    return {cc->EvalAdd(a1b2,a2b1), b1b2};
}
PE ratio_mult(CryptoContext<DCRTPoly>& cc, const PE& x, const PE& y) {
    return {cc->EvalMult(x.a,y.a), cc->EvalMult(x.b,y.b)};
}
double get_val(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, const Ciphertext<DCRTPoly>& c) {
    Plaintext pt; cc->Decrypt(kp.secretKey, c, &pt); return pt->GetCKKSPackedValue()[0].real();
}
PE make_state(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, double v) {
    const double PSI=0.6180339887498949;
    return {cc->Encrypt(kp.publicKey,cc->MakeCKKSPackedPlaintext(std::vector<double>{v+PSI})),
            cc->Encrypt(kp.publicKey,cc->MakeCKKSPackedPlaintext(std::vector<double>{1.0}))};
}
const double PSI=0.6180339887498949, PSI2=PSI*PSI;
double correct_add_then_mul(double raw,double sv,double vC,int N){
    return raw-sv*PSI-N*PSI*vC-N*PSI2+PSI-PSI*(PSI+vC);
}

// Compute f(x, vB, vC) experimentally
double f_experimental(double x, double vB, double vC, CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp) {
    PE A=make_state(cc,kp,x), B=make_state(cc,kp,vB), C=make_state(cc,kp,vC);
    PE sum=ratio_add(cc,A,B);
    double sdec=get_val(cc,kp,sum.a)/get_val(cc,kp,sum.b)-PSI;
    PE res=ratio_mult(cc,sum,C);
    double raw=get_val(cc,kp,res.a)/get_val(cc,kp,res.b)-PSI;
    return correct_add_then_mul(raw-(sdec-(x+vB)), x+vB, vC, 1);
}

int main() {
    std::cout << "\n  ╔══════════════════════════════════════════════════════════╗\n";
    std::cout <<   "  ║   Derive GENERAL f(x, vB, vC) and f_inv(t, vB, vC)       ║\n";
    std::cout <<   "  ╚══════════════════════════════════════════════════════════╝\n\n";
    
    CCParams<CryptoContextCKKSRNS> p;
    p.SetMultiplicativeDepth(200);p.SetScalingModSize(50);p.SetBatchSize(2048);
    p.SetRingDim(4096);p.SetSecretKeyDist(UNIFORM_TERNARY);p.SetSecurityLevel(HEStd_NotSet);
    auto cc=GenCryptoContext(p);
    cc->Enable(PKE);cc->Enable(KEYSWITCH);cc->Enable(LEVELEDSHE);cc->Enable(ADVANCEDSHE);
    auto kp=cc->KeyGen();cc->EvalMultKeyGen(kp.secretKey);
    
    std::cout << std::fixed << std::setprecision(10);
    
    // Test: for each (vB, vC), compute f(0) and f(1) to find slope and intercept
    std::cout << "  vB     vC     f(0)          f(1)          slope(a)  intercept(b)\n";
    std::cout << "  -----------------------------------------------------------------\n";
    
    double vB_vals[] = {0.1, 0.3, 0.5, 1.0};
    double vC_vals[] = {0.2, 0.5, 0.5, 0.5};
    
    for (int i = 0; i < 4; i++) {
        double vB = vB_vals[i], vC = vC_vals[i];
        double f0 = f_experimental(0.0, vB, vC, cc, kp);
        double f1 = f_experimental(1.0, vB, vC, cc, kp);
        double a = f1 - f0;  // slope = f(1) - f(0)
        double b = f0;        // intercept = f(0)
        
        std::cout << "  " << std::setw(5) << vB
             << "  " << std::setw(5) << vC
             << "  " << std::setw(12) << f0
             << "  " << std::setw(12) << f1
             << "  " << std::setw(9) << a
             << "  " << std::setw(12) << b << "\n";
    }
    
    // Derive formula for a and b in terms of vB, vC
    std::cout << "\n  === DERIVED FORMULAS ===\n";
    std::cout << "  From theory: f(x) = vC*x + (vB+vC+2ψ)(vC+ψ) - (vB+vC)ψ - ψ*vC - ψ² + ψ - ψ(ψ+vC) - vB*ψ?\n";
    
    // Let's compute symbolically for vB=0.3, vC=0.2
    double vB=0.3, vC=0.2;
    double a_theory = vC;
    double b_theory = (vB+vC+2*PSI)*(vC+PSI) - (vB+vC)*PSI - PSI*vC - PSI2 + PSI - PSI*(PSI+vC) - vC*vB - vC*vB;
    // Actually let me just compute: f(0) = what?
    // f(0) = correct_add_then_mul(raw - psi_offset, vB, vC, 1)
    // where raw = (vB+2ψ)*(vC+ψ) - ψ, psi_offset = ψ
    double raw0 = (vB+2*PSI)*(vC+PSI) - PSI;
    double corrected0 = correct_add_then_mul(raw0 - PSI, vB, vC, 1);
    std::cout << "  For vB=" << vB << ", vC=" << vC << ":\n";
    std::cout << "  Theoretical f(0) = " << corrected0 << "\n";
    std::cout << "  Experimental f(0) = " << f_experimental(0.0, vB, vC, cc, kp) << "\n\n";
    
    // General formula:
    // f(x) = vC * x + [(vB+2ψ)(vC+ψ) - 2ψ - vB*ψ - ψ*vC - ψ² + ψ - ψ(ψ+vC)]
    // Simplify constant term
    double const_term = (vB+2*PSI)*(vC+PSI) - 2*PSI - vB*PSI - PSI*vC - PSI2 + PSI - PSI*(PSI+vC);
    std::cout << "  f(x) = " << vC << " * x + " << const_term << "\n";
    std::cout << "  f_inv(t) = (t - " << const_term << ") / " << vC << "\n\n";
    
    // Test the general inverse
    double target = 0.16;
    double x_input = (target - const_term) / vC;
    double output = f_experimental(x_input, vB, vC, cc, kp);
    std::cout << "  Test: input=" << x_input << " → output=" << output << " (target=" << target << ")\n";
    std::cout << "  Error: " << std::scientific << std::abs(output - target) << "\n";
    
    return 0;
}
