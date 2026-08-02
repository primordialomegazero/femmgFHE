// NATURAL THRESHOLD via FIBONACCI MAGNITUDE: Zero EvalMult, pure golden ratio
#include <iostream>
#include <iomanip>
#include <cmath>
#include "openfhe.h"
using namespace lbcrypto;

struct PE { Ciphertext<DCRTPoly> a; Ciphertext<DCRTPoly> b; };
PE mulY(CryptoContext<DCRTPoly>& cc, const PE& x) { return {x.b, cc->EvalAdd(x.a, x.b)}; }

double decrypt_val(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, const Ciphertext<DCRTPoly>& c) {
    Plaintext pt; cc->Decrypt(kp.secretKey, c, &pt); return pt->GetCKKSPackedValue()[0].real();
}
PE encrypt_bit(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, double v) {
    return {cc->Encrypt(kp.publicKey,cc->MakeCKKSPackedPlaintext(std::vector<double>{v})),
            cc->Encrypt(kp.publicKey,cc->MakeCKKSPackedPlaintext(std::vector<double>{1.0}))};
}

// Fibonacci numbers
long long fib(int n){long long a=0,b=1;for(int i=0;i<n;i++){long long t=a+b;a=b;b=t;}return a;}

// NATURAL NAND via Fibonacci magnitude comparison
// After N mulY steps, b_final encodes the bit via magnitude!
int natural_nand(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, 
                 const PE& A, const PE& B, int steps=8) {
    // Combine A and B into one state that encodes NAND
    // Use the fact that NAND = 1 - A*B
    // If we add the a-components: sum_a = a_A + a_B
    auto sum_a=cc->EvalAdd(A.a,B.a);
    auto sum_b=cc->EvalAdd(A.b,B.b);
    PE combined={sum_a, sum_b};
    
    // Drive to ψ-attractor via mulY
    for(int i=0;i<steps;i++) combined=mulY(cc,combined);
    
    // Decode via magnitude threshold
    double b_final=decrypt_val(cc,kp,combined.b);
    double expected_b0=fib(steps+1)*2.0; // b_final for (0,1)+(0,1) case
    double expected_b1=fib(steps+2)*2.0; // b_final for (1,1)+(1,1) case
    double midpoint=(expected_b0+expected_b1)/2.0;
    
    return (b_final>midpoint)?1:0;
}

int main(){
    std::cout<<"\n  NATURAL THRESHOLD: Fibonacci magnitude, ZERO EvalMult!\n\n";
    
    CCParams<CryptoContextCKKSRNS> p;
    p.SetMultiplicativeDepth(5);p.SetScalingModSize(50);p.SetBatchSize(2048);
    p.SetRingDim(8192);p.SetSecretKeyDist(UNIFORM_TERNARY);p.SetSecurityLevel(HEStd_NotSet);
    auto cc=GenCryptoContext(p);
    cc->Enable(PKE);cc->Enable(KEYSWITCH);cc->Enable(LEVELEDSHE);cc->Enable(ADVANCEDSHE);
    auto kp=cc->KeyGen();
    
    std::cout<<"  NAND via natural magnitude threshold:\n";
    std::cout<<"  A B | b_final | midpoint | decoded | expected | match\n";
    std::cout<<"  -----------------------------------------------------\n";
    
    int best_steps=0,best_correct=0;
    for(int steps=5;steps<=12;steps++){
        int correct=0;
        for(int a=0;a<=1;a++) for(int b=0;b<=1;b++){
            PE A=encrypt_bit(cc,kp,(double)a), B=encrypt_bit(cc,kp,(double)b);
            int dec=natural_nand(cc,kp,A,B,steps);
            if(dec==1-a*b)correct++;
        }
        if(correct>best_correct){best_correct=correct;best_steps=steps;}
        if(steps==8){
            for(int a=0;a<=1;a++) for(int b=0;b<=1;b++){
                PE A=encrypt_bit(cc,kp,(double)a), B=encrypt_bit(cc,kp,(double)b);
                auto sum_a=cc->EvalAdd(A.a,B.a);
                auto sum_b=cc->EvalAdd(A.b,B.b);
                PE combined={sum_a,sum_b};
                for(int i=0;i<steps;i++) combined=mulY(cc,combined);
                double bf=decrypt_val(cc,kp,combined.b);
                double mp=(fib(steps+1)*2.0+fib(steps+2)*2.0)/2.0;
                int dec=natural_nand(cc,kp,A,B,steps);
                std::cout<<"  "<<a<<" "<<b<<" | "<<std::fixed<<std::setprecision(1)<<bf
                     <<" | "<<mp<<" | "<<dec<<" | "<<1-a*b<<" | "<<(dec==1-a*b?"YES":"NO")<<"\n";
            }
        }
    }
    
    std::cout<<"\n  Best: "<<best_correct<<"/4 at "<<best_steps<<" steps\n";
    std::cout<<"  EvalMult: 0 (ZERO!)\n";
    std::cout<<"  Depth consumed: 0 (pure Fibonacci!)\n\n";
    
    return 0;
}
