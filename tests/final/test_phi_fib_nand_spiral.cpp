// FIBONACCI NAND + ASYMMETRIC SPIRAL: forward-biased, auto-normalize!
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
PE encrypt_bit(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, double v) {
    return {cc->Encrypt(kp.publicKey,cc->MakeCKKSPackedPlaintext(std::vector<double>{v})),
            cc->Encrypt(kp.publicKey,cc->MakeCKKSPackedPlaintext(std::vector<double>{1.0}))};
}
PE and_gate(CryptoContext<DCRTPoly>& cc, const PE& A, const PE& B) {
    auto aabb=cc->EvalMult(A.b,B.b); auto aaba=cc->EvalMult(A.a,B.a);
    return {aaba, aabb};
}

// ASYMMETRIC SPIRAL: mulY → mulY_inv → mulY (net +1 forward!)
// This biases toward ψ-attractor without full identity return
PE asym_spiral(CryptoContext<DCRTPoly>& cc, const PE& x, int cycles=2) {
    PE s=x;
    for(int c=0;c<cycles;c++){
        s=mulY(cc,s); s=mulY_inv(cc,s); s=mulY(cc,s); // net +1 forward
    }
    return s;
}

// FIB NAND + ASYM SPIRAL
PE fib_nand_asym(CryptoContext<DCRTPoly>& cc, const PE& A, const PE& B) {
    PE and_result=and_gate(cc,A,B);
    PE not_result=mulY(cc,and_result);
    return asym_spiral(cc,not_result,2);
}

int decode_bit(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, const PE& s){
    double a=decrypt_val(cc,kp,s.a), b=decrypt_val(cc,kp,s.b);
    return (a>b*0.5)?1:0;
}

int main(){
    std::cout<<"\n  FIB NAND + ASYMMETRIC SPIRAL: net forward bias\n\n";
    
    CCParams<CryptoContextCKKSRNS> p;
    p.SetMultiplicativeDepth(50);p.SetScalingModSize(50);p.SetBatchSize(2048);
    p.SetRingDim(8192);p.SetSecretKeyDist(UNIFORM_TERNARY);p.SetSecurityLevel(HEStd_NotSet);
    auto cc=GenCryptoContext(p);
    cc->Enable(PKE);cc->Enable(KEYSWITCH);cc->Enable(LEVELEDSHE);cc->Enable(ADVANCEDSHE);
    auto kp=cc->KeyGen();cc->EvalMultKeyGen(kp.secretKey);
    
    std::cout<<"  Truth table:\n";
    int correct=0;
    for(int a=0;a<=1;a++) for(int b=0;b<=1;b++){
        PE A=encrypt_bit(cc,kp,(double)a), B=encrypt_bit(cc,kp,(double)b);
        PE nand=fib_nand_asym(cc,A,B);
        int dec=decode_bit(cc,kp,nand);
        if(dec==1-a*b)correct++;
        std::cout<<"  "<<a<<" "<<b<<": "<<dec<<" (exp "<<1-a*b<<") "<<(dec==1-a*b?"OK":"XX")<<"\n";
    }
    std::cout<<"  Truth table: "<<correct<<"/4\n\n";
    
    // Deep chain: 50 NANDs
    int depth=50;
    std::cout<<"  Deep chain: "<<depth<<" NANDs\n";
    PE chain=encrypt_bit(cc,kp,0.0), fixed=encrypt_bit(cc,kp,1.0);
    int expected=(depth%2==0)?0:1;
    bool crashed=false; int result=-1;
    try{
        for(int i=0;i<depth;i++) chain=fib_nand_asym(cc,chain,fixed);
        result=decode_bit(cc,kp,chain);
    }catch(const std::exception& e){crashed=true; std::cout<<"  CRASHED: "<<e.what()<<"\n";}
    if(!crashed)std::cout<<"  Result: "<<result<<" (exp "<<expected<<") "<<(result==expected?"MATCH!":"WRONG")<<"\n";
    
    // Standard NAND comparison
    auto nand_std=[](CryptoContext<DCRTPoly>& cc, const PE& A, const PE& B){
        auto aabb=cc->EvalMult(A.b,B.b); auto aaba=cc->EvalMult(A.a,B.a);
        return PE{cc->EvalSub(aabb,aaba), aabb};
    };
    PE chain2=encrypt_bit(cc,kp,0.0); bool crash2=false;
    try{
        for(int i=0;i<depth;i++) chain2=nand_std(cc,chain2,fixed);
        std::cout<<"  Standard: "<<decode_bit(cc,kp,chain2)<<" (exp "<<expected<<")\n";
    }catch(const std::exception& e){crash2=true; std::cout<<"  Standard CRASHED: "<<e.what()<<"\n";}
    
    std::cout<<"\n  ╔══════════════════════════════════════════════════════════════╗\n";
    std::cout<<"  ║  ASYM SPIRAL: "<<correct<<"/4 truth | Chain: "<<(crashed?"CRASHED":"SURVIVED");
    for(int i=0;i<20;i++)std::cout<<" ";std::cout<<"║\n";
    std::cout<<"  ╚══════════════════════════════════════════════════════════════╝\n\n";
    return 0;
}
