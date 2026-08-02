// NOT via SUBTRACTION: (b-a, b) — ZERO EvalMult, chainable!
#include <iostream>
#include <iomanip>
#include <cmath>
#include "openfhe.h"
using namespace lbcrypto;

struct PE { Ciphertext<DCRTPoly> a; Ciphertext<DCRTPoly> b; };

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

// NOT via SUBTRACTION: (b-a, b) — ZERO EvalMult, chainable!
PE not_subtract(CryptoContext<DCRTPoly>& cc, const PE& A) {
    return {cc->EvalSub(A.b, A.a), A.b};
}

// NAND = NOT(AND) with subtraction-based NOT
PE nand_sub_not(CryptoContext<DCRTPoly>& cc, const PE& A, const PE& B) {
    PE and_result=and_gate(cc,A,B);      // 1 EvalMult
    return not_subtract(cc,and_result);   // 0 EvalMult (EvalSub only!)
}

int decode_bit(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, const PE& s){
    double a=decrypt_val(cc,kp,s.a), b=decrypt_val(cc,kp,s.b);
    return (a>b*0.5)?1:0;
}

int main(){
    std::cout<<"\n  NOT via SUBTRACTION: (b-a, b) — ZERO EvalMult, chainable!\n\n";
    
    CCParams<CryptoContextCKKSRNS> p;
    p.SetMultiplicativeDepth(80);p.SetScalingModSize(50);p.SetBatchSize(2048);
    p.SetRingDim(8192);p.SetSecretKeyDist(UNIFORM_TERNARY);p.SetSecurityLevel(HEStd_NotSet);
    auto cc=GenCryptoContext(p);
    cc->Enable(PKE);cc->Enable(KEYSWITCH);cc->Enable(LEVELEDSHE);cc->Enable(ADVANCEDSHE);
    auto kp=cc->KeyGen();cc->EvalMultKeyGen(kp.secretKey);
    
    // Truth table
    std::cout<<"  Truth table:\n";
    std::cout<<"  A B | AND(a,b) | NOT(AND) | decoded | exp | match\n";
    std::cout<<"  -------------------------------------------------\n";
    int correct=0;
    for(int a=0;a<=1;a++) for(int b=0;b<=1;b++){
        PE A=encrypt_bit(cc,kp,(double)a), B=encrypt_bit(cc,kp,(double)b);
        PE and_res=and_gate(cc,A,B);
        PE nand=nand_sub_not(cc,A,B);
        double and_a=decrypt_val(cc,kp,and_res.a), and_b=decrypt_val(cc,kp,and_res.b);
        double na=decrypt_val(cc,kp,nand.a), nb=decrypt_val(cc,kp,nand.b);
        int dec=decode_bit(cc,kp,nand);
        if(dec==1-a*b)correct++;
        std::cout<<"  "<<a<<" "<<b<<" | ("<<std::fixed<<std::setprecision(0)<<and_a<<","<<and_b
             <<")  | ("<<std::fixed<<std::setprecision(1)<<na<<","<<nb
             <<")    | "<<dec<<" | "<<1-a*b<<" | "<<(dec==1-a*b?"OK":"XX")<<"\n";
    }
    std::cout<<"  Truth table: "<<correct<<"/4\n\n";
    
    // Deep chain: trace first 15
    std::cout<<"  Deep chain trace (first 15):\n";
    PE chain=encrypt_bit(cc,kp,0.0), fixed=encrypt_bit(cc,kp,1.0);
    std::cout<<"  Step | a_val | b_val | ratio  | dec | exp\n";
    std::cout<<"  ------------------------------------------\n";
    for(int i=0;i<15;i++){
        chain=nand_sub_not(cc,chain,fixed);
        double a=decrypt_val(cc,kp,chain.a), b=decrypt_val(cc,kp,chain.b);
        int dec=decode_bit(cc,kp,chain);
        int exp=(i%2==0)?1:0;
        std::cout<<"  "<<std::setw(4)<<i<<" | "<<std::fixed<<std::setprecision(1)<<a
             <<" | "<<b<<" | "<<std::fixed<<std::setprecision(4)<<a/b
             <<" | "<<dec<<" | "<<exp<<" "<<(dec==exp?"OK":"XX")<<"\n";
    }
    
    // 50-gate chain
    std::cout<<"\n  50-gate chain: ";
    PE chain2=encrypt_bit(cc,kp,0.0); bool crashed=false; int result=-1;
    try{
        for(int i=0;i<50;i++) chain2=nand_sub_not(cc,chain2,fixed);
        result=decode_bit(cc,kp,chain2);
    }catch(const std::exception& e){crashed=true; std::cout<<"CRASHED: "<<e.what()<<"\n";}
    if(!crashed)std::cout<<result<<" (exp 0) "<<(result==0?"MATCH!":"WRONG")<<"\n";
    
    std::cout<<"\n  EvalMult per NAND: 1 (instead of 2!)\n";
    std::cout<<"  NOT via subtraction: ZERO EvalMult!\n\n";
    return 0;
}
