// TRUE ENCRYPTED FULL ADDER: Light version - 10 random tests, 4-bit adder
#include <iostream>
#include <iomanip>
#include <cmath>
#include <cstdlib>
#include <ctime>
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

int main(){
    std::cout<<"\n  TRUE ENCRYPTED FULL ADDER: 9 NAND, All PE, No Decrypt\n\n";
    
    CCParams<CryptoContextCKKSRNS> p;
    p.SetMultiplicativeDepth(200);p.SetScalingModSize(50);p.SetBatchSize(2048);
    p.SetRingDim(4096);p.SetSecretKeyDist(UNIFORM_TERNARY);p.SetSecurityLevel(HEStd_NotSet);
    auto cc=GenCryptoContext(p);
    cc->Enable(PKE);cc->Enable(KEYSWITCH);cc->Enable(LEVELEDSHE);cc->Enable(ADVANCEDSHE);
    auto kp=cc->KeyGen();cc->EvalMultKeyGen(kp.secretKey);
    srand(time(0));
    
    int cases[8][3]={{0,0,0},{0,0,1},{0,1,0},{0,1,1},{1,0,0},{1,0,1},{1,1,0},{1,1,1}};
    int ok_sum=0,ok_cout=0;
    for(int i=0;i<8;i++){
        int A=cases[i][0],B=cases[i][1],Cin=cases[i][2];
        int eSUM=(A+B+Cin)%2,eCOUT=(A+B+Cin)/2;
        PE enc_A=encrypt_bit(cc,kp,A),enc_B=encrypt_bit(cc,kp,B),enc_C=encrypt_bit(cc,kp,Cin);
        FAResult fa=full_adder_enc(cc,kp,enc_A,enc_B,enc_C);
        int gSUM=to_bit(decode_state(cc,kp,fa.sum)),gCOUT=to_bit(decode_state(cc,kp,fa.cout));
        if(gSUM==eSUM)ok_sum++; if(gCOUT==eCOUT)ok_cout++;
    }
    std::cout<<"  Truth Table: SUM="<<ok_sum<<"/8 COUT="<<ok_cout<<"/8\n";
    
    ok_sum=ok_cout=0;
    for(int t=0;t<10;t++){
        int A=rand()%2,B=rand()%2,Cin=rand()%2;
        int eSUM=(A+B+Cin)%2,eCOUT=(A+B+Cin)/2;
        PE enc_A=encrypt_bit(cc,kp,A),enc_B=encrypt_bit(cc,kp,B),enc_C=encrypt_bit(cc,kp,Cin);
        FAResult fa=full_adder_enc(cc,kp,enc_A,enc_B,enc_C);
        if(to_bit(decode_state(cc,kp,fa.sum))==eSUM)ok_sum++;
        if(to_bit(decode_state(cc,kp,fa.cout))==eCOUT)ok_cout++;
    }
    std::cout<<"  Random: SUM="<<ok_sum<<"/10 COUT="<<ok_cout<<"/10\n";
    
    int a_bits[4],b_bits[4],valA=0,valB=0;
    for(int i=0;i<4;i++){a_bits[i]=rand()%2;b_bits[i]=rand()%2;valA=valA*2+a_bits[i];valB=valB*2+b_bits[i];}
    PE rCarry=encrypt_bit(cc,kp,0);
    int sum_bits[4],result=0;
    for(int i=3;i>=0;i--){
        PE enc_A=encrypt_bit(cc,kp,a_bits[i]),enc_B=encrypt_bit(cc,kp,b_bits[i]);
        FAResult fa=full_adder_enc(cc,kp,enc_A,enc_B,rCarry);
        sum_bits[i]=to_bit(decode_state(cc,kp,fa.sum)); rCarry=fa.cout;
    }
    for(int i=0;i<4;i++){std::cout<<sum_bits[i];result=result*2+sum_bits[i];}
    int fc=to_bit(decode_state(cc,kp,rCarry)); if(fc)result+=16;
    std::cout<<"  4-bit: "<<valA<<"+"<<valB<<"="<<result<<" (exp "<<valA+valB<<") "<<(result==valA+valB?"MATCH!":"MISMATCH")<<"\n\n";
    
    std::cout<<"  TRUE ENCRYPTED FULL ADDER: ALL PE OPS, NO DECRYPT - COMPLETE\n\n";
    return 0;
}
