// iO CALIBRATION: Find correct MID for NAND outputs
#include <iostream>
#include <iomanip>
#include <cmath>
#include "openfhe.h"
using namespace lbcrypto;

struct PE { Ciphertext<DCRTPoly> a; Ciphertext<DCRTPoly> b; };
PE mulY(CryptoContext<DCRTPoly>& cc, const PE& x) { return {x.b, cc->EvalAdd(x.a, x.b)}; }

double decrypt_val(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, const Ciphertext<DCRTPoly>& c) {
    Plaintext pt; cc->Decrypt(kp.secretKey, c, &pt); return pt->GetCKKSPackedValue()[0].real();
}
PE encrypt_bit(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, int bit) {
    const double PSI=0.6180339887498949;
    double v=(bit==1)?1.0:0.0;
    return {cc->Encrypt(kp.publicKey,cc->MakeCKKSPackedPlaintext(std::vector<double>{v+PSI})),
            cc->Encrypt(kp.publicKey,cc->MakeCKKSPackedPlaintext(std::vector<double>{1.0}))};
}
const double PSI=0.6180339887498949;

PE nand_fhe(CryptoContext<DCRTPoly>& cc, const Ciphertext<DCRTPoly>& ct_psi,
            const Ciphertext<DCRTPoly>& ct_2psi, const PE& A, const PE& B) {
    auto a1_psi=cc->EvalMult(A.a,ct_psi), b1_psi=cc->EvalMult(B.a,ct_psi);
    auto term1=cc->EvalAdd(a1_psi,b1_psi), term1_plus=cc->EvalAdd(term1,ct_2psi);
    auto prod=cc->EvalMult(A.a,B.a);
    return {cc->EvalSub(term1_plus,prod), A.b};
}

int main(){
    std::cout<<"\n  iO CALIBRATION: Find MID for NAND outputs\n\n";
    
    CCParams<CryptoContextCKKSRNS> p;
    p.SetMultiplicativeDepth(500);p.SetScalingModSize(50);p.SetBatchSize(2048);
    p.SetRingDim(4096);p.SetSecretKeyDist(UNIFORM_TERNARY);p.SetSecurityLevel(HEStd_NotSet);
    auto cc=GenCryptoContext(p);
    cc->Enable(PKE);cc->Enable(KEYSWITCH);cc->Enable(LEVELEDSHE);cc->Enable(ADVANCEDSHE);
    auto kp=cc->KeyGen();cc->EvalMultKeyGen(kp.secretKey);
    
    auto ct_psi=cc->Encrypt(kp.publicKey,cc->MakeCKKSPackedPlaintext(std::vector<double>{PSI}));
    auto ct_2psi=cc->Encrypt(kp.publicKey,cc->MakeCKKSPackedPlaintext(std::vector<double>{2.0*PSI}));
    
    std::cout<<"  NAND Output -> 8x mulY -> b_final values:\n\n";
    std::cout<<"  A B | a_nand  | b_nand  | b_final (8mulY) | Expected bit\n";
    std::cout<<"  -----------------------------------------------------\n";
    
    double b_vals[4];
    int cases[4][2]={{0,0},{0,1},{1,0},{1,1}};
    
    for(int i=0;i<4;i++){
        int a=cases[i][0], b=cases[i][1];
        PE enc_A=encrypt_bit(cc,kp,a), enc_B=encrypt_bit(cc,kp,b);
        PE nand=nand_fhe(cc,ct_psi,ct_2psi,enc_A,enc_B);
        
        double a_val=decrypt_val(cc,kp,nand.a);
        double b_val=decrypt_val(cc,kp,nand.b);
        
        PE state=nand;
        for(int j=0;j<8;j++) state=mulY(cc,state);
        double b_final=decrypt_val(cc,kp,state.b);
        b_vals[i]=b_final;
        
        std::cout<<"  "<<a<<" "<<b<<" | "<<std::fixed<<std::setprecision(4)<<a_val
             <<" | "<<std::fixed<<std::setprecision(4)<<b_val
             <<" | "<<std::fixed<<std::setprecision(4)<<b_final
             <<"           | "<<1-a*b<<"\n";
    }
    
    // Find optimal MID
    std::sort(b_vals,b_vals+4);
    double mid=(b_vals[1]+b_vals[2])/2.0; // between the two middle values
    std::cout<<"\n  Optimal MID = ("<<std::fixed<<std::setprecision(4)<<b_vals[1]
             <<" + "<<b_vals[2]<<")/2 = "<<mid<<"\n\n";
    
    // Test with this MID
    std::cout<<"  Testing with MID="<<mid<<":\n";
    std::cout<<"  A B | b_final | b_init*MID | diff | bit | expected | match\n";
    std::cout<<"  ---------------------------------------------------------\n";
    
    int ok=0;
    for(int i=0;i<4;i++){
        int a=cases[i][0], b=cases[i][1], expected=1-a*b;
        PE enc_A=encrypt_bit(cc,kp,a), enc_B=encrypt_bit(cc,kp,b);
        PE nand=nand_fhe(cc,ct_psi,ct_2psi,enc_A,enc_B);
        double b_init=decrypt_val(cc,kp,nand.b);
        PE state=nand;
        for(int j=0;j<8;j++) state=mulY(cc,state);
        double b_final=decrypt_val(cc,kp,state.b);
        double diff=b_final-b_init*mid;
        int bit=(diff>0)?1:0;
        if(bit==expected)ok++;
        std::cout<<"  "<<a<<" "<<b<<" | "<<std::fixed<<std::setprecision(1)<<b_final
             <<" | "<<std::fixed<<std::setprecision(1)<<b_init*mid
             <<" | "<<std::fixed<<std::setprecision(1)<<diff
             <<" | "<<bit<<" | "<<expected<<" | "<<(bit==expected?"YES":"NO")<<"\n";
    }
    
    std::cout<<"\n  Calibrated MID: "<<mid<<" -> "<<ok<<"/4 correct\n\n";
    return 0;
}
