// iO DEBUG: Print all intermediate values
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
const double PSI=0.6180339887498949, MID=57.5;

PE nand_fhe(CryptoContext<DCRTPoly>& cc, const Ciphertext<DCRTPoly>& ct_psi,
            const Ciphertext<DCRTPoly>& ct_2psi, const PE& A, const PE& B) {
    auto a1_psi=cc->EvalMult(A.a,ct_psi), b1_psi=cc->EvalMult(B.a,ct_psi);
    auto term1=cc->EvalAdd(a1_psi,b1_psi), term1_plus=cc->EvalAdd(term1,ct_2psi);
    auto prod=cc->EvalMult(A.a,B.a);
    return {cc->EvalSub(term1_plus,prod), A.b};
}

int main(){
    std::cout<<"\n  iO DEBUG: Trace all values through the pipeline\n\n";
    
    CCParams<CryptoContextCKKSRNS> p;
    p.SetMultiplicativeDepth(500);p.SetScalingModSize(50);p.SetBatchSize(2048);
    p.SetRingDim(4096);p.SetSecretKeyDist(UNIFORM_TERNARY);p.SetSecurityLevel(HEStd_NotSet);
    auto cc=GenCryptoContext(p);
    cc->Enable(PKE);cc->Enable(KEYSWITCH);cc->Enable(LEVELEDSHE);cc->Enable(ADVANCEDSHE);
    auto kp=cc->KeyGen();cc->EvalMultKeyGen(kp.secretKey);
    
    auto ct_psi=cc->Encrypt(kp.publicKey,cc->MakeCKKSPackedPlaintext(std::vector<double>{PSI}));
    auto ct_2psi=cc->Encrypt(kp.publicKey,cc->MakeCKKSPackedPlaintext(std::vector<double>{2.0*PSI}));
    auto ct_mid=cc->Encrypt(kp.publicKey,cc->MakeCKKSPackedPlaintext(std::vector<double>{MID}));
    
    int cases[4][2]={{0,0},{0,1},{1,0},{1,1}};
    
    for(int i=0;i<4;i++){
        int a=cases[i][0], b=cases[i][1], expected=1-a*b;
        std::cout<<"  === NAND("<<a<<","<<b<<") expected="<<expected<<" ===\n";
        
        PE enc_A=encrypt_bit(cc,kp,a), enc_B=encrypt_bit(cc,kp,b);
        PE real_nand=nand_fhe(cc,ct_psi,ct_2psi,enc_A,enc_B);
        
        double b_init_val=decrypt_val(cc,kp,real_nand.b);
        double a_nand_val=decrypt_val(cc,kp,real_nand.a);
        std::cout<<"  After NAND: a="<<std::fixed<<std::setprecision(4)<<a_nand_val
             <<" b="<<b_init_val<<" ratio="<<a_nand_val/b_init_val-PSI<<"\n";
        
        // Obfuscate
        double scale=1.5; // fixed for debug
        PE obf={
            cc->EvalMult(real_nand.a,cc->Encrypt(kp.publicKey,
                cc->MakeCKKSPackedPlaintext(std::vector<double>{scale}))),
            cc->EvalMult(real_nand.b,cc->Encrypt(kp.publicKey,
                cc->MakeCKKSPackedPlaintext(std::vector<double>{scale})))
        };
        double b_obf_val=decrypt_val(cc,kp,obf.b);
        std::cout<<"  After obfuscate (scale="<<scale<<"): b="<<b_obf_val<<"\n";
        
        // mulY x8 on obfuscated
        for(int j=0;j<8;j++) obf=mulY(cc,obf);
        double b_final=decrypt_val(cc,kp,obf.b);
        std::cout<<"  After 8 mulY: b_final="<<b_final<<"\n";
        
        // Threshold
        double b_init_mid=b_init_val*MID;
        double diff=b_final-b_init_mid;
        int bit=(diff>0)?1:0;
        std::cout<<"  b_init*MID="<<b_init_mid<<" diff="<<diff<<" bit="<<bit
             <<" (expected "<<expected<<") "<<(bit==expected?"OK":"XX")<<"\n\n";
    }
    
    return 0;
}
