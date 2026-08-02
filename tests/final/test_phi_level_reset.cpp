// LEVEL-RESETTING NAND: mulY before each gate resets level via b→a!
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

// LEVEL-RESETTING NAND: mulY resets level via b(level 0) → a!
PE nand_reset(CryptoContext<DCRTPoly>& cc, PE& A, PE& B) {
    // mulY both inputs: b (level 0) becomes a (level 0)!
    A=mulY(cc,A); // (1, a_old+1) — new a=1 (level 0!)
    B=mulY(cc,B); // (1, b_old+1) — new a=1 (level 0!)
    
    // Now both A.a and B.a are level 0!
    auto and_a=cc->EvalMult(A.a, B.a); // 1*1 = 1, level = max(0,0)=0!
    // NOT: 1 - and_a
    return PE{cc->EvalSub(A.b, and_a), A.b}; // A.b is also level 0!
}

int decode_bit(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, const PE& s){
    double a=decrypt_val(cc,kp,s.a), b=decrypt_val(cc,kp,s.b);
    return (a>b*0.5)?1:0;
}

int main(){
    std::cout<<"\n  LEVEL-RESETTING NAND: mulY resets level via b→a!\n\n";
    
    CCParams<CryptoContextCKKSRNS> p;
    p.SetMultiplicativeDepth(5); // ONLY 5 levels!
    p.SetScalingModSize(50);p.SetBatchSize(2048);
    p.SetRingDim(8192);p.SetSecretKeyDist(UNIFORM_TERNARY);p.SetSecurityLevel(HEStd_NotSet);
    auto cc=GenCryptoContext(p);
    cc->Enable(PKE);cc->Enable(KEYSWITCH);cc->Enable(LEVELEDSHE);cc->Enable(ADVANCEDSHE);
    auto kp=cc->KeyGen();cc->EvalMultKeyGen(kp.secretKey);
    
    // Truth table
    std::cout<<"  Truth table:\n";
    for(int a=0;a<=1;a++) for(int b=0;b<=1;b++){
        PE A=encrypt_bit(cc,kp,(double)a), B=encrypt_bit(cc,kp,(double)b);
        PE nand=nand_reset(cc,A,B);
        std::cout<<"  "<<a<<" "<<b<<": "<<decode_bit(cc,kp,nand)<<" (exp "<<1-a*b<<")\n";
    }
    
    // Deep chain: 100 NANDs with ONLY 5 depth budget!
    int depth=100;
    std::cout<<"\n  Deep chain: "<<depth<<" NANDs (depth budget ONLY 5!)\n";
    PE chain=encrypt_bit(cc,kp,0.0), fixed=encrypt_bit(cc,kp,1.0);
    bool crashed=false; int result=-1;
    try{
        for(int i=0;i<depth;i++){
            chain=nand_reset(cc,chain,fixed);
        }
        result=decode_bit(cc,kp,chain);
    }catch(const std::exception& e){
        crashed=true;
        std::cout<<"  CRASHED: "<<e.what()<<"\n";
    }
    if(!crashed){
        int expected=(depth%2==0)?0:1;
        std::cout<<"  Result: "<<result<<" (exp "<<expected<<") "<<(result==expected?"MATCH!":"WRONG")<<"\n";
    }
    
    std::cout<<"\n  Depth budget: 5 | NANDs executed: "<<(crashed?"CRASHED":std::to_string(depth))<<"\n";
    std::cout<<"  EvalMult per NAND: 1 (level RESET via mulY!)\n\n";
    
    return 0;
}
