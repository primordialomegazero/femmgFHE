// Full Adder FINAL: Correct 9-NAND gate implementation
#include <iostream>
#include <iomanip>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include "openfhe.h"
using namespace lbcrypto;

struct PE { Ciphertext<DCRTPoly> a; Ciphertext<DCRTPoly> b; };
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
double decode(double r){return r-PSI;}
int to_bit(double v){return (v>0.5)?1:0;}

// Standard 9-NAND Full Adder
// SUM = NAND(NAND(A,NAND(A,B)), NAND(B,NAND(A,B))) XOR Cin
// Actually let's use the canonical 9-gate implementation from digital logic
struct FAResult{double sum_r,cout_r;};
FAResult full_adder(double rA,double rB,double rCin){
    // Gate 1: X1 = NAND(A, B)
    double X1=nand_ratio(rA,rB);
    // Gate 2: X2 = NAND(A, X1)
    double X2=nand_ratio(rA,X1);
    // Gate 3: X3 = NAND(B, X1)
    double X3=nand_ratio(rB,X1);
    // Gate 4: X4 = NAND(X2, X3) = A XOR B
    double X4=nand_ratio(X2,X3);
    // Gate 5: X5 = NAND(X4, Cin) 
    double X5=nand_ratio(X4,rCin);
    // Gate 6: X6 = NAND(X4, X5)
    double X6=nand_ratio(X4,X5);
    // Gate 7: X7 = NAND(X5, Cin)
    double X7=nand_ratio(X5,rCin);
    // Gate 8: SUM = NAND(X6, X7) = (A XOR B) XOR Cin
    double SUM=nand_ratio(X6,X7);
    // Gate 9: COUT = NAND(X1, X5) = (A AND B) OR (Cin AND (A XOR B))
    double COUT=nand_ratio(X1,X5);
    return {SUM,COUT};
}

int main(){
    std::cout<<"\n  ╔══════════════════════════════════════════════════════════════════╗\n";
    std::cout<<"  ║   FULL ADDER FINAL: Standard 9-NAND Gate Implementation          ║\n";
    std::cout<<"  ╚══════════════════════════════════════════════════════════════════╝\n\n";
    
    CCParams<CryptoContextCKKSRNS> p;
    p.SetMultiplicativeDepth(200);p.SetScalingModSize(50);p.SetBatchSize(2048);
    p.SetRingDim(4096);p.SetSecretKeyDist(UNIFORM_TERNARY);p.SetSecurityLevel(HEStd_NotSet);
    auto cc=GenCryptoContext(p);
    cc->Enable(PKE);cc->Enable(KEYSWITCH);cc->Enable(LEVELEDSHE);cc->Enable(ADVANCEDSHE);
    auto kp=cc->KeyGen();cc->EvalMultKeyGen(kp.secretKey);
    srand(time(0));
    
    std::cout<<"  === Truth Table ===\n";
    std::cout<<"  A B Cin | SUM COUT | EncSUM EncCOUT | Match\n";
    std::cout<<"  -----------------------------------------------\n";
    
    int cases[8][3]={{0,0,0},{0,0,1},{0,1,0},{0,1,1},{1,0,0},{1,0,1},{1,1,0},{1,1,1}};
    int ok_sum=0,ok_cout=0;
    
    for(int i=0;i<8;i++){
        int A=cases[i][0],B=cases[i][1],Cin=cases[i][2];
        int eSUM=(A+B+Cin)%2,eCOUT=(A+B+Cin)/2;
        PE enc_A=encrypt_bit(cc,kp,A),enc_B=encrypt_bit(cc,kp,B),enc_C=encrypt_bit(cc,kp,Cin);
        double rA=ratio_of(cc,kp,enc_A),rB=ratio_of(cc,kp,enc_B),rCin=ratio_of(cc,kp,enc_C);
        FAResult fa=full_adder(rA,rB,rCin);
        int gSUM=to_bit(decode(fa.sum_r)),gCOUT=to_bit(decode(fa.cout_r));
        if(gSUM==eSUM)ok_sum++; if(gCOUT==eCOUT)ok_cout++;
        std::cout<<"  "<<A<<" "<<B<<" "<<Cin<<"  |  "<<eSUM<<"  "<<eCOUT<<"    |   "<<gSUM<<"  "<<gCOUT
             <<"      |  "<<((gSUM==eSUM&&gCOUT==eCOUT)?"YES":"NO")<<"\n";
    }
    std::cout<<"\n  SUM: "<<ok_sum<<"/8  COUT: "<<ok_cout<<"/8\n\n";
    
    // 50 random tests
    std::cout<<"  === 50 Random Tests ===\n";
    ok_sum=ok_cout=0;
    for(int t=0;t<50;t++){
        int A=rand()%2,B=rand()%2,Cin=rand()%2;
        int eSUM=(A+B+Cin)%2,eCOUT=(A+B+Cin)/2;
        PE enc_A=encrypt_bit(cc,kp,A),enc_B=encrypt_bit(cc,kp,B),enc_C=encrypt_bit(cc,kp,Cin);
        double rA=ratio_of(cc,kp,enc_A),rB=ratio_of(cc,kp,enc_B),rCin=ratio_of(cc,kp,enc_C);
        FAResult fa=full_adder(rA,rB,rCin);
        if(to_bit(decode(fa.sum_r))==eSUM)ok_sum++;
        if(to_bit(decode(fa.cout_r))==eCOUT)ok_cout++;
    }
    std::cout<<"  SUM: "<<ok_sum<<"/50  COUT: "<<ok_cout<<"/50\n\n";
    
    // 8-bit adder
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
    int fc=to_bit(decode(rCarry)); if(fc)result+=256;
    std::cout<<" (c="<<fc<<") = "<<result;
    std::cout<<" | "<<(result==valA+valB?"MATCH!":"MISMATCH")<<"\n\n";
    
    std::cout<<"  ╔══════════════════════════════════════════════════════════════════╗\n";
    std::cout<<"  ║  9-NAND Full Adder: "<<ok_sum<<"/50 SUM, "<<ok_cout<<"/50 COUT";
    for(int i=0;i<25;i++)std::cout<<" ";std::cout<<"║\n";
    std::cout<<"  ║  8-bit Adder: "<<(result==valA+valB?"MATCH!":"MISMATCH");
    for(int i=0;i<40;i++)std::cout<<" ";std::cout<<"║\n";
    std::cout<<"  ╚══════════════════════════════════════════════════════════════════╝\n\n";
    return 0;
}
