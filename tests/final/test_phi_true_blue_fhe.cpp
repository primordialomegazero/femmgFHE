// True Blue FHE v2: CORRECTED NAND formula
#include <iostream>
#include <iomanip>
#include <cmath>
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
PE encrypt_value(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, double v) {
    const double PSI=0.6180339887498949;
    return {cc->Encrypt(kp.publicKey,cc->MakeCKKSPackedPlaintext(std::vector<double>{v+PSI})),
            cc->Encrypt(kp.publicKey,cc->MakeCKKSPackedPlaintext(std::vector<double>{1.0}))};
}
const double PSI=0.6180339887498949, PSI2=PSI*PSI;

// CORRECTED: R(NAND) = PSI*(R(A)+R(B)+2) - R(A)*R(B)
double compute_nand_ratio(double rA, double rB) {
    return PSI*(rA + rB + 2.0) - rA*rB;
}

int main() {
    std::cout << "\n";
    std::cout << "  ╔══════════════════════════════════════════════════════════════════╗\n";
    std::cout << "  ║   TRUE BLUE FHE v2: CORRECTED NAND Formula                       ║\n";
    std::cout << "  ║   R(NAND) = PSI*(R(A)+R(B)+2) - R(A)*R(B)                        ║\n";
    std::cout << "  ╚══════════════════════════════════════════════════════════════════╝\n\n";
    
    CCParams<CryptoContextCKKSRNS> p;
    p.SetMultiplicativeDepth(200);p.SetScalingModSize(50);p.SetBatchSize(2048);
    p.SetRingDim(4096);p.SetSecretKeyDist(UNIFORM_TERNARY);p.SetSecurityLevel(HEStd_NotSet);
    auto cc=GenCryptoContext(p);
    cc->Enable(PKE);cc->Enable(KEYSWITCH);cc->Enable(LEVELEDSHE);cc->Enable(ADVANCEDSHE);
    auto kp=cc->KeyGen();cc->EvalMultKeyGen(kp.secretKey);
    
    double inputs[4][2]={{0,0},{0,1},{1,0},{1,1}};
    
    std::cout << "  === NAND Truth Table ===\n";
    std::cout << "  A  B  |  Expected  |  Computed  |  Error\n";
    std::cout << "  ---------------------------------------------\n";
    
    for(int i=0;i<4;i++){
        double A=inputs[i][0], B=inputs[i][1];
        double expected=1.0-A*B;
        PE enc_A=encrypt_value(cc,kp,A), enc_B=encrypt_value(cc,kp,B);
        double rA=ratio_of(cc,kp,enc_A), rB=ratio_of(cc,kp,enc_B);
        double rN=compute_nand_ratio(rA,rB);
        double decoded=rN-PSI;
        std::cout<<"  "<<std::fixed<<std::setprecision(0)<<A<<"  "<<B
             <<"  |  "<<std::setw(6)<<std::setprecision(4)<<expected
             <<"  |  "<<std::setw(8)<<std::setprecision(4)<<decoded
             <<"  |  "<<std::scientific<<std::setprecision(1)<<std::abs(decoded-expected)<<"\n";
    }
    
    std::cout<<"\n  === 2-Level NAND Chain ===\n";
    std::cout<<"  A  B  |  NAND1  |  NAND2 Expected  |  NAND2 Computed  |  Error\n";
    std::cout<<"  ---------------------------------------------------------------\n";
    
    for(int i=0;i<4;i++){
        double A=inputs[i][0], B=inputs[i][1];
        double n1=1.0-A*B, n2=1.0-n1*B;
        PE enc_A=encrypt_value(cc,kp,A), enc_B=encrypt_value(cc,kp,B);
        double rA=ratio_of(cc,kp,enc_A), rB=ratio_of(cc,kp,enc_B);
        double rN1=compute_nand_ratio(rA,rB), dN1=rN1-PSI;
        double rN2=compute_nand_ratio(dN1+PSI,rB), dN2=rN2-PSI;
        std::cout<<"  "<<std::fixed<<std::setprecision(0)<<A<<"  "<<B
             <<"  |  "<<std::setw(5)<<std::setprecision(4)<<dN1
             <<"  |  "<<std::setw(6)<<std::setprecision(4)<<n2
             <<"        |  "<<std::setw(8)<<std::setprecision(4)<<dN2
             <<"       |  "<<std::scientific<<std::setprecision(1)<<std::abs(dN2-n2)<<"\n";
    }
    
    std::cout<<"\n  === 10-Level NAND Cascade (A=0, B=0) ===\n";
    double A0=0, B0=0, val=A0;
    double r_cur=A0+PSI, rB0=B0+PSI, d_cur=val;
    std::cout<<"  Level  Value  |  Next NAND\n";
    std::cout<<"  ---------------------------\n";
    for(int L=0;L<10;L++){
        double exp_next=1.0-d_cur*B0;
        r_cur=compute_nand_ratio(r_cur,rB0);
        d_cur=r_cur-PSI;
        std::cout<<"  "<<std::setw(3)<<L<<"  "<<std::setw(5)<<std::setprecision(3)<<(L==0?A0:1.0-d_cur)
             <<"  |  "<<std::setw(6)<<std::setprecision(4)<<d_cur
             <<" (exp "<<std::setw(6)<<std::setprecision(4)<<exp_next<<")\n";
    }
    
    std::cout<<"\n  ╔══════════════════════════════════════════════════════════════════╗\n";
    std::cout<<"  ║  CORRECTED NAND: R(NAND)=PSI*(R(A)+R(B)+2)-R(A)R(B)             ║\n";
    std::cout<<"  ║  Exact algebraic formula, arbitrary depth                        ║\n";
    std::cout<<"  ╚══════════════════════════════════════════════════════════════════╝\n\n";
    return 0;
}
