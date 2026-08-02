// GOLDEN RATIO HARMONIZED SPIRAL: φ-weighted forward/reverse ratio
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

// φ-HARMONIZED SPIRAL: F(fwd) forward steps, F(fwd-1) reverse steps
// This creates net convergence toward ψ!
PE phi_spiral(CryptoContext<DCRTPoly>& cc, const PE& x, int fwd=3) {
    PE s=x;
    for(int i=0;i<fwd;i++) s=mulY_2d(cc,s);       // F(fwd) forward = expand
    for(int i=0;i<fwd-1;i++) s=mulY_inv_2d(cc,s); // F(fwd-1) reverse = contract less
    // Net: 1 extra forward step → ratio moves toward ψ!
    return s;
}

int main(){
    std::cout<<"\n  GOLDEN RATIO HARMONIZED SPIRAL: φ-weighted forward/reverse\n\n";
    
    CCParams<CryptoContextCKKSRNS> p;
    p.SetMultiplicativeDepth(500);p.SetScalingModSize(50);p.SetBatchSize(2048);
    p.SetRingDim(16384);p.SetSecretKeyDist(UNIFORM_TERNARY);p.SetSecurityLevel(HEStd_NotSet);
    auto cc=GenCryptoContext(p);
    cc->Enable(PKE);cc->Enable(KEYSWITCH);cc->Enable(LEVELEDSHE);cc->Enable(ADVANCEDSHE);
    auto kp=cc->KeyGen();cc->EvalMultKeyGen(kp.secretKey);
    srand(time(0));
    
    // Test convergence on noisy state (0.5)
    PE noisy=encrypt_bit(cc,kp,0.5);
    std::cout<<"  Noisy state (0.5) harmonized spiral:\n";
    PE s=noisy;
    for(int i=1;i<=10;i++){
        s=phi_spiral(cc,s,3); // 3 fwd, 2 rev
        double a=decrypt_val(cc,kp,s.a), b=decrypt_val(cc,kp,s.b);
        std::cout<<"    x"<<i<<": ratio="<<std::fixed<<std::setprecision(6)<<a/b;
        if(std::abs(a/b-0.618034)<0.001)std::cout<<" ≈ψ!";
        std::cout<<"\n";
    }
    
    // Test on bit states (should stay at 0 or 1)
    std::cout<<"\n  Harmonized spiral on clean bits:\n";
    for(int bit=0;bit<=1;bit++){
        PE clean=encrypt_bit(cc,kp,(double)bit);
        PE s2=phi_spiral(cc,clean,3);
        double a=decrypt_val(cc,kp,s2.a), b=decrypt_val(cc,kp,s2.b);
        std::cout<<"  Bit "<<bit<<": ratio="<<std::fixed<<std::setprecision(6)<<a/b
             <<" decoded="<<decode_bit(cc,kp,s2)<<" (expected "<<bit<<")\n";
    }
    
    // Full Adder with harmonized spiral
    std::cout<<"\n  Full Adder (harmonized spiral): ";
    int cases[8][3]={{0,0,0},{0,0,1},{0,1,0},{0,1,1},{1,0,0},{1,0,1},{1,1,0},{1,1,1}};
    int ok=0,okc=0;
    struct timeval t0,t1; gettimeofday(&t0,NULL);
    for(int i=0;i<8;i++){
        int A=cases[i][0],B=cases[i][1],Cin=cases[i][2];
        PE a=encrypt_bit(cc,kp,(double)A),b=encrypt_bit(cc,kp,(double)B),c=encrypt_bit(cc,kp,(double)Cin);
        PE X1=phi_spiral(cc,nand_2d(cc,a,b),3);
        PE X2=phi_spiral(cc,nand_2d(cc,a,X1),3);
        PE X3=phi_spiral(cc,nand_2d(cc,b,X1),3);
        PE X4=phi_spiral(cc,nand_2d(cc,X2,X3),3);
        PE X5=phi_spiral(cc,nand_2d(cc,X4,c),3);
        PE X6=phi_spiral(cc,nand_2d(cc,X4,X5),3);
        PE X7=phi_spiral(cc,nand_2d(cc,X5,c),3);
        PE SUM=phi_spiral(cc,nand_2d(cc,X6,X7),3);
        PE COUT=phi_spiral(cc,nand_2d(cc,X1,X5),3);
        if(decode_bit(cc,kp,SUM)==(A+B+Cin)%2)ok++;
        if(decode_bit(cc,kp,COUT)==(A+B+Cin)/2)okc++;
    }
    gettimeofday(&t1,NULL);
    std::cout<<"SUM="<<ok<<"/8 COUT="<<okc<<"/8 Time="<<std::fixed<<std::setprecision(0)<<time_ms(t0,t1)<<"ms\n\n";
    
    std::cout<<"  HARMONIZED SPIRAL: "<<(ok==8&&okc==8?"8/8! φ-weighting works!":"needs work")<<"\n\n";
    return 0;
}
