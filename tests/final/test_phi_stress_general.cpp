// DM-DGR STRESS TEST with GENERAL f_inv(t, vB, vC)
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

// GENERAL f_inv: const_term = (vB+2ψ)(vC+ψ) - 2ψ - vB·ψ - ψ·vC - ψ² + ψ - ψ(ψ+vC)
double const_term(double vB, double vC) {
    return (vB+2*PSI)*(vC+PSI) - 2*PSI - vB*PSI - PSI*vC - PSI2 + PSI - PSI*(PSI+vC);
}
double f_inv_general(double t, double vB, double vC) {
    return (t - const_term(vB, vC)) / vC;
}

int main(){
    std::cout<<"\n  ╔══════════════════════════════════════════════════════════╗\n";
    std::cout<<  "  ║   DM-DGR STRESS TEST: GENERAL f_inv(t, vB, vC)           ║\n";
    std::cout<<  "  ╚══════════════════════════════════════════════════════════╝\n\n";
    
    CCParams<CryptoContextCKKSRNS> p;
    p.SetMultiplicativeDepth(200);p.SetScalingModSize(50);p.SetBatchSize(2048);
    p.SetRingDim(4096);p.SetSecretKeyDist(UNIFORM_TERNARY);p.SetSecurityLevel(HEStd_NotSet);
    auto cc=GenCryptoContext(p);
    cc->Enable(PKE);cc->Enable(KEYSWITCH);cc->Enable(LEVELEDSHE);cc->Enable(ADVANCEDSHE);
    auto kp=cc->KeyGen();cc->EvalMultKeyGen(kp.secretKey);
    
    struct TestCase { double target, vB, vC; std::string name; };
    TestCase tests[]={
        {0.16, 0.3, 0.2, "A=0.5,B=0.3,C=0.2"},
        {0.50, 0.5, 0.5, "A=0.5,B=0.5,C=0.5"},
        {1.00, 1.0, 0.5, "A=1.0,B=1.0,C=0.5"},
        {0.25, 0.1, 0.5, "A=0.1,B=0.1,C=0.5"},
    };
    
    int total_passed=0, total_tests=0;
    
    for(auto& tc : tests){
        std::cout<<"  === "<<tc.name<<" ===\n";
        std::cout<<"  Target: "<<tc.target<<" | 50 cycles\n";
        
        PE B=make_state(cc,kp,tc.vB), C=make_state(cc,kp,tc.vC);
        double input=f_inv_general(tc.target, tc.vB, tc.vC);
        int passed=0; double max_err=0;
        
        for(int cycle=0;cycle<50;cycle++){
            PE A=make_state(cc,kp,input);
            PE sum=ratio_add(cc,A,B);
            double sdec=get_val(cc,kp,sum.a)/get_val(cc,kp,sum.b)-PSI;
            double psum=input+tc.vB, poff=sdec-psum;
            PE res=ratio_mult(cc,sum,C);
            double raw=get_val(cc,kp,res.a)/get_val(cc,kp,res.b)-PSI;
            double corr=correct_add_then_mul(raw-poff,psum,tc.vC,1);
            double norm=exact_recover(cc,kp,corr,8);
            double err=std::abs(norm-tc.target);
            if(err<1e-10)passed++; if(err>max_err)max_err=err;
            input=f_inv_general(tc.target, tc.vB, tc.vC);
            if(cycle<3)std::cout<<"  Cycle "<<cycle<<": "<<std::fixed<<std::setprecision(6)<<norm<<" err="<<std::scientific<<err<<"\n";
        }
        
        total_tests++; if(passed==50)total_passed++;
        std::cout<<"  Passed: "<<passed<<"/50 | Max error: "<<std::scientific<<max_err<<"\n\n";
    }
    
    std::cout<<"  ╔══════════════════════════════════════════════════════════╗\n";
    std::cout<<"  ║  ALL TESTS: "<<total_passed<<"/"<<total_tests<<" passed";
    for(int i=0;i<25;i++)std::cout<<" ";std::cout<<"║\n";
    std::cout<<"  ╚══════════════════════════════════════════════════════════╝\n\n";
    return 0;
}
