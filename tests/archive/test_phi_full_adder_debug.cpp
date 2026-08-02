// Debug: Print actual decoded SUM/COUT values, check noise margin
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

double nand_ratio(double rA, double rB) { return PSI*(rA+rB+2.0)-rA*rB; }
double decode(double ratio) { return ratio-PSI; }

int main() {
    std::cout << "\n";
    std::cout << "  ╔══════════════════════════════════════════════════════════════════╗\n";
    std::cout << "  ║   Noise Margin Analysis: Actual Decoded Values                   ║\n";
    std::cout << "  ╚══════════════════════════════════════════════════════════════════╝\n\n";
    
    CCParams<CryptoContextCKKSRNS> p;
    p.SetMultiplicativeDepth(200);p.SetScalingModSize(50);p.SetBatchSize(2048);
    p.SetRingDim(4096);p.SetSecretKeyDist(UNIFORM_TERNARY);p.SetSecurityLevel(HEStd_NotSet);
    auto cc=GenCryptoContext(p);
    cc->Enable(PKE);cc->Enable(KEYSWITCH);cc->Enable(LEVELEDSHE);cc->Enable(ADVANCEDSHE);
    auto kp=cc->KeyGen();cc->EvalMultKeyGen(kp.secretKey);
    srand(time(0));
    
    std::cout << "  === NAND Gate: Single Gate Noise ===\n";
    std::cout << "  A B | R(NAND) decoded | Expected | Margin\n";
    std::cout << "  -----------------------------------------------\n";
    
    double inputs[4][2]={{0,0},{0,1},{1,0},{1,1}};
    for(int i=0;i<4;i++){
        int A=inputs[i][0], B=inputs[i][1];
        int expected=1-A*B;
        PE enc_A=encrypt_bit(cc,kp,A), enc_B=encrypt_bit(cc,kp,B);
        double rA=ratio_of(cc,kp,enc_A), rB=ratio_of(cc,kp,enc_B);
        double decoded=decode(nand_ratio(rA,rB));
        double margin=std::abs(decoded-0.5);
        std::cout<<"  "<<A<<" "<<B<<" | "<<std::fixed<<std::setprecision(10)<<decoded
             <<"       | "<<expected<<"      | "<<std::setprecision(6)<<margin<<"\n";
    }
    
    std::cout<<"\n  === XOR Gate: 4 NAND Gates, Noise Accumulation ===\n";
    std::cout<<"  A B | XOR decoded  | Expected | Margin\n";
    std::cout<<"  -----------------------------------------------\n";
    
    for(int i=0;i<4;i++){
        int A=inputs[i][0], B=inputs[i][1];
        int expected=A^B;
        PE enc_A=encrypt_bit(cc,kp,A), enc_B=encrypt_bit(cc,kp,B);
        double rA=ratio_of(cc,kp,enc_A), rB=ratio_of(cc,kp,enc_B);
        
        double rAB=nand_ratio(rA,rB);
        double rA_AB=nand_ratio(rA,rAB);
        double rB_AB=nand_ratio(rB,rAB);
        double rXOR=nand_ratio(rA_AB,rB_AB);
        double decoded=decode(rXOR);
        double margin=std::abs(decoded-0.5);
        
        std::cout<<"  "<<A<<" "<<B<<" | "<<std::fixed<<std::setprecision(10)<<decoded
             <<"       | "<<expected<<"      | "<<std::setprecision(6)<<margin<<"\n";
    }
    
    std::cout<<"\n  === Full Adder: 5 NAND Gates for XOR path ===\n";
    std::cout<<"  A B Cin | SUM decoded  | Expected | Margin\n";
    std::cout<<"  --------------------------------------------------\n";
    
    int cases[8][3]={{0,0,0},{0,0,1},{0,1,0},{0,1,1},{1,0,0},{1,0,1},{1,1,0},{1,1,1}};
    double min_margin=999;
    
    for(int i=0;i<8;i++){
        int A=cases[i][0], B=cases[i][1], Cin=cases[i][2];
        int expected=(A+B+Cin)%2;
        PE enc_A=encrypt_bit(cc,kp,A), enc_B=encrypt_bit(cc,kp,B), enc_C=encrypt_bit(cc,kp,Cin);
        double rA=ratio_of(cc,kp,enc_A), rB=ratio_of(cc,kp,enc_B), rCin=ratio_of(cc,kp,enc_C);
        
        double rAB=nand_ratio(rA,rB);
        double rA_AB=nand_ratio(rA,rAB);
        double rB_AB=nand_ratio(rB,rAB);
        double rSUM=nand_ratio(rA_AB,rB_AB);
        double decoded=decode(rSUM);
        double margin=std::abs(decoded-0.5);
        if(margin<min_margin)min_margin=margin;
        
        std::cout<<"  "<<A<<" "<<B<<" "<<Cin<<"  | "<<std::fixed<<std::setprecision(10)<<decoded
             <<"       | "<<expected<<"      | "<<std::setprecision(6)<<margin<<"\n";
    }
    
    std::cout<<"\n  Minimum margin from threshold: "<<std::fixed<<std::setprecision(6)<<min_margin;
    if(min_margin<0.1)std::cout<<" -- NOISE ISSUE: margin too small!";
    else std::cout<<" -- OK";
    std::cout<<"\n\n";
    
    std::cout<<"  ╔══════════════════════════════════════════════════════════════════╗\n";
    std::cout<<"  ║  Noise margin analysis complete                                   ║\n";
    std::cout<<"  ╚══════════════════════════════════════════════════════════════════╝\n\n";
    return 0;
}
