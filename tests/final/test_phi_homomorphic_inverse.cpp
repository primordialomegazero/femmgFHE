// Homomorphic Inverse Mapping v4: Compose f_add then f_mul
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
long long fib(int n){if(n<=1)return n;long long a=0,b=1;for(int i=2;i<=n;i++){long long t=a+b;a=b;b=t;}return b;}
double exact_recover(CryptoContext<DCRTPoly>& cc,KeyPair<DCRTPoly>& kp,double v,int s=8){
    PE st=make_state(cc,kp,v);for(int i=0;i<s;i++)st=mulY(cc,st);
    return(get_val(cc,kp,st.b)-fib(s+1))/fib(s)-PSI;
}

// Gate functions with their OWN corrections
double f_add(double x, double vB, CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp) {
    PE A=make_state(cc,kp,x), B=make_state(cc,kp,vB);
    PE sum=ratio_add(cc,A,B);
    double sdec=get_val(cc,kp,sum.a)/get_val(cc,kp,sum.b)-PSI;
    return sdec - PSI;  // f_add(x) = x + vB
}
double f_add_inv(double t, double vB) { return t - vB; }

double f_mul(double x, double vC, CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp) {
    PE A=make_state(cc,kp,x), C=make_state(cc,kp,vC);
    PE prod=ratio_mult(cc,A,C);
    double raw=get_val(cc,kp,prod.a)/get_val(cc,kp,prod.b)-PSI;
    return raw - PSI*(x+vC) + (2.0*PSI-1.0);  // f_mul(x) = vC*x
}
double f_mul_inv(double t, double vC) { return t / vC; }

double homomorphic_inverse(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp,
                           Ciphertext<DCRTPoly>& ct_target, double slope, double intercept) {
    auto pt_intercept = cc->MakeCKKSPackedPlaintext(std::vector<double>{intercept});
    auto ct_intercept = cc->Encrypt(kp.publicKey, pt_intercept);
    auto ct_diff = cc->EvalSub(ct_target, ct_intercept);
    auto pt_inv = cc->MakeCKKSPackedPlaintext(std::vector<double>{1.0/slope});
    auto ct_inv = cc->Encrypt(kp.publicKey, pt_inv);
    auto ct_result = cc->EvalMult(ct_diff, ct_inv);
    return get_val(cc, kp, ct_result);
}

int main() {
    std::cout << "\n  ╔══════════════════════════════════════════════════════════╗\n";
    std::cout <<   "  ║   Homomorphic Circuit: Compose f_add then f_mul          ║\n";
    std::cout <<   "  ╚══════════════════════════════════════════════════════════╝\n\n";
    
    CCParams<CryptoContextCKKSRNS> p;
    p.SetMultiplicativeDepth(200);p.SetScalingModSize(50);p.SetBatchSize(2048);
    p.SetRingDim(4096);p.SetSecretKeyDist(UNIFORM_TERNARY);p.SetSecurityLevel(HEStd_NotSet);
    auto cc=GenCryptoContext(p);
    cc->Enable(PKE);cc->Enable(KEYSWITCH);cc->Enable(LEVELEDSHE);cc->Enable(ADVANCEDSHE);
    auto kp=cc->KeyGen();cc->EvalMultKeyGen(kp.secretKey);
    
    double vB = 0.3, vC = 0.2;
    double circuit_target = 0.16;
    
    // Encrypt target
    auto pt_target = cc->MakeCKKSPackedPlaintext(std::vector<double>{circuit_target});
    auto ct_target = cc->Encrypt(kp.publicKey, pt_target);
    
    // Circuit: (A+B)×C = f_mul(f_add(x, vB), vC)
    // f_circuit(x) = f_mul(f_add(x, vB), vC)
    //              = f_mul(x + vB, vC)
    //              = vC * (x + vB)
    //              = vC*x + vC*vB
    // So: f_circuit(x) = vC*x + vC*vB
    // f_circuit_inv(t) = (t - vC*vB) / vC
    
    double circuit_slope = vC;
    double circuit_intercept = vC * vB;  // This is the TRUE intercept!
    
    std::cout << "  Circuit: (A+B)xC = f_mul(f_add(x," << vB << ")," << vC << ")\n";
    std::cout << "  f_circuit(x) = " << vC << "*x + " << circuit_intercept << "\n";
    std::cout << "  f_circuit_inv(t) = (t - " << circuit_intercept << ") / " << vC << "\n\n";
    
    // Homomorphic inverse
    double h_input = homomorphic_inverse(cc, kp, ct_target, circuit_slope, circuit_intercept);
    std::cout << "  Homomorphic inverse: " << std::fixed << std::setprecision(10) << h_input << "\n";
    std::cout << "  Expected input: " << (circuit_target - circuit_intercept)/circuit_slope << "\n\n";
    
    // Apply circuit using COMPOSED gates
    double after_add = f_add(h_input, vB, cc, kp);
    double after_mul = f_mul(after_add, vC, cc, kp);
    
    std::cout << "  After f_add: " << after_add << " (expected " << h_input+vB << ")\n";
    std::cout << "  After f_mul: " << after_mul << " (target=" << circuit_target << ")\n";
    std::cout << "  Error: " << std::scientific << std::abs(after_mul - circuit_target) << "\n\n";
    
    // Also verify with the OLD method (make_state + ratio_add + ratio_mult + correct_add_then_mul)
    double old_const = (vB+2*PSI)*(vC+PSI) - 2*PSI - vB*PSI - PSI*vC - PSI2 + PSI - PSI*(PSI+vC);
    PE A=make_state(cc,kp,h_input), B=make_state(cc,kp,vB), C=make_state(cc,kp,vC);
    PE sum=ratio_add(cc,A,B);
    double sdec=get_val(cc,kp,sum.a)/get_val(cc,kp,sum.b)-PSI;
    PE res=ratio_mult(cc,sum,C);
    double raw=get_val(cc,kp,res.a)/get_val(cc,kp,res.b)-PSI;
    double old_corrected = raw - (h_input+vB)*PSI - PSI*vC - PSI2 + PSI - PSI*(PSI+vC);
    double old_norm = exact_recover(cc, kp, old_corrected, 8);
    
    std::cout << "  Old method output: " << old_norm << " (target=" << circuit_target << ")\n";
    std::cout << "  Old method error: " << std::scientific << std::abs(old_norm - circuit_target) << "\n\n";
    
    bool ok = std::abs(after_mul - circuit_target) < 0.01;
    std::cout << "  ╔══════════════════════════════════════════════════════════╗\n";
    std::cout <<   "  ║  Composed gates: " << (ok ? "WORKING!" : "NEED FIX") << "                                 ║\n";
    std::cout <<   "  ╚══════════════════════════════════════════════════════════╝\n\n";
    
    return 0;
}
