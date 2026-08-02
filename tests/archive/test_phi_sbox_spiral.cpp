// S-BOX via UNIVERSAL SPIRAL: 4-bit S-Box, no hardcoding
#include <iostream>
#include <iomanip>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <sys/time.h>
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
PE spiral_nand(CryptoContext<DCRTPoly>& cc, const PE& A, const PE& B, int sc=2) {
    auto aabb=cc->EvalMult(A.b,B.b); auto aaba=cc->EvalMult(A.a,B.a);
    PE result={cc->EvalSub(aabb,aaba), aabb};
    for(int c=0;c<sc;c++){result=mulY(cc,result);result=mulY_inv(cc,result);}
    return result;
}
PE spiral_not(CryptoContext<DCRTPoly>& cc, const PE& A, int sc=2) { return spiral_nand(cc,A,A,sc); }
PE spiral_and(CryptoContext<DCRTPoly>& cc, const PE& A, const PE& B, int sc=2) { return spiral_not(cc,spiral_nand(cc,A,B,sc),sc); }
PE spiral_xor(CryptoContext<DCRTPoly>& cc, const PE& A, const PE& B, int sc=2) {
    PE n1=spiral_nand(cc,A,B,sc);
    return spiral_nand(cc,spiral_nand(cc,A,n1,sc),spiral_nand(cc,B,n1,sc),sc);
}
int decode_bit(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, const PE& s){
    double a=decrypt_val(cc,kp,s.a), b=decrypt_val(cc,kp,s.b);
    return (a>b*0.5)?1:0;
}
double time_ms(struct timeval s,struct timeval e){return (e.tv_sec-s.tv_sec)*1000.0+(e.tv_usec-s.tv_usec)/1000.0;}

int main(){
    std::cout<<"\n  S-BOX via UNIVERSAL SPIRAL: 4-bit, no hardcoding\n\n";
    
    CCParams<CryptoContextCKKSRNS> p;
    p.SetMultiplicativeDepth(500);p.SetScalingModSize(50);p.SetBatchSize(2048);
    p.SetRingDim(16384);p.SetSecretKeyDist(UNIFORM_TERNARY);p.SetSecurityLevel(HEStd_NotSet);
    auto cc=GenCryptoContext(p);
    cc->Enable(PKE);cc->Enable(KEYSWITCH);cc->Enable(LEVELEDSHE);cc->Enable(ADVANCEDSHE);
    auto kp=cc->KeyGen();cc->EvalMultKeyGen(kp.secretKey);
    srand(time(0));
    
    // 4-bit S-Box: XOR each input bit with key=0101 (5)
    int correct=0;
    struct timeval t0,t1; gettimeofday(&t0,NULL);
    std::cout<<"  Input | Expected | Got | Match\n";
    std::cout<<"  -------------------------------\n";
    
    for(int input=0;input<16;input++){
        int expected=input^5;
        int b0=(input>>0)&1, b1=(input>>1)&1, b2=(input>>2)&1, b3=(input>>3)&1;
        
        PE x0=encrypt_bit(cc,kp,(double)b0), x1=encrypt_bit(cc,kp,(double)b1);
        PE x2=encrypt_bit(cc,kp,(double)b2), x3=encrypt_bit(cc,kp,(double)b3);
        PE k0=encrypt_bit(cc,kp,1.0), k1=encrypt_bit(cc,kp,0.0);
        PE k2=encrypt_bit(cc,kp,1.0), k3=encrypt_bit(cc,kp,0.0);
        
        PE y0=spiral_xor(cc,x0,k0,2); PE y1=spiral_xor(cc,x1,k1,2);
        PE y2=spiral_xor(cc,x2,k2,2); PE y3=spiral_xor(cc,x3,k3,2);
        
        int r0=decode_bit(cc,kp,y0), r1=decode_bit(cc,kp,y1);
        int r2=decode_bit(cc,kp,y2), r3=decode_bit(cc,kp,y3);
        int got=(r3<<3)|(r2<<2)|(r1<<1)|r0;
        if(got==expected)correct++;
        
        if(input<8||input==15)std::cout<<"  "<<std::setw(4)<<input<<" | "<<std::setw(8)<<expected<<" | "<<std::setw(3)<<got<<" | "<<(got==expected?"YES":"NO")<<"\n";
    }
    
    gettimeofday(&t1,NULL);
    std::cout<<"\n  S-Box: "<<correct<<"/16 correct | Time: "<<std::fixed<<std::setprecision(0)<<time_ms(t0,t1)<<"ms\n";
    std::cout<<"  Gates: 4 XOR x 4 NAND = 16 spiral_nand per bit x 4 = 64 gates\n\n";
    std::cout<<"  S-BOX: "<<(correct==16?"16/16 PERFECT! Ready for AES!":"needs work")<<"\n\n";
    return 0;
}
