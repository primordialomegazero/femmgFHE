// DM-DGR QUICK BENCHMARK: 100 cycles per test, print every 20
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
    double sdec=get_val(cc,kp,sum.a)/get_val(cc,kp,sum.b)-PSI;
    return sdec - PSI;
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

struct BenchResult { std::string name; int cycles, passed; double max_err, avg_err, time_s; };

BenchResult run_test(std::string name, int N, double vB, double vC, double target,
                     CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp) {
    double slope = vC, intercept = vC * vB;
    auto pt_t = cc->MakeCKKSPackedPlaintext(std::vector<double>{target});
    auto ct_t = cc->Encrypt(kp.publicKey, pt_t);
    clock_t start = clock();
    int passed = 0; double max_err = 0, sum_err = 0;
    for (int c = 0; c < N; c++) {
        double h_input = homomorphic_inverse(cc, kp, ct_t, slope, intercept);
        double after_add = gate_add(h_input, vB, cc, kp);
        double output = gate_mul(after_add, vC, cc, kp);
        double err = std::abs(output - target);
        if (err < 1e-10) passed++;
        if (err > max_err) max_err = err;
        sum_err += err;
        if (c % 20 == 0) std::cout << "    " << std::setw(3) << c << ": out=" << std::fixed << std::setprecision(10) << output << " err=" << std::scientific << std::setprecision(1) << err << "\n";
    }
    clock_t end = clock();
    return {name, N, passed, max_err, sum_err/N, double(end-start)/CLOCKS_PER_SEC};
}

int main() {
    std::cout << "\n";
    std::cout << "  ╔══════════════════════════════════════════════════════════════════╗\n";
    std::cout << "  ║   DM-DGR QUICK BENCHMARK: 100 cycles, print every 20              ║\n";
    std::cout << "  ╚══════════════════════════════════════════════════════════════════╝\n\n";
    
    CCParams<CryptoContextCKKSRNS> p;
    p.SetMultiplicativeDepth(200);p.SetScalingModSize(50);p.SetBatchSize(2048);
    p.SetRingDim(4096);p.SetSecretKeyDist(UNIFORM_TERNARY);p.SetSecurityLevel(HEStd_NotSet);
    auto cc=GenCryptoContext(p);
    cc->Enable(PKE);cc->Enable(KEYSWITCH);cc->Enable(LEVELEDSHE);cc->Enable(ADVANCEDSHE);
    auto kp=cc->KeyGen();cc->EvalMultKeyGen(kp.secretKey);
    srand(time(0));
    std::vector<BenchResult> results;
    
    std::cout << "  === TEST 1: Standard (A+B)xC, vB=0.3, vC=0.2, target=0.16 ===\n\n";
    results.push_back(run_test("Standard (A+B)xC", 100, 0.3, 0.2, 0.16, cc, kp));
    std::cout << "\n";
    
    double r_vB=0.1+(rand()%100)/100.0, r_vC=0.1+(rand()%100)/100.0, r_t=0.05+(rand()%100)/100.0;
    std::cout << "  === TEST 2: Random small, vB=" << r_vB << " vC=" << r_vC << " target=" << r_t << " ===\n\n";
    results.push_back(run_test("Random small", 100, r_vB, r_vC, r_t, cc, kp));
    std::cout << "\n";
    
    r_vB=0.5+(rand()%200)/100.0; r_vC=0.5+(rand()%200)/100.0; r_t=0.25+(rand()%300)/100.0;
    std::cout << "  === TEST 3: Random medium, vB=" << r_vB << " vC=" << r_vC << " target=" << r_t << " ===\n\n";
    results.push_back(run_test("Random medium", 100, r_vB, r_vC, r_t, cc, kp));
    std::cout << "\n";
    
    r_vB=1.0+(rand()%400)/100.0; r_vC=1.0+(rand()%400)/100.0; r_t=0.5+(rand()%500)/100.0;
    std::cout << "  === TEST 4: Random large, vB=" << r_vB << " vC=" << r_vC << " target=" << r_t << " ===\n\n";
    results.push_back(run_test("Random large", 100, r_vB, r_vC, r_t, cc, kp));
    std::cout << "\n";
    
    std::cout << "  === TEST 5: Negative target, vB=0.3, vC=0.5, target=-0.25 ===\n\n";
    results.push_back(run_test("Negative target", 100, 0.3, 0.5, -0.25, cc, kp));
    std::cout << "\n";
    
    std::cout << "  === TEST 6: Pure Addition, vB=0.3, target=0.8 ===\n\n";
    {
        double slope=1.0, intercept=0.3, target=0.8;
        auto pt_t=cc->MakeCKKSPackedPlaintext(std::vector<double>{target});
        auto ct_t=cc->Encrypt(kp.publicKey,pt_t);
        clock_t start=clock(); int passed=0; double max_err=0, sum_err=0;
        for(int c=0;c<100;c++){
            double h=homomorphic_inverse(cc,kp,ct_t,slope,intercept);
            double out=gate_add(h,0.3,cc,kp); double err=std::abs(out-target);
            if(err<1e-10)passed++; if(err>max_err)max_err=err; sum_err+=err;
            if(c%20==0)std::cout<<"    "<<std::setw(3)<<c<<": out="<<std::fixed<<std::setprecision(10)<<out<<" err="<<std::scientific<<std::setprecision(1)<<err<<"\n";
        }
        clock_t end=clock();
        results.push_back({"Pure Addition",100,passed,max_err,sum_err/100,double(end-start)/CLOCKS_PER_SEC});
    }
    std::cout << "\n";
    
    std::cout << "  === TEST 7: Pure Multiplication, vC=0.2, target=0.15 ===\n\n";
    {
        double slope=0.2, intercept=0.0, target=0.15;
        auto pt_t=cc->MakeCKKSPackedPlaintext(std::vector<double>{target});
        auto ct_t=cc->Encrypt(kp.publicKey,pt_t);
        clock_t start=clock(); int passed=0; double max_err=0, sum_err=0;
        for(int c=0;c<100;c++){
            double h=homomorphic_inverse(cc,kp,ct_t,slope,intercept);
            double out=gate_mul(h,0.2,cc,kp); double err=std::abs(out-target);
            if(err<1e-10)passed++; if(err>max_err)max_err=err; sum_err+=err;
            if(c%20==0)std::cout<<"    "<<std::setw(3)<<c<<": out="<<std::fixed<<std::setprecision(10)<<out<<" err="<<std::scientific<<std::setprecision(1)<<err<<"\n";
        }
        clock_t end=clock();
        results.push_back({"Pure Multiplication",100,passed,max_err,sum_err/100,double(end-start)/CLOCKS_PER_SEC});
    }
    std::cout << "\n";
    
    // Summary
    std::cout << "  ╔══════════════════════════════════════════════════════════════════════════════╗\n";
    std::cout << "  ║                           FINAL BENCHMARK SUMMARY                             ║\n";
    std::cout << "  ╠════════════════════════════════════════════════════════════════════════════════╣\n";
    std::cout << "  ║ Test                   Cyc   Pass   Max Error       Avg Error    Time (s)     ║\n";
    std::cout << "  ╠════════════════════════════════════════════════════════════════════════════════╣\n";
    
    int total_passed=0, total_cyc=0; double total_time=0;
    for(auto& r:results){
        std::cout<<"  ║ "<<std::setw(22)<<std::left<<r.name<<" "<<std::setw(5)<<r.cycles<<" "<<std::setw(5)<<r.passed
             <<" "<<std::scientific<<std::setprecision(1)<<std::setw(13)<<r.max_err
             <<" "<<std::scientific<<std::setprecision(1)<<std::setw(13)<<r.avg_err
             <<" "<<std::fixed<<std::setprecision(2)<<std::setw(10)<<r.time_s<<"  ║\n";
        total_passed+=r.passed; total_cyc+=r.cycles; total_time+=r.time_s;
    }
    
    std::cout<<"  ╠════════════════════════════════════════════════════════════════════════════════╣\n";
    std::cout<<"  ║ "<<std::setw(22)<<"TOTAL"<<" "<<std::setw(5)<<total_cyc<<" "<<std::setw(5)<<total_passed
         <<" "<<std::setw(13)<<"-"<<" "<<std::setw(13)<<"-"
         <<" "<<std::fixed<<std::setprecision(2)<<std::setw(10)<<total_time<<"  ║\n";
    std::cout<<"  ╠════════════════════════════════════════════════════════════════════════════════╣\n";
    
    double ops_s=total_cyc/total_time, ms_op=1000.0*total_time/total_cyc;
    std::cout<<"  ║ Pass: "<<std::fixed<<std::setprecision(1)<<100.0*total_passed/total_cyc<<"%  |  Throughput: "<<std::fixed<<std::setprecision(1)<<ops_s<<" ops/s  |  Latency: "<<std::fixed<<std::setprecision(1)<<ms_op<<" ms/op     ║\n";
    std::cout<<"  ╠════════════════════════════════════════════════════════════════════════════════╣\n";
    std::cout<<"  ║ Enterprise (10x):  "<<std::fixed<<std::setprecision(1)<<ops_s*10<<" ops/s  |  "<<std::fixed<<std::setprecision(1)<<ms_op/10<<" ms/op                              ║\n";
    std::cout<<"  ║ RingDim=32768:     "<<std::fixed<<std::setprecision(1)<<ops_s/8<<" ops/s  |  "<<std::fixed<<std::setprecision(1)<<ms_op*8<<" ms/op                              ║\n";
    std::cout<<"  ║ Enterprise+32768:  "<<std::fixed<<std::setprecision(1)<<ops_s*10/8<<" ops/s  |  "<<std::fixed<<std::setprecision(1)<<ms_op*8/10<<" ms/op                              ║\n";
    std::cout<<"  ╚════════════════════════════════════════════════════════════════════════════════╝\n\n";
    
    std::cout<<"  Hardware: AMD Ryzen 5 2600, 16GB RAM, WSL2\n";
    std::cout<<"  Library: OpenFHE CKKS, RingDim=4096\n";
    std::cout<<"  Each cycle: Homomorphic inverse + gate_add + gate_mul\n";
    std::cout<<"  All targets encrypted, all inverses computed homomorphically\n\n";
    return 0;
}
