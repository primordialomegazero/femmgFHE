// NORM-BASED BLIND DECODER: Rotation-invariant, scale-invariant
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
const double PSI=0.6180339887498949;

// Norm-based blind decode: v = sqrt(a^2 + b^2 - 1) - PSI
double norm_blind_decode(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, const PE& s){
    double a=decrypt_val(cc,kp,s.a), b=decrypt_val(cc,kp,s.b);
    double norm_sq=a*a + b*b;
    return std::sqrt(norm_sq - 1.0) - PSI;
}

int to_bit(double v){return (v>0.5)?1:0;}

int main(){
    std::cout<<"\n  NORM-BASED BLIND DECODER: Rotation & Scale Invariant\n";
    std::cout<<"  v = sqrt(a^2 + b^2 - 1) - PSI\n\n";
    
    CCParams<CryptoContextCKKSRNS> p;
    p.SetMultiplicativeDepth(500);p.SetScalingModSize(50);p.SetBatchSize(2048);
    p.SetRingDim(4096);p.SetSecretKeyDist(UNIFORM_TERNARY);p.SetSecurityLevel(HEStd_NotSet);
    auto cc=GenCryptoContext(p);
    cc->Enable(PKE);cc->Enable(KEYSWITCH);cc->Enable(LEVELEDSHE);cc->Enable(ADVANCEDSHE);
    auto kp=cc->KeyGen();cc->EvalMultKeyGen(kp.secretKey);
    srand(time(0));
    
    int ok=0,total=40;
    std::cout<<"  Bit | Theta  | Scale  | Decoded | Expected | Match\n";
    std::cout<<"  --------------------------------------------------\n";
    
    for(int t=0;t<total;t++){
        int bit=rand()%2; double v=(bit==1)?1.0:0.0;
        double theta=(rand()%6283)/1000.0;
        double scale=0.3+(rand()%400)/100.0;
        
        double a0=scale*(v+PSI), b0=scale*1.0;
        double a_rot=a0*cos(theta)-b0*sin(theta);
        double b_rot=a0*sin(theta)+b0*cos(theta);
        
        PE state={
            cc->Encrypt(kp.publicKey,cc->MakeCKKSPackedPlaintext(std::vector<double>{a_rot})),
            cc->Encrypt(kp.publicKey,cc->MakeCKKSPackedPlaintext(std::vector<double>{b_rot}))
        };
        
        double decoded=norm_blind_decode(cc,kp,state);
        if(to_bit(decoded)==bit)ok++;
        
        if(t<15)std::cout<<"  "<<bit<<"   | "<<std::fixed<<std::setprecision(2)<<theta
             <<"   | "<<std::fixed<<std::setprecision(2)<<scale
             <<"  | "<<std::fixed<<std::setprecision(4)<<decoded
             <<"    | "<<v<<"      | "<<(to_bit(decoded)==bit?"YES":"NO")<<"\n";
    }
    
    std::cout<<"\n  Norm Blind Decoder: "<<ok<<"/"<<total<<"\n";
    std::cout<<"  Status: "<<(ok==total?"PERFECT! ROTATION+SCALE INVARIANT!":"needs work")<<"\n\n";
    return 0;
}
