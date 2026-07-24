// FRACTAL INTERFEROMETER: 2-path recursive self-correction
#include <iostream>
#include <iomanip>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include "openfhe.h"
using namespace lbcrypto;

struct PE { Ciphertext<DCRTPoly> a; Ciphertext<DCRTPoly> b; };
PE mulY(CryptoContext<DCRTPoly>& cc, const PE& x) { return {x.b, cc->EvalAdd(x.a, x.b)}; }
PE mulY_inv(CryptoContext<DCRTPoly>& cc, const PE& x) { return {cc->EvalSub(x.b, x.a), x.a}; }

double decrypt_val(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, const Ciphertext<DCRTPoly>& c) {
    Plaintext pt; cc->Decrypt(kp.secretKey, c, &pt); return pt->GetCKKSPackedValue()[0].real();
}
const double PSI=0.6180339887498949;

int to_bit(double v){return (v>0.5)?1:0;}

int main(){
    std::cout<<"\n  FRACTAL INTERFEROMETER: 2-Path Recursive Self-Correction\n";
    std::cout<<"  Path A: Forward (φ-direction, expands)\n";
    std::cout<<"  Path B: Reverse (ψ-direction, contracts)\n";
    std::cout<<"  Interference: A + B cancels noise, preserves signal\n\n";
    
    CCParams<CryptoContextCKKSRNS> p;
    p.SetMultiplicativeDepth(500);p.SetScalingModSize(50);p.SetBatchSize(2048);
    p.SetRingDim(4096);p.SetSecretKeyDist(UNIFORM_TERNARY);p.SetSecurityLevel(HEStd_NotSet);
    auto cc=GenCryptoContext(p);
    cc->Enable(PKE);cc->Enable(KEYSWITCH);cc->Enable(LEVELEDSHE);cc->Enable(ADVANCEDSHE);
    auto kp=cc->KeyGen();cc->EvalMultKeyGen(kp.secretKey);
    srand(time(0));
    
    int ok=0,total=30;
    std::cout<<"  Bit | PathA(mulY) | PathB(mulY_inv) | Interference | Expected | Match\n";
    std::cout<<"  --------------------------------------------------------------------\n";
    
    for(int t=0;t<total;t++){
        int bit=rand()%2; double v=(bit==1)?1.0:0.0;
        double theta=(rand()%6283)/1000.0;
        double a0=v+PSI, b0=1.0;
        double a_rot=a0*cos(theta)-b0*sin(theta);
        double b_rot=a0*sin(theta)+b0*cos(theta);
        
        PE state={
            cc->Encrypt(kp.publicKey,cc->MakeCKKSPackedPlaintext(std::vector<double>{a_rot})),
            cc->Encrypt(kp.publicKey,cc->MakeCKKSPackedPlaintext(std::vector<double>{b_rot}))
        };
        
        // Path A: forward 4 steps
        PE pathA=state; for(int i=0;i<4;i++) pathA=mulY(cc,pathA);
        double aA=decrypt_val(cc,kp,pathA.a), bA=decrypt_val(cc,kp,pathA.b);
        double decA=aA/bA - PSI;
        
        // Path B: reverse 4 steps
        PE pathB=state; for(int i=0;i<4;i++) pathB=mulY_inv(cc,pathB);
        double aB=decrypt_val(cc,kp,pathB.a), bB=decrypt_val(cc,kp,pathB.b);
        double decB=aB/bB - PSI;
        
        // Interference: average the two paths (noise cancels!)
        double interference=(aA+aB)/(bA+bB) - PSI;
        
        if(to_bit(interference)==bit)ok++;
        
        if(t<10)std::cout<<"  "<<bit<<"   | "<<std::fixed<<std::setprecision(4)<<decA
             <<"       | "<<std::fixed<<std::setprecision(4)<<decB
             <<"          | "<<std::fixed<<std::setprecision(4)<<interference
             <<"         | "<<v<<"      | "<<(to_bit(interference)==bit?"YES":"NO")<<"\n";
    }
    
    std::cout<<"\n  Interferometer Recovery: "<<ok<<"/"<<total<<"\n";
    std::cout<<"  Status: "<<(ok==total?"SELF-CORRECTING! NOISE CANCELS!":"needs work")<<"\n\n";
    return 0;
}
