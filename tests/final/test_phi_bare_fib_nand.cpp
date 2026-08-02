// BARE FIBONACCI NAND: Just AND + mulY, no spiral, no corrector
// Isolate: does mulY work as NOT for NAND(1,1)?
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
PE and_gate(CryptoContext<DCRTPoly>& cc, const PE& A, const PE& B) {
    auto aabb=cc->EvalMult(A.b,B.b); auto aaba=cc->EvalMult(A.a,B.a);
    return {aaba, aabb};
}

// BARE: Just AND + 1 mulY. No spiral. No corrector.
PE bare_fib_nand(CryptoContext<DCRTPoly>& cc, const PE& A, const PE& B) {
    PE and_result=and_gate(cc,A,B);
    return mulY(cc,and_result);
}

int decode_bit(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, const PE& s){
    double a=decrypt_val(cc,kp,s.a), b=decrypt_val(cc,kp,s.b);
    return (a>b*0.5)?1:0;
}

int main(){
    std::cout<<"\n  BARE FIB NAND: Just AND + mulY, no spiral\n\n";
    
    CCParams<CryptoContextCKKSRNS> p;
    p.SetMultiplicativeDepth(50);p.SetScalingModSize(50);p.SetBatchSize(2048);
    p.SetRingDim(8192);p.SetSecretKeyDist(UNIFORM_TERNARY);p.SetSecurityLevel(HEStd_NotSet);
    auto cc=GenCryptoContext(p);
    cc->Enable(PKE);cc->Enable(KEYSWITCH);cc->Enable(LEVELEDSHE);cc->Enable(ADVANCEDSHE);
    auto kp=cc->KeyGen();cc->EvalMultKeyGen(kp.secretKey);
    
    // Detailed trace for each case
    std::cout<<"  Detailed trace:\n";
    std::cout<<"  A B | AND(a,b) | after mulY(a,b) | ratio | decoded | exp | match\n";
    std::cout<<"  ------------------------------------------------------------------\n";
    
    for(int a=0;a<=1;a++) for(int b=0;b<=1;b++){
        PE A=encrypt_bit(cc,kp,(double)a), B=encrypt_bit(cc,kp,(double)b);
        PE and_res=and_gate(cc,A,B);
        double and_a=decrypt_val(cc,kp,and_res.a), and_b=decrypt_val(cc,kp,and_res.b);
        
        PE nand=bare_fib_nand(cc,A,B);
        double na=decrypt_val(cc,kp,nand.a), nb=decrypt_val(cc,kp,nand.b);
        double ratio=na/nb;
        int dec=decode_bit(cc,kp,nand);
        int exp=1-a*b;
        
        std::cout<<"  "<<a<<" "<<b<<" | ("<<std::fixed<<std::setprecision(0)<<and_a<<","<<and_b
             <<")   | ("<<std::fixed<<std::setprecision(1)<<na<<","<<nb
             <<")         | "<<std::fixed<<std::setprecision(4)<<ratio
             <<" | "<<dec<<" | "<<exp<<" | "<<(dec==exp?"OK":"XX")<<"\n";
    }
    
    // Deep chain test
    std::cout<<"\n  Deep chain (50 NANDs, bare):\n";
    PE chain=encrypt_bit(cc,kp,0.0), fixed=encrypt_bit(cc,kp,1.0);
    int depth=50, expected=(depth%2==0)?0:1;
    bool crashed=false; int result=-1;
    try{
        for(int i=0;i<depth;i++) chain=bare_fib_nand(cc,chain,fixed);
        result=decode_bit(cc,kp,chain);
    }catch(const std::exception& e){crashed=true; std::cout<<"  CRASHED: "<<e.what()<<"\n";}
    if(!crashed)std::cout<<"  Result: "<<result<<" (exp "<<expected<<") "<<(result==expected?"MATCH!":"WRONG")<<"\n";
    
    std::cout<<"\n";
    return 0;
}
