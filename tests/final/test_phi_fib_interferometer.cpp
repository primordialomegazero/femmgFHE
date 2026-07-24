// FIBONACCI-WEIGHTED INTERFEROMETER: Golden ratio anchored
#include <iostream>
#include <iomanip>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include "openfhe.h"
using namespace lbcrypto;

struct PE { Ciphertext<DCRTPoly> a; Ciphertext<DCRTPoly> b; };
PE mulY(CryptoContext<DCRTPoly>& cc, const PE& x) { return {x.b, cc->EvalAdd(x.a, x.b)}; }
PE mulY_inv(CryptoContext<DCRTPoly>& cc, const PE& x) { return {cc->EvalSub(x.b, x.a), x.a}; }

double decrypt_val(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, const Ciphertext<DCRTPoly>& c) {
    Plaintext pt; cc->Decrypt(kp.secretKey, c, &pt); return pt->GetCKKSPackedValue()[0].real();
}
const double PSI=0.6180339887498949;
long long fib(int n){if(n<=1)return n;long long a=0,b=1;for(int i=2;i<=n;i++){long long t=a+b;a=b;b=t;}return b;}

int to_bit(double v){return (v>0.5)?1:0;}

int main(){
    std::cout<<"\n  FIBONACCI-WEIGHTED INTERFEROMETER\n";
    std::cout<<"  Fibonacci coefficients reconcile forward & reverse paths\n\n";
    
    CCParams<CryptoContextCKKSRNS> p;
    p.SetMultiplicativeDepth(500);p.SetScalingModSize(50);p.SetBatchSize(2048);
    p.SetRingDim(4096);p.SetSecretKeyDist(UNIFORM_TERNARY);p.SetSecurityLevel(HEStd_NotSet);
    auto cc=GenCryptoContext(p);
    cc->Enable(PKE);cc->Enable(KEYSWITCH);cc->Enable(LEVELEDSHE);cc->Enable(ADVANCEDSHE);
    auto kp=cc->KeyGen();cc->EvalMultKeyGen(kp.secretKey);
    srand(time(0));
    
    int N=5; // Fibonacci steps
    int ok=0,total=30;
    std::cout<<"  Bit | Forward | Reverse | Fib-Weighted | Expected | Match\n";
    std::cout<<"  --------------------------------------------------------\n";
    
    for(int t=0;t<total;t++){
        int bit=rand()%2; double v=(bit==1)?1.0:0.0;
        double theta=(rand()%6283)/1000.0;
        double a0=v+PSI, b0=1.0;
        double a_rot=a0*cos(theta)-b0*sin(theta);
        double b_rot=a0*sin(theta)+b0*cos(theta);
        
        PE state={
            cc->Encrypt(kp.publicKey,cc->MakeCKKSPackedPlaintext(std::vector<double>{a_rot})),
            cc->Encrypt(kp.publicKey,cc->MakeCKKSPackedPlaintext(std::vector<double>{b_rot}))
        };
        
        // Forward N steps
        PE fwd=state; for(int i=0;i<N;i++) fwd=mulY(cc,fwd);
        double aF=decrypt_val(cc,kp,fwd.a), bF=decrypt_val(cc,kp,fwd.b);
        
        // Reverse N steps
        PE rev=state; for(int i=0;i<N;i++) rev=mulY_inv(cc,rev);
        double aR=decrypt_val(cc,kp,rev.a), bR=decrypt_val(cc,kp,rev.b);
        
        // Fibonacci-weighted: Fib(N-1)*Forward + Fib(N)*Reverse
        double aW=fib(N-1)*aF + fib(N)*aR;
        double bW=fib(N-1)*bF + fib(N)*bR;
        double decoded=aW/bW - PSI;
        
        if(to_bit(decoded)==bit)ok++;
        
        if(t<10)std::cout<<"  "<<bit<<"   | "
             <<std::fixed<<std::setprecision(4)<<(aF/bF-PSI)<<"  | "
             <<std::fixed<<std::setprecision(4)<<(aR/bR-PSI)<<"   | "
             <<std::fixed<<std::setprecision(4)<<decoded
             <<"        | "<<v<<"      | "<<(to_bit(decoded)==bit?"YES":"NO")<<"\n";
    }
    
    std::cout<<"\n  Fibonacci-Weighted Recovery: "<<ok<<"/"<<total<<"\n";
    std::cout<<"  Status: "<<(ok==total?"FIBONACCI RECONCILES ALL PATHS!":"needs work")<<"\n\n";
    return 0;
}
