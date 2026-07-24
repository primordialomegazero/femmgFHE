// Scale Invariance Test: Fibonacci normalization blind to scaling factor
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
const double PSI=0.6180339887498949;
long long fib(int n){if(n<=1)return n;long long a=0,b=1;for(int i=2;i<=n;i++){long long t=a+b;a=b;b=t;}return b;}

double fib_norm_decode(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, const PE& s, int steps=8){
    PE st=s; for(int i=0;i<steps;i++) st=mulY(cc,st);
    return (decrypt_val(cc,kp,st.b)-fib(steps+1))/fib(steps)-PSI;
}

int main(){
    std::cout<<"\n  SCALE INVARIANCE: Fibonacci Normalization Blind Decoding\n";
    std::cout<<"  Test: PE_original vs PE_scaled (random r) -> same fib_norm_decode?\n\n";
    
    CCParams<CryptoContextCKKSRNS> p;
    p.SetMultiplicativeDepth(500);p.SetScalingModSize(50);p.SetBatchSize(2048);
    p.SetRingDim(4096);p.SetSecretKeyDist(UNIFORM_TERNARY);p.SetSecurityLevel(HEStd_NotSet);
    auto cc=GenCryptoContext(p);
    cc->Enable(PKE);cc->Enable(KEYSWITCH);cc->Enable(LEVELEDSHE);cc->Enable(ADVANCEDSHE);
    auto kp=cc->KeyGen();cc->EvalMultKeyGen(kp.secretKey);
    srand(time(0));
    
    std::cout<<"  Value | Scale r | Original Decode | Scaled Decode | Match\n";
    std::cout<<"  ---------------------------------------------------------\n";
    
    int matches=0, total=0;
    for(int bit=0;bit<=1;bit++){
        for(int t=0;t<5;t++){
            double v=(bit==1)?1.0:0.0;
            double r=0.5+(rand()%200)/100.0; // random 0.5 to 2.5
            
            // PE_original: (v+psi, 1)
            PE orig={
                cc->Encrypt(kp.publicKey,cc->MakeCKKSPackedPlaintext(std::vector<double>{v+PSI})),
                cc->Encrypt(kp.publicKey,cc->MakeCKKSPackedPlaintext(std::vector<double>{1.0}))
            };
            
            // PE_scaled: (r*(v+psi), r)
            PE scaled={
                cc->Encrypt(kp.publicKey,cc->MakeCKKSPackedPlaintext(std::vector<double>{r*(v+PSI)})),
                cc->Encrypt(kp.publicKey,cc->MakeCKKSPackedPlaintext(std::vector<double>{r}))
            };
            
            double orig_dec=fib_norm_decode(cc,kp,orig);
            double scaled_dec=fib_norm_decode(cc,kp,scaled);
            
            bool match=std::abs(orig_dec-scaled_dec)<0.1;
            if(match)matches++;
            total++;
            
            std::cout<<"  "<<std::fixed<<std::setprecision(0)<<v
                 <<"     | "<<std::fixed<<std::setprecision(2)<<std::setw(6)<<r
                 <<" | "<<std::fixed<<std::setprecision(6)<<orig_dec
                 <<"         | "<<std::fixed<<std::setprecision(6)<<scaled_dec
                 <<"        | "<<(match?"YES":"NO")<<"\n";
        }
    }
    
    std::cout<<"\n  Scale Invariance: "<<matches<<"/"<<total<<" correct\n";
    std::cout<<"  Fibonacci normalization is "<<(matches==total?"SCALE-INVARIANT!":"NOT scale-invariant")<<"\n";
    std::cout<<"  Blind decoding "<<(matches==total?"WORKS":"NEEDS FIX")<<" without knowing r!\n\n";
    
    return 0;
}
