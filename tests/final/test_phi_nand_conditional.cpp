// NAND via CONDITIONAL mulY: Use B as switch, apply mulY to A conditionally
#include <iostream>
#include <iomanip>
#include <cmath>
#include "openfhe.h"
using namespace lbcrypto;

struct PE { Ciphertext<DCRTPoly> a; Ciphertext<DCRTPoly> b; };
PE mulY(CryptoContext<DCRTPoly>& cc, const PE& x) { return {x.b, cc->EvalAdd(x.a, x.b)}; }
PE mulY_inv(CryptoContext<DCRTPoly>& cc, const PE& x) { return {cc->EvalSub(x.b, x.a), x.a}; }

double decrypt_val(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, const Ciphertext<DCRTPoly>& c) {
    Plaintext pt; cc->Decrypt(kp.secretKey, c, &pt); return pt->GetCKKSPackedValue()[0].real();
}
PE encrypt_direct(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, double v) {
    return {cc->Encrypt(kp.publicKey,cc->MakeCKKSPackedPlaintext(std::vector<double>{v})),
            cc->Encrypt(kp.publicKey,cc->MakeCKKSPackedPlaintext(std::vector<double>{1.0}))};
}
const double PSI=0.6180339887498949;
int decode_bit(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, const PE& s){
    double a=decrypt_val(cc,kp,s.a), b=decrypt_val(cc,kp,s.b);
    return (a>b*0.5)?1:0;
}

// WILD IDEA: Encode bits as (0,1) for 0, (1,1) for 1
// NOT(A) = swap the encoding!
// Can mulY do this? After 1 mulY: (0,1)→(1,1) and (1,1)→(1,2)
// (0,1) ratio=0 → (1,1) ratio=1 → SWAP! 
// (1,1) ratio=1 → (1,2) ratio=0.5 → NOT quite 0 but close!
PE not_via_mulY(CryptoContext<DCRTPoly>& cc, const PE& A, int steps=1) {
    PE result=A;
    for(int i=0;i<steps;i++) result=mulY(cc,result);
    return result;
}

int main(){
    std::cout<<"\n  NOT via mulY: Fibonacci as bit-flipper?\n\n";
    
    CCParams<CryptoContextCKKSRNS> p;
    p.SetMultiplicativeDepth(5);p.SetScalingModSize(50);p.SetBatchSize(2048);
    p.SetRingDim(8192);p.SetSecretKeyDist(UNIFORM_TERNARY);p.SetSecurityLevel(HEStd_NotSet);
    auto cc=GenCryptoContext(p);
    cc->Enable(PKE);cc->Enable(KEYSWITCH);cc->Enable(LEVELEDSHE);cc->Enable(ADVANCEDSHE);
    auto kp=cc->KeyGen();
    
    std::cout<<"  Bit | Input(a,b) | After 1 mulY | ratio | decoded\n";
    std::cout<<"  --------------------------------------------------\n";
    for(int bit=0;bit<=1;bit++){
        PE A=encrypt_direct(cc,kp,(double)bit);
        double a0=decrypt_val(cc,kp,A.a), b0=decrypt_val(cc,kp,A.b);
        
        PE flipped=not_via_mulY(cc,A,1);
        double a1=decrypt_val(cc,kp,flipped.a), b1=decrypt_val(cc,kp,flipped.b);
        int dec=decode_bit(cc,kp,flipped);
        
        std::cout<<"  "<<bit<<"   | ("<<std::fixed<<std::setprecision(0)<<a0<<","<<b0
             <<")  | ("<<a1<<","<<b1<<")         | "<<std::fixed<<std::setprecision(4)<<a1/b1
             <<" | "<<dec<<" (exp "<<1-bit<<")\n";
    }
    
    // Try: NOT via multiple mulY steps
    std::cout<<"\n  NOT via mulY steps sweep:\n";
    for(int steps=1;steps<=5;steps++){
        int correct=0;
        for(int bit=0;bit<=1;bit++){
            PE A=encrypt_direct(cc,kp,(double)bit);
            PE result=A;
            for(int i=0;i<steps;i++) result=mulY(cc,result);
            if(decode_bit(cc,kp,result)==1-bit)correct++;
        }
        std::cout<<"  Steps="<<steps<<": "<<correct<<"/2 correct\n";
    }
    
    // Try: NOT via mulY then mulY_inv (different ratio)
    std::cout<<"\n  NOT via mulY+mulY_inv sweep:\n";
    for(int steps=1;steps<=5;steps++){
        int correct=0;
        for(int bit=0;bit<=1;bit++){
            PE A=encrypt_direct(cc,kp,(double)bit);
            PE result=A;
            for(int i=0;i<steps;i++){result=mulY(cc,result);result=mulY_inv(cc,result);}
            if(decode_bit(cc,kp,result)==1-bit)correct++;
        }
        std::cout<<"  Steps="<<steps<<": "<<correct<<"/2 correct\n";
    }
    
    std::cout<<"\n";
    return 0;
}
