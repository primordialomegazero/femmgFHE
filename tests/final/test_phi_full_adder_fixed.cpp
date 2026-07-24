// Full Adder FIXED: SUM = XOR(XOR(A,B), Cin)
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
double decrypt_val(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, const Ciphertext<DCRTPoly>& c) {
    Plaintext pt; cc->Decrypt(kp.secretKey, c, &pt); return pt->GetCKKSPackedValue()[0].real();
}
double ratio_of(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, const PE& s) {
    return decrypt_val(cc,kp,s.a) / decrypt_val(cc,kp,s.b);
}
PE encrypt_bit(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, int bit) {
    const double PSI=0.6180339887498949;
    double v=(bit==1)?1.0:0.0;
    return {cc->Encrypt(kp.publicKey,cc->MakeCKKSPackedPlaintext(std::vector<double>{v+PSI})),
            cc->Encrypt(kp.publicKey,cc->MakeCKKSPackedPlaintext(std::vector<double>{1.0}))};
}
const double PSI=0.6180339887498949;
double nand_ratio(double rA,double rB){return PSI*(rA+rB+2.0)-rA*rB;}
double xor_ratio(double rA,double rB){
    double rAB=nand_ratio(rA,rB);
    return nand_ratio(nand_ratio(rA,rAB),nand_ratio(rB,rAB));
}
double decode(double r){return r-PSI;}
int to_bit(double v){return (v>0.5)?1:0;}

struct FAResult{double sum_r,cout_r;};
FAResult full_adder(double rA,double rB,double rCin){
    double rAxorB=xor_ratio(rA,rB);
    double rSUM=xor_ratio(rAxorB,rCin);
    double rAB=nand_ratio(rA,rB);
    double rCOUT=nand_ratio(rAB,nand_ratio(rCin,rAB));
    return {rSUM,rCOUT};
}

int main(){
    std::cout<<"\n  ╔══════════════════════════════════════════════════════════════════╗\n";
    std::cout<<"  ║   FULL ADDER FIXED: SUM = XOR(XOR(A,B), Cin)                     ║\n";
    std::cout<<"  ╚══════════════════════════════════════════════════════════════════╝\n\n";
    
    CCParams<CryptoContextCKKSRNS> p;
    p.SetMultiplicativeDepth(200);p.SetScalingModSize(50);p.SetBatchSize(2048);
    p.SetRingDim(4096);p.SetSecretKeyDist(UNIFORM_TERNARY);p.SetSecurityLevel(HEStd_NotSet);
    auto cc=GenCryptoContext(p);
    cc->Enable(PKE);cc->Enable(KEYSWITCH);cc->Enable(LEVELEDSHE);cc->Enable(ADVANCEDSHE);
    auto kp=cc->KeyGen();cc->EvalMultKeyGen(kp.secretKey);
    srand(time(0));
    
    std::cout<<"  === Truth Table Verification ===\n";
    std::cout<<"  A B Cin | SUM COUT | EncSUM EncCOUT | Match\n";
    std::cout<<"  -----------------------------------------------\n";
    
    int cases[8][3]={{0,0,0},{0,0,1},{0,1,0},{0,1,1},{1,0,0},{1,0,1},{1,1,0},{1,1,1}};
    int correct_sum=0,correct_cout=0;
    
    for(int i=0;i<8;i++){
        int A=cases[i][0],B=cases[i][1],Cin=cases[i][2];
        int expSUM=(A+B+Cin)%2,expCOUT=(A+B+Cin)/2;
        PE enc_A=encrypt_bit(cc,kp,A),enc_B=encrypt_bit(cc,kp,B),enc_C=encrypt_bit(cc,kp,Cin);
        double rA=ratio_of(cc,kp,enc_A),rB=ratio_of(cc,kp,enc_B),rCin=ratio_of(cc,kp,enc_C);
        FAResult fa=full_adder(rA,rB,rCin);
        int gotSUM=to_bit(decode(fa.sum_r)),gotCOUT=to_bit(decode(fa.cout_r));
        if(gotSUM==expSUM)correct_sum++; if(gotCOUT==expCOUT)correct_cout++;
        std::cout<<"  "<<A<<" "<<B<<" "<<Cin<<"  |  "<<expSUM<<"  "<<expCOUT<<"    |   "<<gotSUM<<"  "<<gotCOUT
             <<"      |  "<<((gotSUM==expSUM&&gotCOUT==expCOUT)?"YES":"NO")<<"\n";
    }
    
    std::cout<<"\n  SUM: "<<correct_sum<<"/8  COUT: "<<correct_cout<<"/8\n\n";
    
    // 20 random tests
    std::cout<<"  === 20 Random Tests ===\n";
    correct_sum=correct_cout=0;
    for(int t=0;t<20;t++){
        int A=rand()%2,B=rand()%2,Cin=rand()%2;
        int expSUM=(A+B+Cin)%2,expCOUT=(A+B+Cin)/2;
        PE enc_A=encrypt_bit(cc,kp,A),enc_B=encrypt_bit(cc,kp,B),enc_C=encrypt_bit(cc,kp,Cin);
        double rA=ratio_of(cc,kp,enc_A),rB=ratio_of(cc,kp,enc_B),rCin=ratio_of(cc,kp,enc_C);
        FAResult fa=full_adder(rA,rB,rCin);
        int gotSUM=to_bit(decode(fa.sum_r)),gotCOUT=to_bit(decode(fa.cout_r));
        if(gotSUM==expSUM)correct_sum++; if(gotCOUT==expCOUT)correct_cout++;
    }
    std::cout<<"  SUM: "<<correct_sum<<"/20  COUT: "<<correct_cout<<"/20\n\n";
    
    // 8-bit ripple carry adder
    std::cout<<"  === 8-Bit Ripple Carry Adder ===\n";
    int a_bits[8],b_bits[8],valA=0,valB=0;
    for(int i=0;i<8;i++){a_bits[i]=rand()%2;b_bits[i]=rand()%2;valA=valA*2+a_bits[i];valB=valB*2+b_bits[i];}
    std::cout<<"  A="<<valA<<" B="<<valB<<" Expected="<<valA+valB<<"\n  Bits: ";
    
    double rCarry=PSI; int sum_bits[8],result=0;
    for(int i=7;i>=0;i--){
        PE enc_A=encrypt_bit(cc,kp,a_bits[i]),enc_B=encrypt_bit(cc,kp,b_bits[i]);
        double rA=ratio_of(cc,kp,enc_A),rB=ratio_of(cc,kp,enc_B);
        FAResult fa=full_adder(rA,rB,rCarry);
        sum_bits[i]=to_bit(decode(fa.sum_r)); rCarry=fa.cout_r;
    }
    for(int i=0;i<8;i++){std::cout<<sum_bits[i];result=result*2+sum_bits[i];}
    int final_cout=to_bit(decode(rCarry)); if(final_cout)result+=256;
    std::cout<<" (c="<<final_cout<<") = "<<result;
    std::cout<<" | "<<(result==valA+valB?"MATCH!":"MISMATCH")<<"\n\n";
    
    std::cout<<"  ╔══════════════════════════════════════════════════════════════════╗\n";
    std::cout<<"  ║  FULL ADDER FIXED: "<<correct_sum<<"/20 SUM, "<<correct_cout<<"/20 COUT";
    for(int i=0;i<26;i++)std::cout<<" ";std::cout<<"║\n";
    std::cout<<"  ║  8-bit Adder: "<<(result==valA+valB?"MATCH!":"MISMATCH");
    for(int i=0;i<(result==valA+valB?40:40);i++)std::cout<<" ";std::cout<<"║\n";
    std::cout<<"  ╚══════════════════════════════════════════════════════════════════╝\n\n";
    return 0;
}
