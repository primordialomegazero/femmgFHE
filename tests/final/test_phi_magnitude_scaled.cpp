// MAGNITUDE DECODE ON SCALED INPUTS: The final Fractal iO test
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
long long fib(int n){if(n<=1)return n;long long a=0,b=1;for(int i=2;i<=n;i++){long long t=a+b;a=b;b=t;}return b;}

int main(){
    std::cout<<"\n  MAGNITUDE DECODE ON SCALED INPUTS: Fractal iO Final Test\n";
    std::cout<<"  If midpoint can be computed blind, Fractal iO is COMPLETE\n\n";
    
    CCParams<CryptoContextCKKSRNS> p;
    p.SetMultiplicativeDepth(500);p.SetScalingModSize(50);p.SetBatchSize(2048);
    p.SetRingDim(4096);p.SetSecretKeyDist(UNIFORM_TERNARY);p.SetSecurityLevel(HEStd_NotSet);
    auto cc=GenCryptoContext(p);
    cc->Enable(PKE);cc->Enable(KEYSWITCH);cc->Enable(LEVELEDSHE);cc->Enable(ADVANCEDSHE);
    auto kp=cc->KeyGen();cc->EvalMultKeyGen(kp.secretKey);
    srand(time(0));
    
    int steps=8, ok_fixed=0, ok_scaled=0, total=40;
    
    // Fixed midpoint for unscaled inputs (known: 57.5)
    double fixed_mid=57.5;
    
    std::cout<<"  Bit | Scale | b_value  | FixedMid("<<fixed_mid<<") | ScaledMid | Match\n";
    std::cout<<"  -------------------------------------------------------------\n";
    
    for(int t=0;t<total;t++){
        int bit=rand()%2; double v=(bit==1)?1.0:0.0;
        double scale=0.3+(rand()%400)/100.0; // random scale
        
        PE scaled={
            cc->Encrypt(kp.publicKey,cc->MakeCKKSPackedPlaintext(std::vector<double>{scale*(v+PSI)})),
            cc->Encrypt(kp.publicKey,cc->MakeCKKSPackedPlaintext(std::vector<double>{scale*1.0}))
        };
        
        for(int i=0;i<steps;i++) scaled=mulY(cc,scaled);
        double b_val=decrypt_val(cc,kp,scaled.b);
        
        // Fixed midpoint decode
        int dec_fixed=(b_val>fixed_mid)?1:0;
        if(dec_fixed==bit)ok_fixed++;
        
        // Scaled midpoint: scale * 57.5 (need to know scale!)
        // But we can compute scale from initial b: b_init = scale*1.0
        // Actually: we need the initial b to compute scale
        // For blind: midpoint = scale * 57.5 = b_init * 57.5
        // But b_init is encrypted... 
        // Alternative: use the ratio b/a instead of absolute b?
        
        // Let's compute the ideal scaled midpoint
        double scaled_mid=scale*fixed_mid;
        int dec_scaled=(b_val>scaled_mid)?1:0;
        if(dec_scaled==bit)ok_scaled++;
        
        if(t<12)std::cout<<"  "<<bit<<"   | "
             <<std::fixed<<std::setprecision(2)<<scale<<" | "
             <<std::fixed<<std::setprecision(4)<<b_val<<"    | "
             <<dec_fixed<<"                 | "
             <<dec_scaled<<"        | "
             <<(dec_scaled==bit?"YES":"NO")<<"\n";
    }
    
    std::cout<<"\n  Fixed Midpoint ("<<fixed_mid<<"): "<<ok_fixed<<"/"<<total;
    std::cout<<"  (requires knowing scale for correct threshold)\n";
    std::cout<<"  Scaled Midpoint (scale*"<<fixed_mid<<"): "<<ok_scaled<<"/"<<total;
    std::cout<<"  (ideal, but needs scale from b_init)\n\n";
    
    std::cout<<"  KEY INSIGHT: b_init = scale. So scaled_mid = b_init * "<<fixed_mid<<".\n";
    std::cout<<"  b_init is encrypted. We need homomorphic comparison:\n";
    std::cout<<"  b_final > b_init * "<<fixed_mid<<" ?\n";
    std::cout<<"  This is doable with homomorphic operations!\n\n";
    
    return 0;
}
