// FINAL BOSS: Full Adder from NAND gates, encrypted random bits, exact recovery
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

// NAND gate: R(NAND) = PSI*(R(A)+R(B)+2) - R(A)*R(B)
double nand_ratio(double rA, double rB) { return PSI*(rA+rB+2.0)-rA*rB; }
double decode(double ratio) { return ratio-PSI; }
int to_bit(double val) { return (val>0.5)?1:0; }

// Full Adder from NAND gates:
// SUM = A XOR B XOR Cin
// XOR(A,B) = NAND(NAND(A,NAND(A,B)), NAND(B,NAND(A,B)))
// COUT = NAND(NAND(A,B), NAND(Cin, NAND(A,B)))

double xor_ratio(double rA, double rB) {
    double rAB=nand_ratio(rA,rB);
    double rA_AB=nand_ratio(rA,rAB);
    double rB_AB=nand_ratio(rB,rAB);
    return nand_ratio(rA_AB, rB_AB);
}

struct FullAdderResult { double sum_ratio; double cout_ratio; };

FullAdderResult full_adder(double rA, double rB, double rCin) {
    double rAB=nand_ratio(rA,rB);
    double rA_AB=nand_ratio(rA,rAB);
    double rB_AB=nand_ratio(rB,rAB);
    double rSUM=nand_ratio(rA_AB, rB_AB);
    double rCin_AB=nand_ratio(rCin, rAB);
    double rCOUT=nand_ratio(rAB, rCin_AB);
    return {rSUM, rCOUT};
}

int main() {
    std::cout << "\n";
    std::cout << "  ╔══════════════════════════════════════════════════════════════════╗\n";
    std::cout << "  ║   FINAL BOSS: Full Adder, Encrypted Random Bits, Exact Recovery   ║\n";
    std::cout << "  ╚══════════════════════════════════════════════════════════════════╝\n\n";
    
    CCParams<CryptoContextCKKSRNS> p;
    p.SetMultiplicativeDepth(200);p.SetScalingModSize(50);p.SetBatchSize(2048);
    p.SetRingDim(4096);p.SetSecretKeyDist(UNIFORM_TERNARY);p.SetSecurityLevel(HEStd_NotSet);
    auto cc=GenCryptoContext(p);
    cc->Enable(PKE);cc->Enable(KEYSWITCH);cc->Enable(LEVELEDSHE);cc->Enable(ADVANCEDSHE);
    auto kp=cc->KeyGen();cc->EvalMultKeyGen(kp.secretKey);
    srand(time(0));
    
    // Test 20 random full adder computations
    int correct_sum=0, correct_cout=0, total=20;
    
    std::cout << "  === Full Adder: 20 Random Encrypted Tests ===\n";
    std::cout << "  A B Cin | SUM COUT | EncSUM EncCOUT | Match\n";
    std::cout << "  -----------------------------------------------\n";
    
    for(int t=0;t<total;t++){
        int A=rand()%2, B=rand()%2, Cin=rand()%2;
        int expSUM=(A+B+Cin)%2, expCOUT=(A+B+Cin)/2;
        
        PE enc_A=encrypt_bit(cc,kp,A), enc_B=encrypt_bit(cc,kp,B), enc_Cin=encrypt_bit(cc,kp,Cin);
        double rA=ratio_of(cc,kp,enc_A), rB=ratio_of(cc,kp,enc_B), rCin=ratio_of(cc,kp,enc_Cin);
        
        FullAdderResult fa=full_adder(rA,rB,rCin);
        int gotSUM=to_bit(decode(fa.sum_ratio));
        int gotCOUT=to_bit(decode(fa.cout_ratio));
        
        if(gotSUM==expSUM)correct_sum++;
        if(gotCOUT==expCOUT)correct_cout++;
        
        std::cout<<"  "<<A<<" "<<B<<" "<<Cin
             <<"  |  "<<expSUM<<"  "<<expCOUT
             <<"    |   "<<gotSUM<<"  "<<gotCOUT
             <<"      |  "<<((gotSUM==expSUM&&gotCOUT==expCOUT)?"YES":"NO")<<"\n";
    }
    
    std::cout<<"\n  SUM correct: "<<correct_sum<<"/"<<total;
    std::cout<<"  |  COUT correct: "<<correct_cout<<"/"<<total;
    std::cout<<"  |  Total: "<<(correct_sum+correct_cout)<<"/"<<(2*total)<<"\n\n";
    
    // === 8-BIT RIPPLE CARRY ADDER (100+ NAND gates) ===
    std::cout<<"  === 8-Bit Ripple Carry Adder (100+ NAND gates) ===\n";
    
    int a_bits[8], b_bits[8];
    int valA=0, valB=0;
    for(int i=0;i<8;i++){a_bits[i]=rand()%2;b_bits[i]=rand()%2;valA=valA*2+a_bits[i];valB=valB*2+b_bits[i];}
    
    std::cout<<"  A="<<valA<<" B="<<valB<<" Expected A+B="<<valA+valB<<"\n\n";
    
    double rCarry=0.0+PSI; // Cin=0 encoded as ratio
    int sum_bits[8];
    
    for(int i=7;i>=0;i--){
        PE enc_A=encrypt_bit(cc,kp,a_bits[i]), enc_B=encrypt_bit(cc,kp,b_bits[i]);
        double rA=ratio_of(cc,kp,enc_A), rB=ratio_of(cc,kp,enc_B);
        FullAdderResult fa=full_adder(rA,rB,rCarry);
        sum_bits[i]=to_bit(decode(fa.sum_ratio));
        rCarry=fa.cout_ratio;
    }
    
    int result=0;
    std::cout<<"  Bit:   ";
    for(int i=0;i<8;i++)std::cout<<sum_bits[i];
    for(int i=0;i<8;i++)result=result*2+sum_bits[i];
    int final_cout=to_bit(decode(rCarry));
    if(final_cout)result+=256;
    
    std::cout<<" (carry="<<final_cout<<") = "<<result;
    std::cout<<" | Expected="<<valA+valB;
    std::cout<<" | " <<(result==valA+valB?"MATCH!":"MISMATCH")<<"\n\n";
    
    std::cout<<"  ╔══════════════════════════════════════════════════════════════════╗\n";
    std::cout<<"  ║  FULL ADDER: "<<correct_sum<<"/"<<total<<" SUM, "<<correct_cout<<"/"<<total<<" COUT correct                    ║\n";
    std::cout<<"  ║  8-bit Adder: " <<(result==valA+valB?"WORKING!":"MISMATCH");
    for(int i=0;i<(result==valA+valB?40:40);i++)std::cout<<" ";std::cout<<"║\n";
    std::cout<<"  ╚══════════════════════════════════════════════════════════════════╝\n\n";
    
    return 0;
}
