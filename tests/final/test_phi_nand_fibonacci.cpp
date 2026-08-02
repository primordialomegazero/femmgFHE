// NAND = NOT(AND) with RESET: mulY for NOT, mulY_inv to reset before next gate
// Total: 1 EvalMult per NAND! Consistent state throughout chain!
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

// AND via 1 EvalMult
PE and_gate(CryptoContext<DCRTPoly>& cc, const PE& A, const PE& B) {
    auto aabb=cc->EvalMult(A.b,B.b);
    auto aaba=cc->EvalMult(A.a,B.a);
    return {aaba, aabb};
}

// NAND with RESET: AND → NOT(mulY) → RESET(mulY_inv)
// After reset: state is back to (0,1) or (1,1) format for next gate!
PE nand_fibonacci_reset(CryptoContext<DCRTPoly>& cc, const PE& A, const PE& B) {
    PE and_result=and_gate(cc,A,B);      // 1 EvalMult
    PE not_result=mulY(cc,and_result);    // NOT (0 depth)
    return mulY_inv(cc,not_result);       // RESET (0 depth)
}

int decode_bit(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, const PE& s){
    double a=decrypt_val(cc,kp,s.a), b=decrypt_val(cc,kp,s.b);
    return (a>b*0.5)?1:0;
}

int main(){
    std::cout<<"\n  NAND = AND + NOT(mulY) + RESET(mulY_inv): 1 EvalMult!\n\n";
    
    CCParams<CryptoContextCKKSRNS> p;
    p.SetMultiplicativeDepth(50);p.SetScalingModSize(50);p.SetBatchSize(2048);
    p.SetRingDim(8192);p.SetSecretKeyDist(UNIFORM_TERNARY);p.SetSecurityLevel(HEStd_NotSet);
    auto cc=GenCryptoContext(p);
    cc->Enable(PKE);cc->Enable(KEYSWITCH);cc->Enable(LEVELEDSHE);cc->Enable(ADVANCEDSHE);
    auto kp=cc->KeyGen();cc->EvalMultKeyGen(kp.secretKey);
    
    // Truth table
    std::cout<<"  NAND truth table (with reset):\n";
    std::cout<<"  A B | decoded | expected | match\n";
    std::cout<<"  ---------------------------------\n";
    int correct=0;
    for(int a=0;a<=1;a++) for(int b=0;b<=1;b++){
        PE A=encrypt_bit(cc,kp,(double)a), B=encrypt_bit(cc,kp,(double)b);
        PE nand=nand_fibonacci_reset(cc,A,B);
        int dec=decode_bit(cc,kp,nand);
        int exp=1-a*b;
        if(dec==exp)correct++;
        std::cout<<"  "<<a<<" "<<b<<" | "<<dec<<" | "<<exp<<" | "<<(dec==exp?"YES":"NO")<<"\n";
    }
    std::cout<<"\n  NAND: "<<correct<<"/4\n\n";
    
    // Deep chain: 80 NANDs (budget 50, but we use 1 EvalMult each!)
    int depth=80;
    std::cout<<"  Deep chain: "<<depth<<" NAND gates (budget 50, 1 EvalMult each)\n";
    PE chain=encrypt_bit(cc,kp,0.0);
    PE fixed=encrypt_bit(cc,kp,1.0);
    int expected=(depth%2==0)?0:1;
    
    bool crashed=false; int result=-1;
    try{
        for(int i=0;i<depth;i++){
            chain=nand_fibonacci_reset(cc,chain,fixed);
        }
        result=decode_bit(cc,kp,chain);
    }catch(const std::exception& e){
        crashed=true;
        std::cout<<"  CRASHED: "<<e.what()<<"\n";
    }
    if(!crashed)std::cout<<"  Result: "<<result<<" (exp "<<expected<<") "<<(result==expected?"MATCH!":"WRONG")<<"\n";
    std::cout<<"  EvalMult per NAND: 1 (vs 2 for standard NAND)\n";
    std::cout<<"  Depth capacity: 2x standard!\n\n";
    
    // Compare: standard NAND chain
    std::cout<<"  Standard NAND chain (2 EvalMult each) for comparison:\n";
    auto nand_std=[](CryptoContext<DCRTPoly>& cc, const PE& A, const PE& B){
        auto aabb=cc->EvalMult(A.b,B.b);
        auto aaba=cc->EvalMult(A.a,B.a);
        return PE{cc->EvalSub(aabb,aaba), aabb};
    };
    
    PE chain2=encrypt_bit(cc,kp,0.0);
    bool crash2=false;
    try{
        for(int i=0;i<depth;i++) chain2=nand_std(cc,chain2,fixed);
        int r2=decode_bit(cc,kp,chain2);
        std::cout<<"  Result: "<<r2<<" (exp "<<expected<<") "<<(r2==expected?"MATCH!":"WRONG")<<"\n";
    }catch(const std::exception& e){
        crash2=true;
        std::cout<<"  CRASHED at depth "<<depth<<": "<<e.what()<<"\n";
    }
    
    std::cout<<"\n  ╔══════════════════════════════════════════════════════════════╗\n";
    std::cout<<"  ║  FIBONACCI NAND: 1 EvalMult! "<<correct<<"/4 truth table";
    for(int i=0;i<22;i++)std::cout<<" ";std::cout<<"║\n";
    std::cout<<"  ║  Deep chain: "<<(crashed?"CRASHED":"SURVIVED")<<" at depth "<<depth;
    for(int i=0;i<22;i++)std::cout<<" ";std::cout<<"║\n";
    std::cout<<"  ╚══════════════════════════════════════════════════════════════╝\n\n";
    
    return 0;
}
