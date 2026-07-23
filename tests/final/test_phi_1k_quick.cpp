// DM-DGR 1K QUICK TEST
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
double correct_add_then_mul(double raw,double sv,double vC,int N){
    return raw-sv*PSI-N*PSI*vC-N*PSI2+PSI-PSI*(PSI+vC);
}
double exact_recover(CryptoContext<DCRTPoly>& cc,KeyPair<DCRTPoly>& kp,double v,int s=8){
    PE st=make_state(cc,kp,v);for(int i=0;i<s;i++)st=mulY(cc,st);
    return(get_val(cc,kp,st.b)-fib(s+1))/fib(s)-PSI;
}
double const_term(double vB, double vC) {
    return (vB+2*PSI)*(vC+PSI) - 2*PSI - vB*PSI - PSI*vC - PSI2 + PSI - PSI*(PSI+vC);
}
double f_inv(double t, double vB, double vC) { return (t - const_term(vB, vC)) / vC; }

int main(){
    std::cout<<"\n  ╔══════════════════════════════════════════════════════════╗\n";
    std::cout<<  "  ║   DM-DGR 1K QUICK TEST                                   ║\n";
    std::cout<<  "  ╚══════════════════════════════════════════════════════════╝\n\n";
    
    CCParams<CryptoContextCKKSRNS> p;
    p.SetMultiplicativeDepth(200);p.SetScalingModSize(50);p.SetBatchSize(2048);
    p.SetRingDim(4096);p.SetSecretKeyDist(UNIFORM_TERNARY);p.SetSecurityLevel(HEStd_NotSet);
    auto cc=GenCryptoContext(p);
    cc->Enable(PKE);cc->Enable(KEYSWITCH);cc->Enable(LEVELEDSHE);cc->Enable(ADVANCEDSHE);
    auto kp=cc->KeyGen();cc->EvalMultKeyGen(kp.secretKey);
    
    double vB=0.3, vC=0.2, target=0.16;
    PE B=make_state(cc,kp,vB), C=make_state(cc,kp,vC);
    double input=f_inv(target,vB,vC);
    int passed=0; double max_err=0;
    
    std::cout<<"  Target: "<<target<<" | vB="<<vB<<" vC="<<vC<<" | 1000 cycles\n\n";
    std::cout<<"  Cycle  Error          Status\n";
    std::cout<<"  -----------------------------\n";
    
    for(int cycle=0;cycle<1000;cycle++){
        PE A=make_state(cc,kp,input);
        PE sum=ratio_add(cc,A,B);
        double sdec=get_val(cc,kp,sum.a)/get_val(cc,kp,sum.b)-PSI;
        PE res=ratio_mult(cc,sum,C);
        double raw=get_val(cc,kp,res.a)/get_val(cc,kp,res.b)-PSI;
        double corr=correct_add_then_mul(raw-(sdec-(input+vB)),input+vB,vC,1);
        double norm=exact_recover(cc,kp,corr,8);
        double err=std::abs(norm-target);
        if(err<1e-10)passed++; if(err>max_err)max_err=err;
        input=f_inv(target,vB,vC);
        
        if(cycle%100==0)std::cout<<"  "<<std::setw(5)<<cycle<<std::scientific<<std::setprecision(1)<<err<<"      ✅\n";
    }
    
    std::cout<<"\n  ╔══════════════════════════════════════════════════════════╗\n";
    std::cout<<  "  ║  Passed: "<<passed<<"/1000 | Max error: "<<std::scientific<<max_err;
    for(int i=0;i<20;i++)std::cout<<" ";std::cout<<"║\n";
    std::cout<<  "  ╚══════════════════════════════════════════════════════════╝\n\n";
    return 0;
}
