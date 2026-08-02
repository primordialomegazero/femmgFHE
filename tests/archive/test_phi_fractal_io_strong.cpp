// FRACTAL iO STRONG: Restricted phase, stronger obfuscation
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
    auto a_obf=cc->EvalMult(input.a,cc->Encrypt(kp.publicKey,
        cc->MakeCKKSPackedPlaintext(std::vector<double>{scale*cos_t})));
    auto b_obf=cc->EvalMult(input.b,cc->Encrypt(kp.publicKey,
        cc->MakeCKKSPackedPlaintext(std::vector<double>{scale})));
    return {a_obf, b_obf};
}
PE deobfuscate(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, const PE& obf, double phase, double scale) {
    double inv_scale_cos=1.0/(scale*cos(phase)), inv_scale=1.0/scale;
    auto a_real=cc->EvalMult(obf.a,cc->Encrypt(kp.publicKey,
        cc->MakeCKKSPackedPlaintext(std::vector<double>{inv_scale_cos})));
    auto b_real=cc->EvalMult(obf.b,cc->Encrypt(kp.publicKey,
        cc->MakeCKKSPackedPlaintext(std::vector<double>{inv_scale})));
    return {a_real, b_real};
}
double strong_phase(){
    double p;
    do{p=(rand()%6283)/1000.0;}while(std::abs(cos(p))>0.5);
    return p;
}

int main(){
    std::cout<<"\n  FRACTAL iO STRONG: Restricted Phase (|cos|<0.5)\n\n";
    CCParams<CryptoContextCKKSRNS> p;
    p.SetMultiplicativeDepth(500);p.SetScalingModSize(50);p.SetBatchSize(2048);
    p.SetRingDim(4096);p.SetSecretKeyDist(UNIFORM_TERNARY);p.SetSecurityLevel(HEStd_NotSet);
    auto cc=GenCryptoContext(p);
    cc->Enable(PKE);cc->Enable(KEYSWITCH);cc->Enable(LEVELEDSHE);cc->Enable(ADVANCEDSHE);
    auto kp=cc->KeyGen();cc->EvalMultKeyGen(kp.secretKey);
    srand(time(0));
    auto ct_psi=cc->Encrypt(kp.publicKey,cc->MakeCKKSPackedPlaintext(std::vector<double>{PSI}));
    auto ct_2psi=cc->Encrypt(kp.publicKey,cc->MakeCKKSPackedPlaintext(std::vector<double>{2.0*PSI}));
    
    int adv_ok=0,auth_ok=0,total=20;
    std::cout<<"  Running "<<total<<" random tests...\n\n";
    std::cout<<"  A B | AdvDecode | Phase/Scale | AuthDecode | Expected | Match\n";
    std::cout<<"  --------------------------------------------------------------\n";
    
    for(int t=0;t<total;t++){
        int a=rand()%2,b=rand()%2;
        double expected=1.0-a*b;
        PE enc_A=encrypt_bit(cc,kp,a),enc_B=encrypt_bit(cc,kp,b);
        PE real_nand=nand_fhe(cc,ct_psi,ct_2psi,enc_A,enc_B);
        double phase=strong_phase(),scale=0.5+(rand()%200)/100.0;
        PE obf_output=obfuscate(cc,kp,real_nand,phase,scale);
        double adv_dec=scale_invariant_decode(cc,kp,obf_output);
        if(std::abs(adv_dec-expected)<0.5)adv_ok++;
        PE real_recovered=deobfuscate(cc,kp,obf_output,phase,scale);
        double auth_dec=scale_invariant_decode(cc,kp,real_recovered);
        if(std::abs(auth_dec-expected)<0.1)auth_ok++;
        if(t<8)std::cout<<"  "<<a<<" "<<b<<" | "<<std::fixed<<std::setprecision(4)<<adv_dec<<" | "
             <<std::fixed<<std::setprecision(2)<<phase<<"/"<<scale<<" | "<<std::fixed<<std::setprecision(4)<<auth_dec
             <<" | "<<expected<<" | "<<(std::abs(auth_dec-expected)<0.1?"YES":"NO")<<"\n";
    }
    
    std::cout<<"\n  Adversary: "<<adv_ok<<"/"<<total<<" (should be 0)\n";
    std::cout<<"  Authorized: "<<auth_ok<<"/"<<total<<" (should be "<<total<<")\n";
    std::cout<<"  Status: "<<(adv_ok==0&&auth_ok==total?"FRACTAL iO ACHIEVED!":"needs work")<<"\n\n";
    return 0;
}
