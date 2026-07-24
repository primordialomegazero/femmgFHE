// CONTROLLED STATE SPIRAL: Bit-aware harmonization
#include <iostream>
#include <iomanip>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <sys/time.h>
#include "openfhe.h"
using namespace lbcrypto;

struct PE { Ciphertext<DCRTPoly> a; Ciphertext<DCRTPoly> b; };
PE mulY_2d(CryptoContext<DCRTPoly>& cc, const PE& x) { return {x.b, cc->EvalAdd(x.a, x.b)}; }
PE mulY_inv_2d(CryptoContext<DCRTPoly>& cc, const PE& x) { return {cc->EvalSub(x.b, x.a), x.a}; }

double decrypt_val(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, const Ciphertext<DCRTPoly>& c) {
    Plaintext pt; cc->Decrypt(kp.secretKey, c, &pt); return pt->GetCKKSPackedValue()[0].real();
}
PE encrypt_bit(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, double v) {
    return {cc->Encrypt(kp.publicKey,cc->MakeCKKSPackedPlaintext(std::vector<double>{v})),
            cc->Encrypt(kp.publicKey,cc->MakeCKKSPackedPlaintext(std::vector<double>{1.0}))};
}
PE nand_2d(CryptoContext<DCRTPoly>& cc, const PE& A, const PE& B) {
    auto aabb=cc->EvalMult(A.b,B.b); auto aaba=cc->EvalMult(A.a,B.a);
    return {cc->EvalSub(aabb,aaba), aabb};
}
int decode_bit(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, const PE& s){
    double a=decrypt_val(cc,kp,s.a), b=decrypt_val(cc,kp,s.b);
    return (a>b*0.5)?1:0;
}
double time_ms(struct timeval s,struct timeval e){return (e.tv_sec-s.tv_sec)*1000.0+(e.tv_usec-s.tv_usec)/1000.0;}

// CONTROLLED SPIRAL: Preserve bit value by using symmetric forward/reverse
PE controlled_spiral(CryptoContext<DCRTPoly>& cc, const PE& x, int cycles=3) {
    PE s=x;
    // Equal forward and reverse = preserves ratio (identity-like)
    // But with CKKS noise, the slight asymmetry cleans the state
    for(int c=0;c<cycles;c++){
        s=mulY_2d(cc,s); s=mulY_inv_2d(cc,s);  // symmetric pair
    }
    // After pairs: ratio preserved, but noise redistributed
    // Then one extra forward to push toward ψ-attractor slightly
    // but threshold decode handles both 0 and 1
    return s;
}

// BIT-AWARE: Use threshold, not attractor convergence
int main(){
    std::cout<<"\n  CONTROLLED STATE SPIRAL: Symmetric pairs + threshold decode\n\n";
    
    CCParams<CryptoContextCKKSRNS> p;
    p.SetMultiplicativeDepth(500);p.SetScalingModSize(50);p.SetBatchSize(2048);
    p.SetRingDim(16384);p.SetSecretKeyDist(UNIFORM_TERNARY);p.SetSecurityLevel(HEStd_NotSet);
    auto cc=GenCryptoContext(p);
    cc->Enable(PKE);cc->Enable(KEYSWITCH);cc->Enable(LEVELEDSHE);cc->Enable(ADVANCEDSHE);
    auto kp=cc->KeyGen();cc->EvalMultKeyGen(kp.secretKey);
    srand(time(0));
    
    // Test on clean bits: ratio should stay at 0 or 1
    std::cout<<"  Controlled spiral on clean bits:\n";
    for(int bit=0;bit<=1;bit++){
        PE clean=encrypt_bit(cc,kp,(double)bit);
        PE s=controlled_spiral(cc,clean,4);
        double a=decrypt_val(cc,kp,s.a), b=decrypt_val(cc,kp,s.b);
        std::cout<<"  Bit "<<bit<<": ratio="<<std::fixed<<std::setprecision(6)<<a/b
             <<" decoded="<<decode_bit(cc,kp,s)<<" (expected "<<bit<<")\n";
    }
    
    // Test on NAND outputs
    std::cout<<"\n  NAND with controlled spiral: ";
    int ok=0;
    for(int a=0;a<=1;a++) for(int b=0;b<=1;b++){
        PE A=encrypt_bit(cc,kp,(double)a), B=encrypt_bit(cc,kp,(double)b);
        PE nand=nand_2d(cc,A,B);
        PE corrected=controlled_spiral(cc,nand,3);
        if(decode_bit(cc,kp,corrected)==1-a*b)ok++;
    }
    std::cout<<ok<<"/4\n";
    
    // Full Adder
    std::cout<<"  Full Adder (controlled spiral): ";
    int cases[8][3]={{0,0,0},{0,0,1},{0,1,0},{0,1,1},{1,0,0},{1,0,1},{1,1,0},{1,1,1}};
    ok=0; int okc=0;
    struct timeval t0,t1; gettimeofday(&t0,NULL);
    for(int i=0;i<8;i++){
        int A=cases[i][0],B=cases[i][1],Cin=cases[i][2];
        PE a=encrypt_bit(cc,kp,(double)A),b=encrypt_bit(cc,kp,(double)B),c=encrypt_bit(cc,kp,(double)Cin);
        PE X1=controlled_spiral(cc,nand_2d(cc,a,b),3);
        PE X2=controlled_spiral(cc,nand_2d(cc,a,X1),3);
        PE X3=controlled_spiral(cc,nand_2d(cc,b,X1),3);
        PE X4=controlled_spiral(cc,nand_2d(cc,X2,X3),3);
        PE X5=controlled_spiral(cc,nand_2d(cc,X4,c),3);
        PE X6=controlled_spiral(cc,nand_2d(cc,X4,X5),3);
        PE X7=controlled_spiral(cc,nand_2d(cc,X5,c),3);
        PE SUM=controlled_spiral(cc,nand_2d(cc,X6,X7),3);
        PE COUT=controlled_spiral(cc,nand_2d(cc,X1,X5),3);
        if(decode_bit(cc,kp,SUM)==(A+B+Cin)%2)ok++;
        if(decode_bit(cc,kp,COUT)==(A+B+Cin)/2)okc++;
    }
    gettimeofday(&t1,NULL);
    std::cout<<"SUM="<<ok<<"/8 COUT="<<okc<<"/8 Time="<<std::fixed<<std::setprecision(0)<<time_ms(t0,t1)<<"ms\n\n";
    
    // 4-bit adder
    std::cout<<"  4-Bit Adder (controlled spiral): ";
    int bits=4; int a_bits[bits],b_bits[bits]; long long valA=0,valB=0;
    for(int i=0;i<bits;i++){a_bits[i]=rand()%2;b_bits[i]=rand()%2;valA=valA*2+a_bits[i];valB=valB*2+b_bits[i];}
    PE rCarry=encrypt_bit(cc,kp,0.0);
    int sum_bits[bits]; long long result=0;
    gettimeofday(&t0,NULL);
    for(int i=bits-1;i>=0;i--){
        PE a=encrypt_bit(cc,kp,(double)a_bits[i]),b=encrypt_bit(cc,kp,(double)b_bits[i]);
        PE X1=controlled_spiral(cc,nand_2d(cc,a,b),2);
        PE X2=controlled_spiral(cc,nand_2d(cc,a,X1),2);
        PE X3=controlled_spiral(cc,nand_2d(cc,b,X1),2);
        PE X4=controlled_spiral(cc,nand_2d(cc,X2,X3),2);
        PE X5=controlled_spiral(cc,nand_2d(cc,X4,rCarry),2);
        PE X6=controlled_spiral(cc,nand_2d(cc,X4,X5),2);
        PE X7=controlled_spiral(cc,nand_2d(cc,X5,rCarry),2);
        PE SUM=controlled_spiral(cc,nand_2d(cc,X6,X7),2);
        PE COUT=controlled_spiral(cc,nand_2d(cc,X1,X5),2);
        sum_bits[i]=decode_bit(cc,kp,SUM); rCarry=COUT;
    }
    for(int i=0;i<bits;i++){std::cout<<sum_bits[i];result=result*2+sum_bits[i];}
    int fc=decode_bit(cc,kp,rCarry); if(fc)result+=(1LL<<bits);
    gettimeofday(&t1,NULL);
    std::cout<<" (c="<<fc<<") = "<<result<<" | Expected="<<valA+valB<<" | "<<(result==valA+valB?"MATCH!":"MISMATCH")<<"\n";
    std::cout<<"  Time: "<<std::fixed<<std::setprecision(0)<<time_ms(t0,t1)<<"ms\n\n";
    
    std::cout<<"  CONTROLLED SPIRAL: "<<(result==valA+valB?"4-BIT MATCH! Bit-aware!":"needs work")<<"\n\n";
    return 0;
}
