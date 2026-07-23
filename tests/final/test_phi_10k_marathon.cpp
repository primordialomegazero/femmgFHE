// DM-DGR 10K MARATHON: Random parameters, 10,000 cycles
#include <iostream>
#include <iomanip>
#include <cmath>
#include <cstdlib>
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
    std::cout<<  "  ║   DM-DGR 10K MARATHON + RANDOM PARAMETERS                ║\n";
    std::cout<<  "  ╚══════════════════════════════════════════════════════════╝\n\n";
    
    CCParams<CryptoContextCKKSRNS> p;
    p.SetMultiplicativeDepth(200);p.SetScalingModSize(50);p.SetBatchSize(2048);
    p.SetRingDim(4096);p.SetSecretKeyDist(UNIFORM_TERNARY);p.SetSecurityLevel(HEStd_NotSet);
    auto cc=GenCryptoContext(p);
    cc->Enable(PKE);cc->Enable(KEYSWITCH);cc->Enable(LEVELEDSHE);cc->Enable(ADVANCEDSHE);
    auto kp=cc->KeyGen();cc->EvalMultKeyGen(kp.secretKey);
    
    srand(time(0));
    int total_passed=0, total_tests=0;
    double global_max_err=0;
    
    // === TEST 1-5: RANDOM PARAMETERS ===
    std::cout<<"  === RANDOM PARAMETER TESTS ===\n";
    for(int t=0;t<5;t++){
        double vB=0.1+(rand()%100)/100.0;  // 0.1 to 1.1
        double vC=0.1+(rand()%100)/100.0;  // 0.1 to 1.1
        double target=0.05+(rand()%100)/100.0; // 0.05 to 1.05
        
        PE B=make_state(cc,kp,vB), C=make_state(cc,kp,vC);
        double input=f_inv(target,vB,vC);
        int passed=0; double max_err=0;
        
        for(int cycle=0;cycle<20;cycle++){
            PE A=make_state(cc,kp,input);
            PE sum=ratio_add(cc,A,B);
            double sdec=get_val(cc,kp,sum.a)/get_val(cc,kp,sum.b)-PSI;
            PE res=ratio_mult(cc,sum,C);
            double raw=get_val(cc,kp,res.a)/get_val(cc,kp,res.b)-PSI;
            double corr=correct_add_then_mul(raw-(sdec-(input+vB)),input+vB,vC,1);
            double norm=exact_recover(cc,kp,corr,8);
            double err=std::abs(norm-target);
            if(err<1e-10)passed++; if(err>max_err)max_err=err;
            if(err>global_max_err)global_max_err=err;
            input=f_inv(target,vB,vC);
        }
        
        total_tests++; if(passed==20)total_passed++;
        std::cout<<"  vB="<<std::fixed<<std::setprecision(2)<<vB
             <<" vC="<<vC<<" target="<<target
             <<" | "<<passed<<"/20 | max err="<<std::scientific<<max_err<<"\n";
    }
    
    // === 10,000 CYCLE MARATHON ===
    std::cout<<"\n  === 10,000 CYCLE MARATHON ===\n";
    double vB=0.3, vC=0.2, target=0.16;
    PE B=make_state(cc,kp,vB), C=make_state(cc,kp,vC);
    double input=f_inv(target,vB,vC);
    int passed=0; double max_err=0;
    
    for(int cycle=0;cycle<10000;cycle++){
        PE A=make_state(cc,kp,input);
        PE sum=ratio_add(cc,A,B);
        double sdec=get_val(cc,kp,sum.a)/get_val(cc,kp,sum.b)-PSI;
        PE res=ratio_mult(cc,sum,C);
        double raw=get_val(cc,kp,res.a)/get_val(cc,kp,res.b)-PSI;
        double corr=correct_add_then_mul(raw-(sdec-(input+vB)),input+vB,vC,1);
        double norm=exact_recover(cc,kp,corr,8);
        double err=std::abs(norm-target);
        if(err<1e-10)passed++; if(err>max_err)max_err=err;
        if(err>global_max_err)global_max_err=err;
        input=f_inv(target,vB,vC);
        
        if(cycle%2000==0)std::cout<<"  Cycle "<<cycle<<": err="<<std::scientific<<err<<" | passed so far: "<<passed<<"\n";
    }
    
    total_tests++; if(passed==10000)total_passed++;
    std::cout<<"  Final: "<<passed<<"/10000 | Max error: "<<std::scientific<<max_err<<"\n\n";
    
    std::cout<<"  ╔══════════════════════════════════════════════════════════╗\n";
    std::cout<<"  ║  TOTAL: "<<total_passed<<"/"<<total_tests<<" test groups passed";
    for(int i=0;i<20;i++)std::cout<<" ";std::cout<<"║\n";
    std::cout<<"  ║  10K marathon: "<<passed<<"/10000 cycles";
    for(int i=0;i<22;i++)std::cout<<" ";std::cout<<"║\n";
    std::cout<<"  ║  Global max error: "<<std::scientific<<global_max_err;
    for(int i=0;i<22;i++)std::cout<<" ";std::cout<<"║\n";
    std::cout<<"  ╚══════════════════════════════════════════════════════════╝\n\n";
    return 0;
}
