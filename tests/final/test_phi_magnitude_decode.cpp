// MAGNITUDE-BASED DECODE: b after mulY distinguishes 0 vs 1
#include <iostream>
#include <iomanip>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include "openfhe.h"
using namespace lbcrypto;

struct PE { Ciphertext<DCRTPoly> a; Ciphertext<DCRTPoly> b; };
PE mulY(CryptoContext<DCRTPoly>& cc, const PE& x) { return {x.b, cc->EvalAdd(x.a, x.b)}; }

double decrypt_val(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, const Ciphertext<DCRTPoly>& c) {
    Plaintext pt; cc->Decrypt(kp.secretKey, c, &pt); return pt->GetCKKSPackedValue()[0].real();
}
PE encrypt_bit(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, int bit) {
    const double PSI=0.6180339887498949;
    double v=(bit==1)?1.0:0.0;
    return {cc->Encrypt(kp.publicKey,cc->MakeCKKSPackedPlaintext(std::vector<double>{v+PSI})),
            cc->Encrypt(kp.publicKey,cc->MakeCKKSPackedPlaintext(std::vector<double>{1.0}))};
}
const double PSI=0.6180339887498949;
long long fib(int n){if(n<=1)return n;long long a=0,b=1;for(int i=2;i<=n;i++){long long t=a+b;a=b;b=t;}return b;}

int main(){
    std::cout<<"\n  MAGNITUDE-BASED DECODE: b after mulY separates 0 vs 1\n\n";
    
    CCParams<CryptoContextCKKSRNS> p;
    p.SetMultiplicativeDepth(500);p.SetScalingModSize(50);p.SetBatchSize(2048);
    p.SetRingDim(4096);p.SetSecretKeyDist(UNIFORM_TERNARY);p.SetSecurityLevel(HEStd_NotSet);
    auto cc=GenCryptoContext(p);
    cc->Enable(PKE);cc->Enable(KEYSWITCH);cc->Enable(LEVELEDSHE);cc->Enable(ADVANCEDSHE);
    auto kp=cc->KeyGen();cc->EvalMultKeyGen(kp.secretKey);
    
    int steps=8;
    double mid=(fib(steps+1)+fib(steps)*PSI + fib(steps+1)+fib(steps)*(1.0+PSI))/2.0;
    std::cout<<"  Steps: "<<steps<<" Midpoint: "<<std::fixed<<std::setprecision(1)<<mid<<"\n\n";
    
    int ok=0,total=40;
    std::cout<<"  Bit | b_after_mulY | Expected range | Decoded | Match\n";
    std::cout<<"  --------------------------------------------------\n";
    
    for(int t=0;t<total;t++){
        int bit=rand()%2;
        PE state=encrypt_bit(cc,kp,bit);
        for(int i=0;i<steps;i++) state=mulY(cc,state);
        double b_val=decrypt_val(cc,kp,state.b);
        
        int decoded=(b_val>mid)?1:0;
        if(decoded==bit)ok++;
        
        if(t<15)std::cout<<"  "<<bit<<"   | "
             <<std::fixed<<std::setprecision(4)<<b_val
             <<"        | "
             <<(bit==0?"~76":"~110")
             <<"           | "<<decoded
             <<"      | "<<(decoded==bit?"YES":"NO")<<"\n";
    }
    
    std::cout<<"\n  Magnitude Decode: "<<ok<<"/"<<total<<"\n";
    std::cout<<"  Status: "<<(ok==total?"PERFECT! NO DIVISION NEEDED!":"needs work")<<"\n\n";
    return 0;
}
