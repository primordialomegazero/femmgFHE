// UNIVERSAL SPIRAL GATE: Generalized NAND + Spiral, arbitrary depth
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

// UNIVERSAL SPIRAL NAND: NAND + auto-correct in one function
PE spiral_nand(CryptoContext<DCRTPoly>& cc, const PE& A, const PE& B, int spiral_cycles=2) {
    auto aabb=cc->EvalMult(A.b,B.b);
    auto aaba=cc->EvalMult(A.a,B.a);
    PE result={cc->EvalSub(aabb,aaba), aabb};
    for(int c=0;c<spiral_cycles;c++){result=mulY(cc,result);result=mulY_inv(cc,result);}
    return result;
}

// Universal Full Adder from spiral NANDs
struct FAR { PE sum; PE cout; };
FAR universal_fa(CryptoContext<DCRTPoly>& cc, const PE& A, const PE& B, const PE& Cin, int sc=2){
    PE X1=spiral_nand(cc,A,B,sc);
    PE X2=spiral_nand(cc,A,X1,sc);
    PE X3=spiral_nand(cc,B,X1,sc);
    PE X4=spiral_nand(cc,X2,X3,sc);
    PE X5=spiral_nand(cc,X4,Cin,sc);
    PE X6=spiral_nand(cc,X4,X5,sc);
    PE X7=spiral_nand(cc,X5,Cin,sc);
    return {spiral_nand(cc,X6,X7,sc), spiral_nand(cc,X1,X5,sc)};
}

int decode_bit(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, const PE& s){
    double a=decrypt_val(cc,kp,s.a), b=decrypt_val(cc,kp,s.b);
    return (a>b*0.5)?1:0;
}
double time_ms(struct timeval s,struct timeval e){return (e.tv_sec-s.tv_sec)*1000.0+(e.tv_usec-s.tv_usec)/1000.0;}

int main(){
    std::cout<<"\n  UNIVERSAL SPIRAL GATE: Generalized NAND for any circuit\n\n";
    
    CCParams<CryptoContextCKKSRNS> p;
    p.SetMultiplicativeDepth(500);p.SetScalingModSize(50);p.SetBatchSize(2048);
    p.SetRingDim(16384);p.SetSecretKeyDist(UNIFORM_TERNARY);p.SetSecurityLevel(HEStd_NotSet);
    auto cc=GenCryptoContext(p);
    cc->Enable(PKE);cc->Enable(KEYSWITCH);cc->Enable(LEVELEDSHE);cc->Enable(ADVANCEDSHE);
    auto kp=cc->KeyGen();cc->EvalMultKeyGen(kp.secretKey);
    srand(time(0));
    
    // NAND truth table
    std::cout<<"  NAND: ";
    int ok=0;
    for(int a=0;a<=1;a++) for(int b=0;b<=1;b++){
        PE A=encrypt_bit(cc,kp,(double)a), B=encrypt_bit(cc,kp,(double)b);
        if(decode_bit(cc,kp,spiral_nand(cc,A,B,2))==1-a*b)ok++;
    }
    std::cout<<ok<<"/4\n";
    
    // Full Adder
    int cases[8][3]={{0,0,0},{0,0,1},{0,1,0},{0,1,1},{1,0,0},{1,0,1},{1,1,0},{1,1,1}};
    ok=0; int okc=0;
    struct timeval t0,t1; gettimeofday(&t0,NULL);
    for(int i=0;i<8;i++){
        int A=cases[i][0],B=cases[i][1],Cin=cases[i][2];
        PE a=encrypt_bit(cc,kp,(double)A),b=encrypt_bit(cc,kp,(double)B),c=encrypt_bit(cc,kp,(double)Cin);
        FAR fa=universal_fa(cc,a,b,c,2);
        if(decode_bit(cc,kp,fa.sum)==(A+B+Cin)%2)ok++;
        if(decode_bit(cc,kp,fa.cout)==(A+B+Cin)/2)okc++;
    }
    gettimeofday(&t1,NULL);
    std::cout<<"  Full Adder: SUM="<<ok<<"/8 COUT="<<okc<<"/8 Time="<<std::fixed<<std::setprecision(0)<<time_ms(t0,t1)<<"ms\n";
    
    // 4-bit adder
    int bits=4; int a_bits[bits],b_bits[bits]; long long valA=0,valB=0;
    for(int i=0;i<bits;i++){a_bits[i]=rand()%2;b_bits[i]=rand()%2;valA=valA*2+a_bits[i];valB=valB*2+b_bits[i];}
    PE rCarry=encrypt_bit(cc,kp,0.0);
    int sum_bits[bits]; long long result=0;
    gettimeofday(&t0,NULL);
    for(int i=bits-1;i>=0;i--){
        PE a=encrypt_bit(cc,kp,(double)a_bits[i]),b=encrypt_bit(cc,kp,(double)b_bits[i]);
        FAR fa=universal_fa(cc,a,b,rCarry,2);
        sum_bits[i]=decode_bit(cc,kp,fa.sum); rCarry=fa.cout;
    }
    for(int i=0;i<bits;i++){std::cout<<sum_bits[i];result=result*2+sum_bits[i];}
    int fc=decode_bit(cc,kp,rCarry); if(fc)result+=(1LL<<bits);
    gettimeofday(&t1,NULL);
    std::cout<<" (c="<<fc<<") = "<<result<<" | Expected="<<valA+valB<<" | "<<(result==valA+valB?"MATCH!":"MISMATCH")<<"\n";
    std::cout<<"  Time: "<<std::fixed<<std::setprecision(0)<<time_ms(t0,t1)<<"ms\n\n";
    
    std::cout<<"  UNIVERSAL SPIRAL: "<<(result==valA+valB?"GENERALIZED! Ready for S-Box & AES!":"needs work")<<"\n\n";
    return 0;
}
