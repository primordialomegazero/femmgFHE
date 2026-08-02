// BARE FIB CHAIN DEBUG: Trace first 10 NANDs to find where it breaks
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
PE bare_fib_nand(CryptoContext<DCRTPoly>& cc, const PE& A, const PE& B) {
    return mulY(cc,and_gate(cc,A,B));
}
int decode_bit(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, const PE& s){
    double a=decrypt_val(cc,kp,s.a), b=decrypt_val(cc,kp,s.b);
    return (a>b*0.5)?1:0;
}

int main(){
    std::cout<<"\n  BARE FIB CHAIN DEBUG: Trace every step for first 15 NANDs\n\n";
    
    CCParams<CryptoContextCKKSRNS> p;
    p.SetMultiplicativeDepth(30);p.SetScalingModSize(50);p.SetBatchSize(2048);
    p.SetRingDim(8192);p.SetSecretKeyDist(UNIFORM_TERNARY);p.SetSecurityLevel(HEStd_NotSet);
    auto cc=GenCryptoContext(p);
    cc->Enable(PKE);cc->Enable(KEYSWITCH);cc->Enable(LEVELEDSHE);cc->Enable(ADVANCEDSHE);
    auto kp=cc->KeyGen();cc->EvalMultKeyGen(kp.secretKey);
    
    PE chain=encrypt_bit(cc,kp,0.0), fixed=encrypt_bit(cc,kp,1.0);
    
    std::cout<<"  Step | a_val | b_val | ratio  | decoded | expected\n";
    std::cout<<"  -------------------------------------------------\n";
    
    for(int i=0;i<15;i++){
        chain=bare_fib_nand(cc,chain,fixed);
        double a=decrypt_val(cc,kp,chain.a), b=decrypt_val(cc,kp,chain.b);
        int dec=decode_bit(cc,kp,chain);
        int exp=(i%2==0)?1:0; // NOT(0)=1, NOT(1)=0, alternating
        std::cout<<"  "<<std::setw(4)<<i<<" | "<<std::fixed<<std::setprecision(1)<<a
             <<" | "<<std::fixed<<std::setprecision(1)<<b
             <<" | "<<std::fixed<<std::setprecision(4)<<a/b
             <<" | "<<dec<<" | "<<exp<<" "<<(dec==exp?"OK":"XX")<<"\n";
    }
    
    std::cout<<"\n  EvalMult used: 15 (1 per NAND)\n";
    std::cout<<"  Depth budget: 30\n\n";
    return 0;
}
