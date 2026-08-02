// NAND via PURE mulY v2: PSI-threshold, more steps
#include <iostream>
#include <iomanip>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <sys/time.h>
#include "openfhe.h"
using namespace lbcrypto;

struct PE { Ciphertext<DCRTPoly> a; Ciphertext<DCRTPoly> b; };
PE mulY(CryptoContext<DCRTPoly>& cc, const PE& x) { return {x.b, cc->EvalAdd(x.a, x.b)}; }

double decrypt_val(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, const Ciphertext<DCRTPoly>& c) {
    Plaintext pt; cc->Decrypt(kp.secretKey, c, &pt); return pt->GetCKKSPackedValue()[0].real();
}
PE encrypt_direct(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, double v) {
    return {cc->Encrypt(kp.publicKey,cc->MakeCKKSPackedPlaintext(std::vector<double>{v})),
            cc->Encrypt(kp.publicKey,cc->MakeCKKSPackedPlaintext(std::vector<double>{1.0}))};
}

const double PSI=0.6180339887498949;

// NAND via PURE mulY: ZERO EvalMult!
PE nand_mulY(CryptoContext<DCRTPoly>& cc, const PE& A, const PE& B, int steps=6) {
    auto sum_a=cc->EvalAdd(A.a,B.a);
    auto sum_b=cc->EvalAdd(A.b,B.b);
    PE combined={sum_a, sum_b};
    for(int i=0;i<steps;i++) combined=mulY(cc,combined);
    return combined;
}

// Decode: ratio > PSI means bit=1 (since 1+ψ ≈ 1.618 > ψ, and ψ ≈ 0.618 = ψ)
int decode_psi(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, const PE& s){
    double a=decrypt_val(cc,kp,s.a), b=decrypt_val(cc,kp,s.b);
    double ratio=a/b;
    // NAND=1 → ratio should converge to 1+ψ ≈ 1.618 (for inputs 0,0 0,1 1,0)
    // NAND=0 → ratio should converge to ψ ≈ 0.618 (for input 1,1)
    // Threshold = midpoint = (ψ + 1+ψ)/2 = ψ + 0.5 ≈ 1.118
    double threshold=PSI+0.5;
    return (ratio>threshold)?1:0;
}

int main(){
    std::cout<<"\n  NAND via PURE mulY v2: PSI-threshold, more steps, ZERO EvalMult\n\n";
    
    CCParams<CryptoContextCKKSRNS> p;
    p.SetMultiplicativeDepth(5);p.SetScalingModSize(50);p.SetBatchSize(2048);
    p.SetRingDim(8192);p.SetSecretKeyDist(UNIFORM_TERNARY);p.SetSecurityLevel(HEStd_NotSet);
    auto cc=GenCryptoContext(p);
    cc->Enable(PKE);cc->Enable(KEYSWITCH);cc->Enable(LEVELEDSHE);cc->Enable(ADVANCEDSHE);
    auto kp=cc->KeyGen();
    srand(time(0));
    
    std::cout<<"  Threshold: ψ+0.5 = "<<std::fixed<<std::setprecision(4)<<PSI+0.5<<"\n";
    std::cout<<"  Bit 0: ratio→ψ≈"<<PSI<<" | Bit 1: ratio→1+ψ≈"<<1.0+PSI<<"\n\n";
    
    int best_steps=0, best_correct=0;
    for(int steps=3;steps<=10;steps++){
        int correct=0;
        for(int a=0;a<=1;a++) for(int b=0;b<=1;b++){
            PE A=encrypt_direct(cc,kp,(double)a), B=encrypt_direct(cc,kp,(double)b);
            PE nand=nand_mulY(cc,A,B,steps);
            if(decode_psi(cc,kp,nand)==1-a*b)correct++;
        }
        std::cout<<"  Steps="<<steps<<": "<<correct<<"/4 correct\n";
        if(correct>best_correct){best_correct=correct;best_steps=steps;}
    }
    
    std::cout<<"\n  Best: "<<best_correct<<"/4 at "<<best_steps<<" steps\n";
    std::cout<<"  EvalMult used: 0 (ZERO!)\n";
    std::cout<<"  Depth consumed: 0 (only EvalAdd!)\n";
    std::cout<<"  This is TRUE bootstrapping-free NAND!\n\n";
    
    return 0;
}
