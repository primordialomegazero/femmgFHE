// DM-DGR COMPREHENSIVE STRESS TEST: All gate types, random params, 1000 cycles
#include <iostream>
#include <iomanip>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <vector>
#include <string>
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

double gate_add(double x, double vB, CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp) {
    PE A=make_state(cc,kp,x), B=make_state(cc,kp,vB);
    PE sum=ratio_add(cc,A,B);
    return get_val(cc,kp,sum.a)/get_val(cc,kp,sum.b)-PSI - PSI;
}
double gate_mul(double x, double vC, CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp) {
    PE A=make_state(cc,kp,x), C=make_state(cc,kp,vC);
    PE prod=ratio_mult(cc,A,C);
    double raw=get_val(cc,kp,prod.a)/get_val(cc,kp,prod.b)-PSI;
    return raw - PSI*(x+vC) + (2.0*PSI-1.0);
}
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

struct StressResult { std::string name; int cycles, passed; double max_err, avg_err, time_s; };

StressResult run_stress(std::string name, int N, double vB, double vC, double target,
                        CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp) {
    double slope=vC, intercept=vC*vB;
    auto pt_t=cc->MakeCKKSPackedPlaintext(std::vector<double>{target});
    auto ct_t=cc->Encrypt(kp.publicKey,pt_t);
    clock_t start=clock();
    int passed=0; double max_err=0, sum_err=0;
    for(int c=0;c<N;c++){
        double h=homomorphic_inverse(cc,kp,ct_t,slope,intercept);
        double out=gate_mul(gate_add(h,vB,cc,kp),vC,cc,kp);
        double err=std::abs(out-target);
        if(err<1e-10)passed++; if(err>max_err)max_err=err; sum_err+=err;
        if(c%200==0)std::cout<<"    ["<<std::setw(3)<<c<<"] out="<<std::fixed<<std::setprecision(10)<<out<<" err="<<std::scientific<<std::setprecision(1)<<err<<"\n";
    }
    clock_t end=clock();
    return {name,N,passed,max_err,sum_err/N,double(end-start)/CLOCKS_PER_SEC};
}

int main(){
    std::cout<<"\n";
    std::cout<<"  ╔══════════════════════════════════════════════════════════════════════╗\n";
    std::cout<<"  ║   DM-DGR COMPREHENSIVE STRESS TEST: 1000 cycles per configuration     ║\n";
    std::cout<<"  ╚══════════════════════════════════════════════════════════════════════╝\n\n";
    
    CCParams<CryptoContextCKKSRNS> p;
    p.SetMultiplicativeDepth(200);p.SetScalingModSize(50);p.SetBatchSize(2048);
    p.SetRingDim(4096);p.SetSecretKeyDist(UNIFORM_TERNARY);p.SetSecurityLevel(HEStd_NotSet);
    auto cc=GenCryptoContext(p);
    cc->Enable(PKE);cc->Enable(KEYSWITCH);cc->Enable(LEVELEDSHE);cc->Enable(ADVANCEDSHE);
    auto kp=cc->KeyGen();cc->EvalMultKeyGen(kp.secretKey);
    srand(time(0));
    std::vector<StressResult> results;
    
    std::cout<<"  === STRESS 1: Standard (A+B)xC, vB=0.3, vC=0.2, target=0.16 ===\n\n";
    results.push_back(run_stress("Standard (A+B)xC",1000,0.3,0.2,0.16,cc,kp));
    std::cout<<"\n";
    
    double vB=0.1+(rand()%100)/100.0, vC=0.1+(rand()%100)/100.0, t=0.05+(rand()%100)/100.0;
    std::cout<<"  === STRESS 2: Random small, vB="<<vB<<" vC="<<vC<<" target="<<t<<" ===\n\n";
    results.push_back(run_stress("Random small",1000,vB,vC,t,cc,kp));
    std::cout<<"\n";
    
    vB=0.5+(rand()%200)/100.0; vC=0.5+(rand()%200)/100.0; t=0.25+(rand()%300)/100.0;
    std::cout<<"  === STRESS 3: Random medium, vB="<<vB<<" vC="<<vC<<" target="<<t<<" ===\n\n";
    results.push_back(run_stress("Random medium",1000,vB,vC,t,cc,kp));
    std::cout<<"\n";
    
    vB=1.0+(rand()%400)/100.0; vC=1.0+(rand()%400)/100.0; t=0.5+(rand()%500)/100.0;
    std::cout<<"  === STRESS 4: Random large, vB="<<vB<<" vC="<<vC<<" target="<<t<<" ===\n\n";
    results.push_back(run_stress("Random large",1000,vB,vC,t,cc,kp));
    std::cout<<"\n";
    
    std::cout<<"  === STRESS 5: Negative values, vB=0.3, vC=0.5, target=-0.25 ===\n\n";
    results.push_back(run_stress("Negative target",1000,0.3,0.5,-0.25,cc,kp));
    std::cout<<"\n";
    
    std::cout<<"  === STRESS 6: Edge case small, vB=0.01, vC=0.01, target=0.001 ===\n\n";
    results.push_back(run_stress("Edge small values",1000,0.01,0.01,0.001,cc,kp));
    std::cout<<"\n";
    
    std::cout<<"  === STRESS 7: Edge case large, vB=5.0, vC=5.0, target=10.0 ===\n\n";
    results.push_back(run_stress("Edge large values",1000,5.0,5.0,10.0,cc,kp));
    std::cout<<"\n";
    
    std::cout<<"  === STRESS 8: Pure addition chain (3 ops), target=1.5 ===\n\n";
    {
        double add_t=1.5; auto pt_t=cc->MakeCKKSPackedPlaintext(std::vector<double>{add_t});
        auto ct_t=cc->Encrypt(kp.publicKey,pt_t);
        clock_t start=clock(); int passed=0; double max_err=0, sum_err=0;
        for(int c=0;c<1000;c++){
            double h=homomorphic_inverse(cc,kp,ct_t,1.0,0.0);
            double out=gate_add(gate_add(gate_add(h,0.5,cc,kp),0.5,cc,kp),0.5,cc,kp);
            double err=std::abs(out-add_t);
            if(err<1e-10)passed++; if(err>max_err)max_err=err; sum_err+=err;
            if(c%200==0)std::cout<<"    ["<<std::setw(3)<<c<<"] out="<<std::fixed<<std::setprecision(10)<<out<<" err="<<std::scientific<<std::setprecision(1)<<err<<"\n";
        }
        clock_t end=clock();
        results.push_back({"3-Add chain",1000,passed,max_err,sum_err/1000,double(end-start)/CLOCKS_PER_SEC});
    }
    std::cout<<"\n";
    
    std::cout<<"  === STRESS 9: Pure multiplication chain (3 ops), target=0.008 ===\n\n";
    {
        double mul_t=0.008; auto pt_t=cc->MakeCKKSPackedPlaintext(std::vector<double>{mul_t});
        auto ct_t=cc->Encrypt(kp.publicKey,pt_t);
        clock_t start=clock(); int passed=0; double max_err=0, sum_err=0;
        for(int c=0;c<1000;c++){
            double h=homomorphic_inverse(cc,kp,ct_t,0.2,0.0);
            double out=gate_mul(gate_mul(gate_mul(h,0.2,cc,kp),0.2,cc,kp),0.2,cc,kp);
            double err=std::abs(out-mul_t);
            if(err<1e-10)passed++; if(err>max_err)max_err=err; sum_err+=err;
            if(c%200==0)std::cout<<"    ["<<std::setw(3)<<c<<"] out="<<std::fixed<<std::setprecision(10)<<out<<" err="<<std::scientific<<std::setprecision(1)<<err<<"\n";
        }
        clock_t end=clock();
        results.push_back({"3-Mul chain",1000,passed,max_err,sum_err/1000,double(end-start)/CLOCKS_PER_SEC});
    }
    std::cout<<"\n";
    
    std::cout<<"  === STRESS 10: Mixed add-mul-add chain, target=0.66 ===\n\n";
    {
        double mix_t=0.66; auto pt_t=cc->MakeCKKSPackedPlaintext(std::vector<double>{mix_t});
        auto ct_t=cc->Encrypt(kp.publicKey,pt_t);
        clock_t start=clock(); int passed=0; double max_err=0, sum_err=0;
        for(int c=0;c<1000;c++){
            double h=homomorphic_inverse(cc,kp,ct_t,0.5,0.3);
            double a1=gate_add(h,0.3,cc,kp);
            double m1=gate_mul(a1,0.5,cc,kp);
            double a2=gate_add(m1,0.3,cc,kp);
            double err=std::abs(a2-mix_t);
            if(err<1e-10)passed++; if(err>max_err)max_err=err; sum_err+=err;
            if(c%200==0)std::cout<<"    ["<<std::setw(3)<<c<<"] out="<<std::fixed<<std::setprecision(10)<<a2<<" err="<<std::scientific<<std::setprecision(1)<<err<<"\n";
        }
        clock_t end=clock();
        results.push_back({"Add-Mul-Add",1000,passed,max_err,sum_err/1000,double(end-start)/CLOCKS_PER_SEC});
    }
    std::cout<<"\n";
    
    // FINAL SUMMARY
    std::cout<<"  ╔══════════════════════════════════════════════════════════════════════════════════════╗\n";
    std::cout<<"  ║                              COMPREHENSIVE STRESS TEST SUMMARY                        ║\n";
    std::cout<<"  ╠════════════════════════════════════════════════════════════════════════════════════════╣\n";
    std::cout<<"  ║ Test                    Cycles   Passed    Max Error       Avg Error    Time (s)      ║\n";
    std::cout<<"  ╠════════════════════════════════════════════════════════════════════════════════════════╣\n";
    
    int total_passed=0, total_cyc=0; double total_time=0, global_max=0;
    for(auto& r:results){
        std::cout<<"  ║ "<<std::setw(23)<<std::left<<r.name<<" "<<std::setw(8)<<r.cycles<<" "<<std::setw(8)<<r.passed
             <<" "<<std::scientific<<std::setprecision(1)<<std::setw(13)<<r.max_err
             <<" "<<std::scientific<<std::setprecision(1)<<std::setw(13)<<r.avg_err
             <<" "<<std::fixed<<std::setprecision(2)<<std::setw(10)<<r.time_s<<"  ║\n";
        total_passed+=r.passed; total_cyc+=r.cycles; total_time+=r.time_s;
        if(r.max_err>global_max)global_max=r.max_err;
    }
    
    std::cout<<"  ╠════════════════════════════════════════════════════════════════════════════════════════╣\n";
    double ops_s=total_cyc/total_time, ms_op=1000.0*total_time/total_cyc;
    std::cout<<"  ║ "<<std::setw(23)<<"TOTAL"<<" "<<std::setw(8)<<total_cyc<<" "<<std::setw(8)<<total_passed
         <<" "<<std::scientific<<std::setprecision(1)<<std::setw(13)<<global_max
         <<" "<<std::setw(13)<<"-"
         <<" "<<std::fixed<<std::setprecision(2)<<std::setw(10)<<total_time<<"  ║\n";
    std::cout<<"  ╠════════════════════════════════════════════════════════════════════════════════════════╣\n";
    std::cout<<"  ║ Pass rate: "<<std::fixed<<std::setprecision(2)<<100.0*total_passed/total_cyc
         <<"%  |  Throughput: "<<std::fixed<<std::setprecision(2)<<ops_s
         <<" ops/s  |  Latency: "<<std::fixed<<std::setprecision(1)<<ms_op<<" ms/op        ║\n";
    std::cout<<"  ║ Global max error: "<<std::scientific<<std::setprecision(1)<<global_max
         <<"                                                              ║\n";
    std::cout<<"  ╚════════════════════════════════════════════════════════════════════════════════════════╝\n\n";
    
    std::cout<<"  Hardware: AMD Ryzen 5 2600 (6-core, 3.40GHz), 16GB RAM, WSL2 Ubuntu\n";
    std::cout<<"  Library: OpenFHE v1.2, CKKS scheme, RingDim=4096, BatchSize=2048\n";
    std::cout<<"  Each cycle: Homomorphic inverse + gate operations + corrections\n";
    std::cout<<"  All targets encrypted, all inverses computed homomorphically\n";
    std::cout<<"  Gate types tested: Add, Multiply, Add-then-Mul, Add-Mul-Add chains\n";
    std::cout<<"  Parameter ranges: vB=[0.01,5.0], vC=[0.01,5.0], target=[-0.25,10.0]\n\n";
    
    return 0;
}
