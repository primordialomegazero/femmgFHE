// Derive f_mul(x, vC): Pure multiplication mapping
#include <iostream>
#include <iomanip>
#include <cmath>
#include "openfhe.h"
using namespace lbcrypto;

struct PE { Ciphertext<DCRTPoly> a; Ciphertext<DCRTPoly> b; };
PE mulY(CryptoContext<DCRTPoly>& cc, const PE& x) { return {x.b, cc->EvalAdd(x.a, x.b)}; }
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
long long fib(int n){if(n<=1)return n;long long a=0,b=1;for(int i=2;i<=n;i++){long long t=a+b;a=b;b=t;}return b;}
double exact_recover(CryptoContext<DCRTPoly>& cc,KeyPair<DCRTPoly>& kp,double v,int s=8){
    PE st=make_state(cc,kp,v);for(int i=0;i<s;i++)st=mulY(cc,st);
    return(get_val(cc,kp,st.b)-fib(s+1))/fib(s)-PSI;
}

// f_mul(x, vC): output = x * vC (via ratio_mult, then corrected)
double f_mul(double x, double vC, CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp) {
    PE A=make_state(cc,kp,x), C=make_state(cc,kp,vC);
    PE prod=ratio_mult(cc,A,C);
    double raw=get_val(cc,kp,prod.a)/get_val(cc,kp,prod.b)-PSI;
    // Pure multiplication correction: raw - psi*(v1+v2) + (2*psi-1)
    return raw - PSI*(x + vC) + (2.0*PSI - 1.0);
}

int main() {
    std::cout << "\n  ╔══════════════════════════════════════════════════════════╗\n";
    std::cout <<   "  ║   Derive f_mul(x, vC): Pure Multiplication Mapping       ║\n";
    std::cout <<   "  ╚══════════════════════════════════════════════════════════╝\n\n";
    
    CCParams<CryptoContextCKKSRNS> p;
    p.SetMultiplicativeDepth(200);p.SetScalingModSize(50);p.SetBatchSize(2048);
    p.SetRingDim(4096);p.SetSecretKeyDist(UNIFORM_TERNARY);p.SetSecurityLevel(HEStd_NotSet);
    auto cc=GenCryptoContext(p);
    cc->Enable(PKE);cc->Enable(KEYSWITCH);cc->Enable(LEVELEDSHE);cc->Enable(ADVANCEDSHE);
    auto kp=cc->KeyGen();cc->EvalMultKeyGen(kp.secretKey);
    
    double vC = 0.2;
    std::cout << "  vC = " << vC << " (constant)\n\n";
    std::cout << "  Input(x)   f_mul(x)    Expected(x*vC)  Error\n";
    std::cout << "  ---------------------------------------------\n";
    
    double f0 = f_mul(0.0, vC, cc, kp);
    double f1 = f_mul(1.0, vC, cc, kp);
    double slope = f1 - f0;
    double intercept = f0;
    
    for (double x = -2.0; x <= 2.0; x += 0.5) {
        double fx = f_mul(x, vC, cc, kp);
        double expected = x * vC;
        double err = std::abs(fx - expected);
        std::cout << "  " << std::setw(8) << std::fixed << std::setprecision(1) << x
             << "  " << std::setw(10) << std::fixed << std::setprecision(6) << fx
             << "  " << std::setw(10) << expected
             << "  " << std::scientific << std::setprecision(1) << err << "\n";
    }
    
    std::cout << "\n  f_mul(x, " << vC << ") = " << slope << " * x + " << intercept << "\n";
    std::cout << "  f_mul_inv(t, " << vC << ") = (t - " << intercept << ") / " << slope << "\n\n";
    
    // Test inverse for 50 cycles
    std::cout << "  === 50-CYCLE TEST: f_mul_inv ===\n";
    double target = 0.15;
    double input = (target - intercept) / slope;
    int passed = 0; double max_err = 0;
    
    for (int cycle = 0; cycle < 50; cycle++) {
        double output = f_mul(input, vC, cc, kp);
        double err = std::abs(output - target);
        if (err < 1e-10) passed++;
        if (err > max_err) max_err = err;
        input = (target - intercept) / slope;
        if (cycle < 3) std::cout << "  Cycle " << cycle << ": " << std::fixed << std::setprecision(6) << output << " (err=" << std::scientific << err << ")\n";
    }
    std::cout << "  Passed: " << passed << "/50 | Max error: " << std::scientific << max_err << "\n";
    
    return 0;
}
