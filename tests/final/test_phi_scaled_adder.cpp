// SCALED ENCRYPTED ADDER: 4-bit, 8-bit, 16-bit with full timing
#include <iostream>
#include <iomanip>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <sys/time.h>
#include "openfhe.h"
using namespace lbcrypto;

struct PE { Ciphertext<DCRTPoly> a; Ciphertext<DCRTPoly> b; };
PE ratio_add(CryptoContext<DCRTPoly>& cc, const PE& x, const PE& y) {
    auto a1b2=cc->EvalMult(x.a,y.b), a2b1=cc->EvalMult(y.a,x.b), b1b2=cc->EvalMult(x.b,y.b);
    return {cc->EvalAdd(a1b2,a2b1), b1b2};
}
PE ratio_mult(CryptoContext<DCRTPoly>& cc, const PE& x, const PE& y) {
    return {cc->EvalMult(x.a,y.a), cc->EvalMult(x.b,y.b)};
}
PE ratio_add_constant(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, const PE& x, double C) {
    auto ptC=cc->MakeCKKSPackedPlaintext(std::vector<double>{C});
    auto ctC=cc->Encrypt(kp.publicKey,ptC);
    return {cc->EvalAdd(x.a, cc->EvalMult(x.b,ctC)), x.b};
}
PE ratio_mul_constant(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, const PE& x, double K) {
    auto ptK=cc->MakeCKKSPackedPlaintext(std::vector<double>{K});
    auto ctK=cc->Encrypt(kp.publicKey,ptK);
    return {cc->EvalMult(x.a,ctK), x.b};
}
PE ratio_sub(CryptoContext<DCRTPoly>& cc, const PE& x, const PE& y) {
    auto num1=cc->EvalMult(x.a,y.b), num2=cc->EvalMult(y.a,x.b);
    auto den=cc->EvalMult(x.b,y.b);
    return {cc->EvalSub(num1,num2), den};
}
double decrypt_val(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, const Ciphertext<DCRTPoly>& c) {
    Plaintext pt; cc->Decrypt(kp.secretKey, c, &pt); return pt->GetCKKSPackedValue()[0].real();
}
double decode_state(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, const PE& s) {
    return decrypt_val(cc,kp,s.a)/decrypt_val(cc,kp,s.b) - 0.6180339887498949;
}
PE encrypt_bit(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, int bit) {
    const double PSI=0.6180339887498949;
    double v=(bit==1)?1.0:0.0;
    return {cc->Encrypt(kp.publicKey,cc->MakeCKKSPackedPlaintext(std::vector<double>{v+PSI})),
            cc->Encrypt(kp.publicKey,cc->MakeCKKSPackedPlaintext(std::vector<double>{1.0}))};
}
const double PSI=0.6180339887498949;

PE nand_enc(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, const PE& A, const PE& B) {
    PE psi_A=ratio_mul_constant(cc,kp,A,PSI);
    PE psi_B=ratio_mul_constant(cc,kp,B,PSI);
    PE psi_sum=ratio_add(cc,psi_A,psi_B);
    PE term1=ratio_add_constant(cc,kp,psi_sum,2.0*PSI);
    PE prod=ratio_mult(cc,A,B);
    return ratio_sub(cc,term1,prod);
}

struct FAResult{PE sum; PE cout;};
FAResult full_adder_enc(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, const PE& A, const PE& B, const PE& Cin){
    PE X1=nand_enc(cc,kp,A,B);
    PE X2=nand_enc(cc,kp,A,X1);
    PE X3=nand_enc(cc,kp,B,X1);
    PE X4=nand_enc(cc,kp,X2,X3);
    PE X5=nand_enc(cc,kp,X4,Cin);
    PE X6=nand_enc(cc,kp,X4,X5);
    PE X7=nand_enc(cc,kp,X5,Cin);
    PE SUM=nand_enc(cc,kp,X6,X7);
    PE COUT=nand_enc(cc,kp,X1,X5);
    return {SUM,COUT};
}

int to_bit(double v){return (v>0.5)?1:0;}
double time_ms(struct timeval start, struct timeval end){
    return (end.tv_sec-start.tv_sec)*1000.0 + (end.tv_usec-start.tv_usec)/1000.0;
}

int main(){
    std::cout<<"\n  SCALED ENCRYPTED ADDER: 4-bit, 8-bit, 16-bit with Full Timing\n\n";
    
    CCParams<CryptoContextCKKSRNS> p;
    p.SetMultiplicativeDepth(500);p.SetScalingModSize(50);p.SetBatchSize(2048);
    p.SetRingDim(4096);p.SetSecretKeyDist(UNIFORM_TERNARY);p.SetSecurityLevel(HEStd_NotSet);
    auto cc=GenCryptoContext(p);
    cc->Enable(PKE);cc->Enable(KEYSWITCH);cc->Enable(LEVELEDSHE);cc->Enable(ADVANCEDSHE);
    auto kp=cc->KeyGen();cc->EvalMultKeyGen(kp.secretKey);
    srand(time(0));
    
    struct timeval t_start, t_end;
    int bit_widths[]={4,8,16};
    
    std::cout<<"  Bits | Gates | Time (ms) | ms/gate | Expected | Got | Match\n";
    std::cout<<"  -----------------------------------------------------------\n";
    
    for(int w=0;w<3;w++){
        int bits=bit_widths[w];
        int gates=bits*9;
        
        int a_bits[bits],b_bits[bits];
        long long valA=0,valB=0;
        for(int i=0;i<bits;i++){a_bits[i]=rand()%2;b_bits[i]=rand()%2;valA=valA*2+a_bits[i];valB=valB*2+b_bits[i];}
        
        gettimeofday(&t_start,NULL);
        
        PE rCarry=encrypt_bit(cc,kp,0);
        int sum_bits[bits]; long long result=0;
        for(int i=bits-1;i>=0;i--){
            PE enc_A=encrypt_bit(cc,kp,a_bits[i]), enc_B=encrypt_bit(cc,kp,b_bits[i]);
            FAResult fa=full_adder_enc(cc,kp,enc_A,enc_B,rCarry);
            sum_bits[i]=to_bit(decode_state(cc,kp,fa.sum));
            rCarry=fa.cout;
        }
        for(int i=0;i<bits;i++) result=result*2+sum_bits[i];
        int fc=to_bit(decode_state(cc,kp,rCarry));
        long long expected=valA+valB;
        if(fc) result+=(1LL<<bits);
        
        gettimeofday(&t_end,NULL);
        double ms=time_ms(t_start,t_end);
        
        std::cout<<"  "<<std::setw(4)<<bits<<" | "<<std::setw(5)<<gates
             <<" | "<<std::fixed<<std::setprecision(0)<<std::setw(8)<<ms
             <<" | "<<std::fixed<<std::setprecision(1)<<std::setw(6)<<ms/gates
             <<" | "<<std::setw(8)<<expected<<" | "<<std::setw(6)<<result
             <<" | "<<(result==expected?"YES":"NO")<<"\n";
    }
    
    std::cout<<"\n  NAND per gate timing:\n";
    PE A=encrypt_bit(cc,kp,0), B=encrypt_bit(cc,kp,0);
    gettimeofday(&t_start,NULL);
    for(int i=0;i<9;i++){PE X=nand_enc(cc,kp,A,B);}
    gettimeofday(&t_end,NULL);
    double ms_9nand=time_ms(t_start,t_end);
    std::cout<<"  9 NAND gates: "<<std::fixed<<std::setprecision(0)<<ms_9nand<<" ms\n";
    std::cout<<"  Per NAND: "<<std::fixed<<std::setprecision(1)<<ms_9nand/9.0<<" ms\n";
    std::cout<<"  Per Full Adder (9 NAND): "<<std::fixed<<std::setprecision(0)<<ms_9nand<<" ms\n";
    std::cout<<"  Projected 32-bit adder: "<<std::fixed<<std::setprecision(0)<<ms_9nand*32/9.0<<" ms\n\n";
    
    std::cout<<"  SCALED ENCRYPTED ADDER: COMPLETE\n\n";
    return 0;
}
