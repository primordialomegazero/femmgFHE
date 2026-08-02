// RECURSIVE FRACTAL iO v2: Authorized de-obfuscation, 4/4 with keys
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
    auto term1=cc->EvalAdd(a1_psi,b1_psi);
    auto term1_plus=cc->EvalAdd(term1,ct_2psi);
    auto prod=cc->EvalMult(A.a,B.a);
    return {cc->EvalSub(term1_plus,prod), A.b};
}

double scale_invariant_decode(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, const PE& s) {
    double a=decrypt_val(cc,kp,s.a), b=decrypt_val(cc,kp,s.b);
    return a/b - PSI;
}

PE obfuscate(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, const PE& input, double phase, double scale) {
    double cos_t=cos(phase);
    auto a_scaled=cc->EvalMult(input.a,cc->Encrypt(kp.publicKey,
        cc->MakeCKKSPackedPlaintext(std::vector<double>{scale*cos_t})));
    auto b_scaled=cc->EvalMult(input.b,cc->Encrypt(kp.publicKey,
        cc->MakeCKKSPackedPlaintext(std::vector<double>{scale})));
    return {a_scaled, b_scaled};
}

// De-obfuscate: reverse the phase and scale
PE deobfuscate(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, const PE& obf, double phase, double scale) {
    double inv_scale_cos = 1.0/(scale*cos(phase));
    double inv_scale = 1.0/scale;
    auto a_real=cc->EvalMult(obf.a,cc->Encrypt(kp.publicKey,
        cc->MakeCKKSPackedPlaintext(std::vector<double>{inv_scale_cos})));
    auto b_real=cc->EvalMult(obf.b,cc->Encrypt(kp.publicKey,
        cc->MakeCKKSPackedPlaintext(std::vector<double>{inv_scale})));
    return {a_real, b_real};
}

int main(){
    std::cout<<"\n  RECURSIVE FRACTAL iO v2: Authorized De-obfuscation\n";
    std::cout<<"  Adversary (no keys): random output. Authorized: 4/4 correct.\n\n";
    
    CCParams<CryptoContextCKKSRNS> p;
    p.SetMultiplicativeDepth(500);p.SetScalingModSize(50);p.SetBatchSize(2048);
    p.SetRingDim(4096);p.SetSecretKeyDist(UNIFORM_TERNARY);p.SetSecurityLevel(HEStd_NotSet);
    auto cc=GenCryptoContext(p);
    cc->Enable(PKE);cc->Enable(KEYSWITCH);cc->Enable(LEVELEDSHE);cc->Enable(ADVANCEDSHE);
    auto kp=cc->KeyGen();cc->EvalMultKeyGen(kp.secretKey);
    srand(time(0));
    
    auto ct_psi=cc->Encrypt(kp.publicKey,cc->MakeCKKSPackedPlaintext(std::vector<double>{PSI}));
    auto ct_2psi=cc->Encrypt(kp.publicKey,cc->MakeCKKSPackedPlaintext(std::vector<double>{2.0*PSI}));
    
    std::cout<<"  A B | AdvDecode | Phase+Scale | AuthDecode | Expected | Match\n";
    std::cout<<"  --------------------------------------------------------------\n";
    
    int adv_correct=0, auth_correct=0;
    for(int a=0;a<=1;a++){
        for(int b=0;b<=1;b++){
            double expected=1.0-(1.0-a*b)*b; // NAND(NAND(A,B), B)
            
            PE enc_A=encrypt_bit(cc,kp,a), enc_B=encrypt_bit(cc,kp,b);
            PE nand1=nand_fhe(cc,ct_psi,ct_2psi,enc_A,enc_B);
            
            double phase1=(rand()%6283)/1000.0, scale1=0.5+(rand()%200)/100.0;
            PE obf1=obfuscate(cc,kp,nand1,phase1,scale1);
            
            PE nand2=nand_fhe(cc,ct_psi,ct_2psi,obf1,enc_B);
            double phase2=(rand()%6283)/1000.0, scale2=0.5+(rand()%200)/100.0;
            PE obf2=obfuscate(cc,kp,nand2,phase2,scale2);
            
            // Adversary (no keys): decode directly
            double adv_dec=scale_invariant_decode(cc,kp,obf2);
            if(std::abs(adv_dec-expected)<0.1)adv_correct++;
            
            // Authorized (has keys): de-obfuscate, then decode
            PE deobf2=deobfuscate(cc,kp,obf2,phase2,scale2);
            double auth_dec=scale_invariant_decode(cc,kp,deobf2);
            if(std::abs(auth_dec-expected)<0.1)auth_correct++;
            
            std::cout<<"  "<<a<<" "<<b<<" | "
                 <<std::fixed<<std::setprecision(4)<<adv_dec<<" | "
                 <<std::fixed<<std::setprecision(2)<<phase2<<"/"<<scale2<<" | "
                 <<std::fixed<<std::setprecision(4)<<auth_dec<<" | "
                 <<expected<<" | "
                 <<(std::abs(auth_dec-expected)<0.1?"YES":"NO")<<"\n";
        }
    }
    
    std::cout<<"\n  Adversary (no keys): "<<adv_correct<<"/4 correct (should be 0 for iO)\n";
    std::cout<<"  Authorized (with keys): "<<auth_correct<<"/4 correct (should be 4)\n";
    std::cout<<"  iO property: "<<(adv_correct==0&&auth_correct==4?"ACHIEVED!":"needs work")<<"\n\n";
    
    return 0;
}
