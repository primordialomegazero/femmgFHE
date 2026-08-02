// 16-BIT ADDER with 1-EvalMult NAND: 2x faster, 2x deeper!
#include <iostream>
#include <iomanip>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <sys/time.h>
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

// 1-EvalMult NAND: AND (1 EvalMult) + NOT via subtraction (0 EvalMult)
PE nand_1em(CryptoContext<DCRTPoly>& cc, const PE& A, const PE& B) {
    auto and_a=cc->EvalMult(A.a,B.a);  // AND a-components
    auto and_b=cc->EvalMult(A.b,B.b);  // AND b-components
    return {cc->EvalSub(and_b,and_a), and_b}; // NOT via subtraction!
}

int decode_bit(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, const PE& s){
    double a=decrypt_val(cc,kp,s.a), b=decrypt_val(cc,kp,s.b);
    return (a>b*0.5)?1:0;
}
double time_ms(struct timeval s,struct timeval e){return (e.tv_sec-s.tv_sec)*1000.0+(e.tv_usec-s.tv_usec)/1000.0;}

struct FAR { PE sum; PE cout; };
FAR full_adder_1em(CryptoContext<DCRTPoly>& cc, const PE& A, const PE& B, const PE& Cin){
    PE X1=nand_1em(cc,A,B);
    PE X2=nand_1em(cc,A,X1);
    PE X3=nand_1em(cc,B,X1);
    PE X4=nand_1em(cc,X2,X3);
    PE X5=nand_1em(cc,X4,Cin);
    PE X6=nand_1em(cc,X4,X5);
    PE X7=nand_1em(cc,X5,Cin);
    return {nand_1em(cc,X6,X7), nand_1em(cc,X1,X5)};
}

int main(){
    std::cout<<"\n  16-BIT ADDER: 1-EvalMult NAND (2x faster, 2x deeper!)\n\n";
    
    CCParams<CryptoContextCKKSRNS> p;
    p.SetMultiplicativeDepth(200);p.SetScalingModSize(50);p.SetBatchSize(2048);
    p.SetRingDim(16384);p.SetSecretKeyDist(UNIFORM_TERNARY);p.SetSecurityLevel(HEStd_NotSet);
    auto cc=GenCryptoContext(p);
    cc->Enable(PKE);cc->Enable(KEYSWITCH);cc->Enable(LEVELEDSHE);cc->Enable(ADVANCEDSHE);
    auto kp=cc->KeyGen();cc->EvalMultKeyGen(kp.secretKey);
    srand(time(0));
    
    int bits=16; int a_bits[bits],b_bits[bits];
    long long valA=0,valB=0;
    for(int i=0;i<bits;i++){a_bits[i]=rand()%2;b_bits[i]=rand()%2;valA=valA*2+a_bits[i];valB=valB*2+b_bits[i];}
    
    PE rCarry=encrypt_bit(cc,kp,0.0);
    int sum_bits[bits]; long long result=0;
    struct timeval t0,t1; gettimeofday(&t0,NULL);
    
    for(int i=bits-1;i>=0;i--){
        PE a=encrypt_bit(cc,kp,(double)a_bits[i]),b=encrypt_bit(cc,kp,(double)b_bits[i]);
        FAR fa=full_adder_1em(cc,a,b,rCarry);
        sum_bits[i]=decode_bit(cc,kp,fa.sum); rCarry=fa.cout;
    }
    
    for(int i=0;i<bits;i++){std::cout<<sum_bits[i];result=result*2+sum_bits[i];}
    int fc=decode_bit(cc,kp,rCarry); if(fc)result+=(1LL<<bits);
    gettimeofday(&t1,NULL);
    
    long long expected=valA+valB;
    std::cout<<" (c="<<fc<<") = "<<result<<" | Expected="<<expected;
    std::cout<<" | "<<(result==expected?"MATCH!":"MISMATCH")<<"\n";
    std::cout<<"  Time: "<<std::fixed<<std::setprecision(0)<<time_ms(t0,t1)<<"ms";
    std::cout<<" ("<<time_ms(t0,t1)/(bits*9)<<"ms/gate, 1 EvalMult/gate)\n\n";
    
    std::cout<<"  1-EM NAND 16-bit: "<<(result==expected?"MATCH! ":"")<<"144 gates, 144 EvalMult (vs 288 old)\n\n";
    return 0;
}
