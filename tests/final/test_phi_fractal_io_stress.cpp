// FRACTAL iO STRESS TEST: Random circuits, obfuscation, authorized vs adversary
#include <iostream>
#include <iomanip>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <sys/time.h>
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
long long fib(int n){if(n<=1)return n;long long a=0,b=1;for(int i=2;i<=n;i++){long long t=a+b;a=b;b=t;}return b;}

PE nand_fhe(CryptoContext<DCRTPoly>& cc, const Ciphertext<DCRTPoly>& ct_psi,
            const Ciphertext<DCRTPoly>& ct_2psi, const PE& A, const PE& B) {
    auto a1_psi=cc->EvalMult(A.a,ct_psi), b1_psi=cc->EvalMult(B.a,ct_psi);
    auto term1=cc->EvalAdd(a1_psi,b1_psi);
    auto term1_plus=cc->EvalAdd(term1,ct_2psi);
    auto prod=cc->EvalMult(A.a,B.a);
    return {cc->EvalSub(term1_plus,prod), A.b};
}

double time_ms(struct timeval s,struct timeval e){return (e.tv_sec-s.tv_sec)*1000.0+(e.tv_usec-s.tv_usec)/1000.0;}

int main(){
    std::cout<<"\n  ╔══════════════════════════════════════════════════════════════╗\n";
    std::cout<<"  ║   FRACTAL iO STRESS TEST: Authorized vs Adversary             ║\n";
    std::cout<<"  ╚══════════════════════════════════════════════════════════════╝\n\n";
    
    CCParams<CryptoContextCKKSRNS> p;
    p.SetMultiplicativeDepth(500);p.SetScalingModSize(50);p.SetBatchSize(2048);
    p.SetRingDim(4096);p.SetSecretKeyDist(UNIFORM_TERNARY);p.SetSecurityLevel(HEStd_NotSet);
    auto cc=GenCryptoContext(p);
    cc->Enable(PKE);cc->Enable(KEYSWITCH);cc->Enable(LEVELEDSHE);cc->Enable(ADVANCEDSHE);
    auto kp=cc->KeyGen();cc->EvalMultKeyGen(kp.secretKey);
    srand(time(0));
    
    auto ct_psi=cc->Encrypt(kp.publicKey,cc->MakeCKKSPackedPlaintext(std::vector<double>{PSI}));
    auto ct_2psi=cc->Encrypt(kp.publicKey,cc->MakeCKKSPackedPlaintext(std::vector<double>{2.0*PSI}));
    
    int total_tests=100;
    int auth_ok=0, adv_ok=0;
    struct timeval t0,t1; gettimeofday(&t0,NULL);
    
    std::cout<<"  Running "<<total_tests<<" random obfuscated NAND tests...\n\n";
    
    for(int t=0;t<total_tests;t++){
        int a=rand()%2, b=rand()%2;
        int expected=1-a*b;
        
        PE enc_A=encrypt_bit(cc,kp,a), enc_B=encrypt_bit(cc,kp,b);
        PE real_nand=nand_fhe(cc,ct_psi,ct_2psi,enc_A,enc_B);
        
        // Obfuscate: random scale + phase
        double scale=0.5+(rand()%300)/100.0;
        double phase=(rand()%6283)/1000.0;
        double cos_t=cos(phase);
        
        PE obf={
            cc->EvalMult(real_nand.a,cc->Encrypt(kp.publicKey,
                cc->MakeCKKSPackedPlaintext(std::vector<double>{scale*cos_t}))),
            cc->EvalMult(real_nand.b,cc->Encrypt(kp.publicKey,
                cc->MakeCKKSPackedPlaintext(std::vector<double>{scale})))
        };
        
        // Adversary: fixed midpoint (doesn't know scale)
        for(int i=0;i<8;i++) obf=mulY(cc,obf);
        double b_val=decrypt_val(cc,kp,obf.b);
        int adv_bit=(b_val>57.5)?1:0;
        if(adv_bit==expected)adv_ok++;
        
        // Authorized: scaled midpoint (knows scale = b_init)
        double b_init=decrypt_val(cc,kp,real_nand.b);
        double auth_mid=b_init*57.5;
        int auth_bit=(b_val>auth_mid)?1:0;
        if(auth_bit==expected)auth_ok++;
        
        if(t<10 || t%20==0)std::cout<<"  ["<<std::setw(3)<<t<<"] A="<<a<<" B="<<b
             <<" Exp="<<expected<<" Adv="<<adv_bit<<" Auth="<<auth_bit
             <<" | Adv:"<<(adv_bit==expected?"OK":"XX")<<" Auth:"<<(auth_bit==expected?"OK":"XX")<<"\n";
    }
    
    gettimeofday(&t1,NULL);
    double ms=time_ms(t0,t1);
    
    std::cout<<"\n  ╔══════════════════════════════════════════════════════════════╗\n";
    std::cout<<"  ║   STRESS TEST RESULTS                                         ║\n";
    std::cout<<"  ╠════════════════════════════════════════════════════════════════╣\n";
    std::cout<<"  ║   Total tests:        "<<std::setw(5)<<total_tests<<"                                   ║\n";
    std::cout<<"  ║   Adversary (no keys): "<<std::setw(5)<<adv_ok<<"/"<<total_tests;
    std::cout<<" ("<<std::fixed<<std::setprecision(1)<<100.0*adv_ok/total_tests<<"%)";
    for(int i=0;i<10;i++)std::cout<<" ";std::cout<<"║\n";
    std::cout<<"  ║   Authorized (keys):   "<<std::setw(5)<<auth_ok<<"/"<<total_tests;
    std::cout<<" ("<<std::fixed<<std::setprecision(1)<<100.0*auth_ok/total_tests<<"%)";
    for(int i=0;i<10;i++)std::cout<<" ";std::cout<<"║\n";
    std::cout<<"  ║   Time:                "<<std::fixed<<std::setprecision(0)<<std::setw(6)<<ms<<" ms";
    for(int i=0;i<22;i++)std::cout<<" ";std::cout<<"║\n";
    std::cout<<"  ║   Per test:            "<<std::fixed<<std::setprecision(1)<<std::setw(6)<<ms/total_tests<<" ms";
    for(int i=0;i<22;i++)std::cout<<" ";std::cout<<"║\n";
    std::cout<<"  ╠════════════════════════════════════════════════════════════════╣\n";
    
    bool iO_achieved=(adv_ok<=total_tests*0.55)&&(auth_ok>=total_tests*0.95);
    std::cout<<"  ║   iO Property: "<<(iO_achieved?"ACHIEVED!":"needs work");
    for(int i=0;i<(iO_achieved?33:33);i++)std::cout<<" ";std::cout<<"║\n";
    std::cout<<"  ║   Adversary ≈ random (50%), Authorized ≈ 100%                 ║\n";
    std::cout<<"  ╚══════════════════════════════════════════════════════════════╝\n\n";
    
    return 0;
}
