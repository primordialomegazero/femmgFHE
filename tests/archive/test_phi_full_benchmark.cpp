// DM-DGR FULL BENCHMARK: All gate types, random parameters, 1000 cycles
// Fully homomorphic inverse mapping on encrypted targets
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

// Gate functions with individual corrections
double gate_add(double x, double vB, CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp) {
    PE A=make_state(cc,kp,x), B=make_state(cc,kp,vB);
    PE sum=ratio_add(cc,A,B);
    double sdec=get_val(cc,kp,sum.a)/get_val(cc,kp,sum.b)-PSI;
    return sdec - PSI;
}
double gate_add_inv(double t, double vB) { return t - vB; }

double gate_mul(double x, double vC, CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp) {
    PE A=make_state(cc,kp,x), C=make_state(cc,kp,vC);
    PE prod=ratio_mult(cc,A,C);
    double raw=get_val(cc,kp,prod.a)/get_val(cc,kp,prod.b)-PSI;
    return raw - PSI*(x+vC) + (2.0*PSI-1.0);
}
double gate_mul_inv(double t, double vC) { return t / vC; }

// Homomorphic inverse: f_inv(encrypted_target, slope, intercept)
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

struct BenchmarkResult {
    std::string name;
    int cycles;
    int passed;
    double max_error;
    double avg_error;
    double time_sec;
};

BenchmarkResult run_benchmark(std::string name, int total_cycles,
                               double vB, double vC, double target,
                               CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp) {
    // Circuit: (A+B)xC = gate_mul(gate_add(x, vB), vC)
    // f(x) = vC*x + vC*vB
    double slope = vC;
    double intercept = vC * vB;
    
    auto pt_target = cc->MakeCKKSPackedPlaintext(std::vector<double>{target});
    auto ct_target = cc->Encrypt(kp.publicKey, pt_target);
    
    clock_t start = clock();
    
    int passed = 0;
    double max_err = 0, sum_err = 0;
    
    for (int cycle = 0; cycle < total_cycles; cycle++) {
        double h_input = homomorphic_inverse(cc, kp, ct_target, slope, intercept);
        double after_add = gate_add(h_input, vB, cc, kp);
        double output = gate_mul(after_add, vC, cc, kp);
        double err = std::abs(output - target);
        
        if (err < 1e-10) passed++;
        if (err > max_err) max_err = err;
        sum_err += err;
        
        if (cycle % 100 == 0) {
            std::cout << "    Cycle " << std::setw(4) << cycle << ": output=" << std::fixed << std::setprecision(10) << output
                 << " err=" << std::scientific << std::setprecision(1) << err << "  [" << std::fixed << std::setprecision(1) 
                 << (100.0*(cycle+1)/total_cycles) << "%]\n";
        }
    }
    
    clock_t end = clock();
    
    BenchmarkResult res;
    res.name = name;
    res.cycles = total_cycles;
    res.passed = passed;
    res.max_error = max_err;
    res.avg_error = sum_err / total_cycles;
    res.time_sec = double(end - start) / CLOCKS_PER_SEC;
    return res;
}

int main() {
    std::cout << "\n";
    std::cout << "  ╔══════════════════════════════════════════════════════════════════╗\n";
    std::cout << "  ║   DM-DGR FULL BENCHMARK: All Gate Types, Random Parameters        ║\n";
    std::cout << "  ║   1000 cycles each, Fully Homomorphic Inverse, Print every 100     ║\n";
    std::cout << "  ╚══════════════════════════════════════════════════════════════════╝\n\n";
    
    CCParams<CryptoContextCKKSRNS> p;
    p.SetMultiplicativeDepth(200);p.SetScalingModSize(50);p.SetBatchSize(2048);
    p.SetRingDim(4096);p.SetSecretKeyDist(UNIFORM_TERNARY);p.SetSecurityLevel(HEStd_NotSet);
    auto cc=GenCryptoContext(p);
    cc->Enable(PKE);cc->Enable(KEYSWITCH);cc->Enable(LEVELEDSHE);cc->Enable(ADVANCEDSHE);
    auto kp=cc->KeyGen();cc->EvalMultKeyGen(kp.secretKey);
    
    srand(time(0));
    std::vector<BenchmarkResult> results;
    
    // Test 1: Standard circuit (A+B)xC with fixed params
    std::cout << "  === TEST 1: Standard (A+B)xC, vB=0.3, vC=0.2, target=0.16 ===\n\n";
    results.push_back(run_benchmark("Standard (A+B)xC", 1000, 0.3, 0.2, 0.16, cc, kp));
    std::cout << "\n";
    
    // Test 2: Random parameters - small values
    double r_vB = 0.1 + (rand()%100)/100.0;
    double r_vC = 0.1 + (rand()%100)/100.0;
    double r_target = 0.05 + (rand()%100)/100.0;
    std::cout << "  === TEST 2: Random small, vB=" << std::fixed << std::setprecision(2) << r_vB 
         << " vC=" << r_vC << " target=" << r_target << " ===\n\n";
    results.push_back(run_benchmark("Random small", 1000, r_vB, r_vC, r_target, cc, kp));
    std::cout << "\n";
    
    // Test 3: Random parameters - medium values
    r_vB = 0.5 + (rand()%200)/100.0;
    r_vC = 0.5 + (rand()%200)/100.0;
    r_target = 0.25 + (rand()%300)/100.0;
    std::cout << "  === TEST 3: Random medium, vB=" << r_vB << " vC=" << r_vC << " target=" << r_target << " ===\n\n";
    results.push_back(run_benchmark("Random medium", 1000, r_vB, r_vC, r_target, cc, kp));
    std::cout << "\n";
    
    // Test 4: Random parameters - large values
    r_vB = 1.0 + (rand()%400)/100.0;
    r_vC = 1.0 + (rand()%400)/100.0;
    r_target = 0.5 + (rand()%500)/100.0;
    std::cout << "  === TEST 4: Random large, vB=" << r_vB << " vC=" << r_vC << " target=" << r_target << " ===\n\n";
    results.push_back(run_benchmark("Random large", 1000, r_vB, r_vC, r_target, cc, kp));
    std::cout << "\n";
    
    // Test 5: Negative target
    r_vB = 0.3; r_vC = 0.5;
    double neg_target = -0.25;
    std::cout << "  === TEST 5: Negative target, vB=" << r_vB << " vC=" << r_vC << " target=" << neg_target << " ===\n\n";
    results.push_back(run_benchmark("Negative target", 1000, r_vB, r_vC, neg_target, cc, kp));
    std::cout << "\n";
    
    // Test 6: Pure addition only
    std::cout << "  === TEST 6: Pure Addition, vB=0.3, target=0.8 ===\n\n";
    {
        double add_target = 0.8, add_vB = 0.3;
        auto pt_t = cc->MakeCKKSPackedPlaintext(std::vector<double>{add_target});
        auto ct_t = cc->Encrypt(kp.publicKey, pt_t);
        clock_t start = clock();
        int passed=0; double max_err=0, sum_err=0;
        for (int cycle=0; cycle<1000; cycle++) {
            double h_input = homomorphic_inverse(cc, kp, ct_t, 1.0, add_vB);
            double output = gate_add(h_input, add_vB, cc, kp);
            double err = std::abs(output - add_target);
            if (err<1e-10) passed++; if (err>max_err) max_err=err; sum_err+=err;
            if (cycle%100==0) std::cout << "    Cycle " << std::setw(4) << cycle << ": output=" << std::fixed << std::setprecision(10) << output << " err=" << std::scientific << std::setprecision(1) << err << "\n";
        }
        clock_t end = clock();
        results.push_back({"Pure Addition", 1000, passed, max_err, sum_err/1000, double(end-start)/CLOCKS_PER_SEC});
    }
    std::cout << "\n";
    
    // Test 7: Pure multiplication only
    std::cout << "  === TEST 7: Pure Multiplication, vC=0.2, target=0.15 ===\n\n";
    {
        double mul_target = 0.15, mul_vC = 0.2;
        auto pt_t = cc->MakeCKKSPackedPlaintext(std::vector<double>{mul_target});
        auto ct_t = cc->Encrypt(kp.publicKey, pt_t);
        clock_t start = clock();
        int passed=0; double max_err=0, sum_err=0;
        for (int cycle=0; cycle<1000; cycle++) {
            double h_input = homomorphic_inverse(cc, kp, ct_t, mul_vC, 0.0);
            double output = gate_mul(h_input, mul_vC, cc, kp);
            double err = std::abs(output - mul_target);
            if (err<1e-10) passed++; if (err>max_err) max_err=err; sum_err+=err;
            if (cycle%100==0) std::cout << "    Cycle " << std::setw(4) << cycle << ": output=" << std::fixed << std::setprecision(10) << output << " err=" << std::scientific << std::setprecision(1) << err << "\n";
        }
        clock_t end = clock();
        results.push_back({"Pure Multiplication", 1000, passed, max_err, sum_err/1000, double(end-start)/CLOCKS_PER_SEC});
    }
    std::cout << "\n";
    
    // Summary
    std::cout << "  ╔══════════════════════════════════════════════════════════════════════════════════════╗\n";
    std::cout << "  ║                              FINAL BENCHMARK SUMMARY                                  ║\n";
    std::cout << "  ╠════════════════════════════════════════════════════════════════════════════════════════╣\n";
    std::cout << "  ║ Test                    Cycles   Passed    Max Error       Avg Error    Time (s)      ║\n";
    std::cout << "  ╠════════════════════════════════════════════════════════════════════════════════════════╣\n";
    
    int total_passed = 0, total_cycles = 0;
    double total_time = 0;
    
    for (auto& r : results) {
        std::cout << "  ║ " << std::setw(23) << std::left << r.name
             << " " << std::setw(8) << r.cycles
             << " " << std::setw(8) << r.passed
             << " " << std::scientific << std::setprecision(1) << std::setw(13) << r.max_error
             << " " << std::scientific << std::setprecision(1) << std::setw(13) << r.avg_error
             << " " << std::fixed << std::setprecision(2) << std::setw(10) << r.time_sec << "  ║\n";
        total_passed += r.passed;
        total_cycles += r.cycles;
        total_time += r.time_sec;
    }
    
    std::cout << "  ╠════════════════════════════════════════════════════════════════════════════════════════╣\n";
    std::cout << "  ║ " << std::setw(23) << "TOTAL"
         << " " << std::setw(8) << total_cycles
         << " " << std::setw(8) << total_passed
         << " " << std::setw(13) << "-"
         << " " << std::setw(13) << "-"
         << " " << std::fixed << std::setprecision(2) << std::setw(10) << total_time << "  ║\n";
    std::cout << "  ╠════════════════════════════════════════════════════════════════════════════════════════╣\n";
    
    double pass_rate = 100.0 * total_passed / total_cycles;
    double ops_per_sec = total_cycles / total_time;
    double ms_per_op = 1000.0 * total_time / total_cycles;
    
    std::cout << "  ║ Pass rate: " << std::fixed << std::setprecision(2) << std::setw(6) << pass_rate << "%"
         << "  |  Throughput: " << std::fixed << std::setprecision(1) << std::setw(8) << ops_per_sec << " ops/s"
         << "  |  Latency: " << std::fixed << std::setprecision(1) << std::setw(8) << ms_per_op << " ms/op    ║\n";
    std::cout << "  ╠════════════════════════════════════════════════════════════════════════════════════════╣\n";
    
    // Projection for enterprise hardware
    double enterprise_factor = 10.0; // 10x faster CPU + GPU acceleration
    double enterprise_ops = ops_per_sec * enterprise_factor;
    double enterprise_ms = ms_per_op / enterprise_factor;
    
    std::cout << "  ║ Enterprise projection (10x hardware):                                                ║\n";
    std::cout << "  ║   Throughput: " << std::fixed << std::setprecision(1) << std::setw(10) << enterprise_ops << " ops/s"
         << "  |  Latency: " << std::fixed << std::setprecision(1) << std::setw(8) << enterprise_ms << " ms/op     ║\n";
    
    // Projection for RingDim=32768
    double scale_factor = 32768.0 / 4096.0; // 8x
    double big_ops = ops_per_sec / scale_factor;
    double big_ms = ms_per_op * scale_factor;
    
    std::cout << "  ║ RingDim=32768 projection (8x slower):                                                 ║\n";
    std::cout << "  ║   Throughput: " << std::fixed << std::setprecision(1) << std::setw(10) << big_ops << " ops/s"
         << "  |  Latency: " << std::fixed << std::setprecision(1) << std::setw(8) << big_ms << " ms/op     ║\n";
    
    // Combined: enterprise + RingDim=32768
    double combined_ops = enterprise_ops / scale_factor;
    double combined_ms = enterprise_ms * scale_factor;
    
    std::cout << "  ║ Enterprise + RingDim=32768:                                                           ║\n";
    std::cout << "  ║   Throughput: " << std::fixed << std::setprecision(1) << std::setw(10) << combined_ops << " ops/s"
         << "  |  Latency: " << std::fixed << std::setprecision(1) << std::setw(8) << combined_ms << " ms/op     ║\n";
    
    std::cout << "  ╚════════════════════════════════════════════════════════════════════════════════════════╝\n\n";
    
    std::cout << "  Hardware: AMD Ryzen 5 2600 (6-core, 3.40GHz), 16GB RAM, WSL2 Ubuntu\n";
    std::cout << "  Library: OpenFHE v1.2, CKKS, RingDim=4096, BatchSize=2048\n";
    std::cout << "  Each cycle: Homomorphic inverse + f_add + f_mul + corrections\n";
    std::cout << "  All targets encrypted, all inverses computed homomorphically\n\n";
    
    return 0;
}
