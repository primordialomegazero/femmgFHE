// ROTATION ATTRACTOR: Fibonacci convergence from any initial angle
#include <iostream>
#include <iomanip>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include "openfhe.h"
using namespace lbcrypto;

struct PE { Ciphertext<DCRTPoly> a; Ciphertext<DCRTPoly> b; };
PE mulY(CryptoContext<DCRTPoly>& cc, const PE& x) { return {x.b, cc->EvalAdd(x.a, x.b)}; }

double decrypt_val(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, const Ciphertext<DCRTPoly>& c) {
    Plaintext pt; cc->Decrypt(kp.secretKey, c, &pt); return pt->GetCKKSPackedValue()[0].real();
}
const double PSI=0.6180339887498949;

int main(){
    std::cout<<"\n  ROTATION ATTRACTOR: Fibonacci drives ratio to PSI from any angle\n";
    std::cout<<"  If attractor is rotation-invariant: iO without keys is possible!\n\n";
    
    CCParams<CryptoContextCKKSRNS> p;
    p.SetMultiplicativeDepth(500);p.SetScalingModSize(50);p.SetBatchSize(2048);
    p.SetRingDim(4096);p.SetSecretKeyDist(UNIFORM_TERNARY);p.SetSecurityLevel(HEStd_NotSet);
    auto cc=GenCryptoContext(p);
    cc->Enable(PKE);cc->Enable(KEYSWITCH);cc->Enable(LEVELEDSHE);cc->Enable(ADVANCEDSHE);
    auto kp=cc->KeyGen();cc->EvalMultKeyGen(kp.secretKey);
    srand(time(0));
    
    std::cout<<"  Bit | Theta | a_init  | b_init  | After 8 mulY: a/b - PSI | Match\n";
    std::cout<<"  ------------------------------------------------------------------\n";
    
    int ok=0,total=20;
    for(int t=0;t<total;t++){
        int bit=rand()%2; double v=(bit==1)?1.0:0.0;
        double theta=(rand()%6283)/1000.0; // random 0 to 2pi
        
        // Original encoding: (v+PSI, 1)
        double a0=v+PSI, b0=1.0;
        // Rotate by theta
        double a_rot = a0*cos(theta) - b0*sin(theta);
        double b_rot = a0*sin(theta) + b0*cos(theta);
        
        PE state={
            cc->Encrypt(kp.publicKey,cc->MakeCKKSPackedPlaintext(std::vector<double>{a_rot})),
            cc->Encrypt(kp.publicKey,cc->MakeCKKSPackedPlaintext(std::vector<double>{b_rot}))
        };
        
        // Apply 8 mulY steps
        for(int i=0;i<8;i++) state=mulY(cc,state);
        
        double a_final=decrypt_val(cc,kp,state.a);
        double b_final=decrypt_val(cc,kp,state.b);
        double decoded=a_final/b_final - PSI;
        
        int recovered=(decoded>0.25)?1:0;
        if(recovered==bit)ok++;
        
        if(t<12)std::cout<<"  "<<bit<<"   | "<<std::fixed<<std::setprecision(2)<<theta
             <<" | "<<std::fixed<<std::setprecision(4)<<a_rot
             <<" | "<<std::fixed<<std::setprecision(4)<<b_rot
             <<" | "<<std::fixed<<std::setprecision(4)<<decoded
             <<"                  | "<<(recovered==bit?"YES":"NO")<<"\n";
    }
    
    std::cout<<"\n  Rotation-Invariant Recovery: "<<ok<<"/"<<total<<"\n";
    std::cout<<"  Status: "<<(ok==total?"ATTRACTOR IS ROTATION-INVARIANT! iO WITHOUT KEYS VIABLE!":"needs work")<<"\n\n";
    return 0;
}
