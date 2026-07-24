// BRIDGE LEVEL 2-3: True nested tensor product, compare real damping
#include <iostream>
#include <iomanip>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <sys/time.h>
#include "openfhe.h"
using namespace lbcrypto;

struct PE { Ciphertext<DCRTPoly> a; Ciphertext<DCRTPoly> b; };
PE mulY_2d(CryptoContext<DCRTPoly>& cc, const PE& x) { return {x.b, cc->EvalAdd(x.a, x.b)}; }
PE mulY_inv_2d(CryptoContext<DCRTPoly>& cc, const PE& x) { return {cc->EvalSub(x.b, x.a), x.a}; }

double decrypt_val(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, const Ciphertext<DCRTPoly>& c) {
    Plaintext pt; cc->Decrypt(kp.secretKey, c, &pt); return pt->GetCKKSPackedValue()[0].real();
}
PE encrypt_bit(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, double v) {
    return {cc->Encrypt(kp.publicKey,cc->MakeCKKSPackedPlaintext(std::vector<double>{v})),
            cc->Encrypt(kp.publicKey,cc->MakeCKKSPackedPlaintext(std::vector<double>{1.0}))};
}
PE nand_2d(CryptoContext<DCRTPoly>& cc, const PE& A, const PE& B) {
    auto aabb=cc->EvalMult(A.b,B.b); auto aaba=cc->EvalMult(A.a,B.a);
    return {cc->EvalSub(aabb,aaba), aabb};
}
int decode_bit(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, const PE& s){
    double a=decrypt_val(cc,kp,s.a), b=decrypt_val(cc,kp,s.b);
    return (a>b*0.5)?1:0;
}
double time_ms(struct timeval s,struct timeval e){return (e.tv_sec-s.tv_sec)*1000.0+(e.tv_usec-s.tv_usec)/1000.0;}

// True nested spiral: apply mulY/mulY_inv at each level
PE spiral_l1(CryptoContext<DCRTPoly>& cc, const PE& x) { PE s=x; s=mulY_2d(cc,s); s=mulY_inv_2d(cc,s); return s; }
PE spiral_l2(CryptoContext<DCRTPoly>& cc, const PE& x) { return spiral_l1(cc,spiral_l1(cc,x)); }
PE spiral_l3(CryptoContext<DCRTPoly>& cc, const PE& x) { return spiral_l1(cc,spiral_l2(cc,x)); }

int main(){
    std::cout<<"\n  BRIDGE LEVEL 2-3: True nested tensor product damping\n\n";
    
    CCParams<CryptoContextCKKSRNS> p;
    p.SetMultiplicativeDepth(500);p.SetScalingModSize(50);p.SetBatchSize(2048);
    p.SetRingDim(16384);p.SetSecretKeyDist(UNIFORM_TERNARY);p.SetSecurityLevel(HEStd_NotSet);
    auto cc=GenCryptoContext(p);
    cc->Enable(PKE);cc->Enable(KEYSWITCH);cc->Enable(LEVELEDSHE);cc->Enable(ADVANCEDSHE);
    auto kp=cc->KeyGen();cc->EvalMultKeyGen(kp.secretKey);
    srand(time(0));
    
    // Test on a "noisy" state (value=0.5, should converge to 0 or 1)
    PE noisy=encrypt_bit(cc,kp,0.5);
    double a0=decrypt_val(cc,kp,noisy.a), b0=decrypt_val(cc,kp,noisy.b);
    std::cout<<"  Initial: a="<<std::fixed<<std::setprecision(4)<<a0<<" b="<<b0<<" ratio="<<a0/b0<<"\n\n";
    
    // Apply Level-1 spiral multiple times
    std::cout<<"  Level-1 spiral (φ/ψ only):\n";
    PE s1=noisy;
    for(int i=1;i<=10;i++){
        s1=spiral_l1(cc,s1);
        double a=decrypt_val(cc,kp,s1.a), b=decrypt_val(cc,kp,s1.b);
        std::cout<<"    x"<<i<<": ratio="<<std::fixed<<std::setprecision(6)<<a/b;
        if(i%3==0)std::cout<<"\n";
    }
    
    // Apply Level-2 spiral multiple times
    std::cout<<"\n\n  Level-2 spiral (φ²/ψ²):\n";
    PE s2=noisy;
    for(int i=1;i<=10;i++){
        s2=spiral_l2(cc,s2);
        double a=decrypt_val(cc,kp,s2.a), b=decrypt_val(cc,kp,s2.b);
        std::cout<<"    x"<<i<<": ratio="<<std::fixed<<std::setprecision(6)<<a/b;
        if(i%3==0)std::cout<<"\n";
    }
    
    // Apply Level-3 spiral multiple times
    std::cout<<"\n\n  Level-3 spiral (φ³/ψ³):\n";
    PE s3=noisy;
    for(int i=1;i<=10;i++){
        s3=spiral_l3(cc,s3);
        double a=decrypt_val(cc,kp,s3.a), b=decrypt_val(cc,kp,s3.b);
        std::cout<<"    x"<<i<<": ratio="<<std::fixed<<std::setprecision(6)<<a/b;
        if(i%3==0)std::cout<<"\n";
    }
    
    // Compare convergence to 0 or 1 (ψ or 1+ψ)
    std::cout<<"\n\n  Convergence comparison (target: ψ≈0.618 or 1+ψ≈1.618):\n";
    std::cout<<"  Level  | Final ratio | Drift from 0.618 | Drift from 1.618\n";
    std::cout<<"  ---------------------------------------------------------\n";
    for(int L=1;L<=3;L++){
        PE s=noisy;
        for(int i=0;i<10;i++){
            if(L==1)s=spiral_l1(cc,s);
            else if(L==2)s=spiral_l2(cc,s);
            else s=spiral_l3(cc,s);
        }
        double r=decrypt_val(cc,kp,s.a)/decrypt_val(cc,kp,s.b);
        std::cout<<"  L"<<L<<"      | "<<std::fixed<<std::setprecision(6)<<r
             <<"     | "<<std::scientific<<std::abs(r-0.618034)
             <<"         | "<<std::scientific<<std::abs(r-1.618034)<<"\n";
    }
    
    std::cout<<"\n  BRIDGE: Level-2 and Level-3 damping comparison complete\n\n";
    return 0;
}
