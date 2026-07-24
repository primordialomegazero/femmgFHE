#include <iostream>
#include <iomanip>
#include <cmath>
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

int main() {
    std::cout << "\n";
    std::cout << "  ╔══════════════════════════════════════════════════════════════════╗\n";
    std::cout << "  ║   Encrypted Ratio Extraction + Full Adder Test                   ║\n";
    std::cout << "  ╚══════════════════════════════════════════════════════════════════╝\n\n";
    
    CCParams<CryptoContextCKKSRNS> p;
    p.SetMultiplicativeDepth(200);p.SetScalingModSize(50);p.SetBatchSize(2048);
    p.SetRingDim(4096);p.SetSecretKeyDist(UNIFORM_TERNARY);p.SetSecurityLevel(HEStd_NotSet);
    auto cc=GenCryptoContext(p);
    cc->Enable(PKE);cc->Enable(KEYSWITCH);cc->Enable(LEVELEDSHE);cc->Enable(ADVANCEDSHE);
    auto kp=cc->KeyGen();cc->EvalMultKeyGen(kp.secretKey);
    
    std::cout << "  === Encrypted Ratio Accuracy ===\n";
    std::cout << "  Bit | Ratio        | Expected     | Error\n";
    std::cout << "  ---------------------------------------------\n";
    for(int bit=0;bit<=1;bit++){
        PE enc=encrypt_bit(cc,kp,bit);
        double r=ratio_of(cc,kp,enc);
        double exp=(bit==1)?(1.0+PSI):PSI;
        std::cout<<"  "<<bit<<"   | "<<std::fixed<<std::setprecision(10)<<r
             <<" | "<<exp<<" | "<<std::scientific<<std::setprecision(1)<<std::abs(r-exp)<<"\n";
    }
    
    std::cout<<"\n  === NAND via Encrypted Ratios ===\n";
    std::cout<<"  A B | Decoded     | Expected | Error\n";
    std::cout<<"  ---------------------------------------\n";
    for(int a=0;a<=1;a++){
        for(int b=0;b<=1;b++){
            PE enc_A=encrypt_bit(cc,kp,a), enc_B=encrypt_bit(cc,kp,b);
            double rA=ratio_of(cc,kp,enc_A), rB=ratio_of(cc,kp,enc_B);
            double rN=nand_ratio(rA,rB), dec=rN-PSI;
            double exp=1.0-a*b;
            std::cout<<"  "<<a<<" "<<b<<" | "<<std::fixed<<std::setprecision(10)<<dec
                 <<" | "<<exp<<"      | "<<std::scientific<<std::setprecision(1)<<std::abs(dec-exp)<<"\n";
        }
    }
    
    std::cout<<"\n  === Full Adder SUM via Encrypted Ratios ===\n";
    std::cout<<"  A B Cin | SUM decoded | Expected | Error\n";
    std::cout<<"  ---------------------------------------------\n";
    int cases[8][3]={{0,0,0},{0,0,1},{0,1,0},{0,1,1},{1,0,0},{1,0,1},{1,1,0},{1,1,1}};
    for(int i=0;i<8;i++){
        int A=cases[i][0], B=cases[i][1], Cin=cases[i][2];
        int exp=(A+B+Cin)%2;
        PE enc_A=encrypt_bit(cc,kp,A), enc_B=encrypt_bit(cc,kp,B), enc_C=encrypt_bit(cc,kp,Cin);
        double rA=ratio_of(cc,kp,enc_A), rB=ratio_of(cc,kp,enc_B), rCin=ratio_of(cc,kp,enc_C);
        double rAB=nand_ratio(rA,rB);
        double rA_AB=nand_ratio(rA,rAB), rB_AB=nand_ratio(rB,rAB);
        double rSUM=nand_ratio(rA_AB,rB_AB), dec=rSUM-PSI;
        std::cout<<"  "<<A<<" "<<B<<" "<<Cin<<"  | "<<std::fixed<<std::setprecision(10)<<dec
             <<" | "<<exp<<"      | "<<std::scientific<<std::setprecision(1)<<std::abs(dec-exp)<<"\n";
    }
    std::cout<<"\n";
    return 0;
}
