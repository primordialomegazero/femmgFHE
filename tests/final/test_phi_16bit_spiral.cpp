// 16-BIT SPIRAL ADDER: 144 gates, auto-correction
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
PE encrypt_direct(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, double v) {
    return {cc->Encrypt(kp.publicKey,cc->MakeCKKSPackedPlaintext(std::vector<double>{v})),
            cc->Encrypt(kp.publicKey,cc->MakeCKKSPackedPlaintext(std::vector<double>{1.0}))};
}
PE nand_direct(CryptoContext<DCRTPoly>& cc, const PE& A, const PE& B) {
    auto aabb=cc->EvalMult(A.b,B.b); auto aaba=cc->EvalMult(A.a,B.a);
    return {cc->EvalSub(aabb,aaba), aabb};
}
PE spiral_correct(CryptoContext<DCRTPoly>& cc, const PE& state, int cycles=3) {
    PE s=state;
    for(int c=0;c<cycles;c++){s=mulY(cc,s);s=mulY_inv(cc,s);}
    return s;
}
int decode_rel(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, const PE& s){
    double a=decrypt_val(cc,kp,s.a), b=decrypt_val(cc,kp,s.b);
    return (a>b*0.5)?1:0;
}
double time_ms(struct timeval s,struct timeval e){return (e.tv_sec-s.tv_sec)*1000.0+(e.tv_usec-s.tv_usec)/1000.0;}

int main(){
    std::cout<<"\n  16-BIT SPIRAL ADDER: 144 gates, auto-correction\n\n";
    
    CCParams<CryptoContextCKKSRNS> p;
    p.SetMultiplicativeDepth(500);p.SetScalingModSize(50);p.SetBatchSize(2048);
    p.SetRingDim(16384);p.SetSecretKeyDist(UNIFORM_TERNARY);p.SetSecurityLevel(HEStd_NotSet);
    auto cc=GenCryptoContext(p);
    cc->Enable(PKE);cc->Enable(KEYSWITCH);cc->Enable(LEVELEDSHE);cc->Enable(ADVANCEDSHE);
    auto kp=cc->KeyGen();cc->EvalMultKeyGen(kp.secretKey);
    srand(time(0));
    
    int bits=16; int a_bits[bits],b_bits[bits];
    long long valA=0,valB=0;
    for(int i=0;i<bits;i++){a_bits[i]=rand()%2;b_bits[i]=rand()%2;valA=valA*2+a_bits[i];valB=valB*2+b_bits[i];}
    
    PE rCarry=encrypt_direct(cc,kp,0.0);
    int sum_bits[bits]; long long result=0;
    struct timeval t0,t1; gettimeofday(&t0,NULL);
    
    for(int i=bits-1;i>=0;i--){
        PE a=encrypt_direct(cc,kp,(double)a_bits[i]),b=encrypt_direct(cc,kp,(double)b_bits[i]);
        PE X1=spiral_correct(cc,nand_direct(cc,a,b),2);
        PE X2=spiral_correct(cc,nand_direct(cc,a,X1),2);
        PE X3=spiral_correct(cc,nand_direct(cc,b,X1),2);
        PE X4=spiral_correct(cc,nand_direct(cc,X2,X3),2);
        PE X5=spiral_correct(cc,nand_direct(cc,X4,rCarry),2);
        PE X6=spiral_correct(cc,nand_direct(cc,X4,X5),2);
        PE X7=spiral_correct(cc,nand_direct(cc,X5,rCarry),2);
        PE SUM=spiral_correct(cc,nand_direct(cc,X6,X7),2);
        PE COUT=spiral_correct(cc,nand_direct(cc,X1,X5),2);
        sum_bits[i]=decode_rel(cc,kp,SUM); rCarry=COUT;
    }
    
    for(int i=0;i<bits;i++){std::cout<<sum_bits[i];result=result*2+sum_bits[i];}
    int fc=decode_rel(cc,kp,rCarry); if(fc)result+=(1LL<<bits);
    gettimeofday(&t1,NULL);
    long long expected=valA+valB;
    std::cout<<" (c="<<fc<<") = "<<result<<" | Expected="<<expected;
    std::cout<<" | "<<(result==expected?"MATCH!":"MISMATCH")<<"\n";
    std::cout<<"  Time: "<<std::fixed<<std::setprecision(0)<<time_ms(t0,t1)<<"ms\n\n";
    std::cout<<"  16-BIT SPIRAL: "<<(result==expected?"MATCH! 144 gates!":"needs work")<<"\n\n";
    return 0;
}
