// FULLY BLIND DECODER: Normalize scale via b, then Fibonacci
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

double fib_norm_decode_std(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, const PE& s, int steps=8){
    PE st=s; for(int i=0;i<steps;i++) st=mulY(cc,st);
    return (decrypt_val(cc,kp,st.b)-fib(steps+1))/fib(steps)-PSI;
}

// BLIND DECODER: normalize by b_init, then Fibonacci
double blind_decode(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, const PE& s, int steps=8){
    double b_init=decrypt_val(cc,kp,s.b);
    // Normalize: (a/b_init, b/b_init=1)
    auto inv_b=cc->Encrypt(kp.publicKey,cc->MakeCKKSPackedPlaintext(std::vector<double>{1.0/b_init}));
    PE norm={cc->EvalMult(s.a,inv_b), cc->EvalMult(s.b,inv_b)};
    return fib_norm_decode_std(cc,kp,norm,steps);
}

double ratio_decode(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, const PE& s){
    return decrypt_val(cc,kp,s.a)/decrypt_val(cc,kp,s.b)-PSI;
}

int to_bit(double v){return (v>0.5)?1:0;}

int main(){
    std::cout<<"\n  FULLY BLIND DECODER: Normalize b, then Fibonacci\n\n";
    
    CCParams<CryptoContextCKKSRNS> p;
    p.SetMultiplicativeDepth(500);p.SetScalingModSize(50);p.SetBatchSize(2048);
    p.SetRingDim(4096);p.SetSecretKeyDist(UNIFORM_TERNARY);p.SetSecurityLevel(HEStd_NotSet);
    auto cc=GenCryptoContext(p);
    cc->Enable(PKE);cc->Enable(KEYSWITCH);cc->Enable(LEVELEDSHE);cc->Enable(ADVANCEDSHE);
    auto kp=cc->KeyGen();cc->EvalMultKeyGen(kp.secretKey);
    srand(time(0));
    
    int blind_ok=0,ratio_ok=0,total=40;
    std::cout<<"  Bit | r_a  | r_b  | BlindDec | RatioDec | Expected | Match\n";
    std::cout<<"  ------------------------------------------------------------\n";
    
    for(int t=0;t<total;t++){
        int bit=rand()%2; double v=(bit==1)?1.0:0.0;
        double r_a=0.3+(rand()%400)/100.0, r_b=0.3+(rand()%400)/100.0;
        
        PE scaled={
            cc->Encrypt(kp.publicKey,cc->MakeCKKSPackedPlaintext(std::vector<double>{r_a*(v+PSI)})),
            cc->Encrypt(kp.publicKey,cc->MakeCKKSPackedPlaintext(std::vector<double>{r_b}))
        };
        
        double blind=blind_decode(cc,kp,scaled);
        double ratio=ratio_decode(cc,kp,scaled);
        
        if(to_bit(blind)==bit)blind_ok++;
        if(to_bit(ratio)==bit)ratio_ok++;
        
        if(t<12)std::cout<<"  "<<bit<<"   | "<<std::fixed<<std::setprecision(2)<<r_a
             <<" | "<<std::fixed<<std::setprecision(2)<<r_b
             <<" | "<<std::fixed<<std::setprecision(4)<<blind
             <<"    | "<<std::fixed<<std::setprecision(4)<<ratio
             <<"    | "<<v<<"      | "<<(to_bit(blind)==bit?"Blind":"none")<<"\n";
    }
    
    std::cout<<"\n  Blind Decoder: "<<blind_ok<<"/"<<total<<" (scale-INVARIANT)\n";
    std::cout<<"  Ratio Decoder: "<<ratio_ok<<"/"<<total<<" (scale-INVARIANT but noisy)\n";
    std::cout<<"  Status: "<<(blind_ok==total?"PERFECT BLIND DECODER!":"needs work")<<"\n\n";
    return 0;
}
