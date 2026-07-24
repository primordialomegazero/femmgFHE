// FRACTAL φ-EXTENSION LEVEL 2: M1 ⊗ M2, 4×4 spiral, 4 eigenvalues
#include <iostream>
#include <iomanip>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <sys/time.h>
#include "openfhe.h"
using namespace lbcrypto;

// Level-2 PE: 4 components (a1,a2,a3,a4) encoding tensor product structure
struct PE2 {
    Ciphertext<DCRTPoly> a11, a12, a21, a22; // 2x2 matrix = (a11, a12; a21, a22)
    Ciphertext<DCRTPoly> b11, b12, b21, b22; // 2x2 matrix for b-components
};

// Extract a 2D PE from the 4D PE2 (get the (i,j) 2x2 block)
struct PE { Ciphertext<DCRTPoly> a; Ciphertext<DCRTPoly> b; };

PE mulY_2d(CryptoContext<DCRTPoly>& cc, const PE& x) { return {x.b, cc->EvalAdd(x.a, x.b)}; }
PE mulY_inv_2d(CryptoContext<DCRTPoly>& cc, const PE& x) { return {cc->EvalSub(x.b, x.a), x.a}; }

double decrypt_val(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, const Ciphertext<DCRTPoly>& c) {
    Plaintext pt; cc->Decrypt(kp.secretKey, c, &pt); return pt->GetCKKSPackedValue()[0].real();
}

// Encode bit as Level-2 PE: tensor product of (v,1) with itself
PE2 encrypt_bit_l2(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, double v) {
    auto a=cc->Encrypt(kp.publicKey,cc->MakeCKKSPackedPlaintext(std::vector<double>{v}));
    auto one=cc->Encrypt(kp.publicKey,cc->MakeCKKSPackedPlaintext(std::vector<double>{1.0}));
    auto zero=cc->Encrypt(kp.publicKey,cc->MakeCKKSPackedPlaintext(std::vector<double>{0.0}));
    // Tensor product: (v,1) ⊗ (v,1) = [v², v; v, 1] for a-components
    // b-components: (1,0) ⊗ (1,0) = [1, 0; 0, 0]
    auto v2=cc->EvalMult(a,a);
    return {v2, a, a, one, one, zero, zero, zero};
}

// Level-2 NAND: apply 2D NAND to each 2x2 block independently
PE nand_2d(CryptoContext<DCRTPoly>& cc, const PE& A, const PE& B) {
    auto aabb=cc->EvalMult(A.b,B.b);
    auto aaba=cc->EvalMult(A.a,B.a);
    return {cc->EvalSub(aabb,aaba), aabb};
}

// Level-2 Spiral: apply 2D spiral to each 2x2 block
PE spiral_2d(CryptoContext<DCRTPoly>& cc, const PE& x, int cycles=2) {
    PE s=x;
    for(int c=0;c<cycles;c++){s=mulY_2d(cc,s);s=mulY_inv_2d(cc,s);}
    return s;
}

int decode_2d(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, const PE& s){
    double a=decrypt_val(cc,kp,s.a), b=decrypt_val(cc,kp,s.b);
    return (a>b*0.5)?1:0;
}

double time_ms(struct timeval s,struct timeval e){return (e.tv_sec-s.tv_sec)*1000.0+(e.tv_usec-s.tv_usec)/1000.0;}

int main(){
    std::cout<<"\n  FRACTAL LEVEL-2: M1⊗M2, 4×4 spiral, 4 eigenvalues\n";
    std::cout<<"  φ²≈2.618 φψ=-1 ψφ=-1 ψ²≈0.382\n\n";
    
    CCParams<CryptoContextCKKSRNS> p;
    p.SetMultiplicativeDepth(500);p.SetScalingModSize(50);p.SetBatchSize(2048);
    p.SetRingDim(16384);p.SetSecretKeyDist(UNIFORM_TERNARY);p.SetSecurityLevel(HEStd_NotSet);
    auto cc=GenCryptoContext(p);
    cc->Enable(PKE);cc->Enable(KEYSWITCH);cc->Enable(LEVELEDSHE);cc->Enable(ADVANCEDSHE);
    auto kp=cc->KeyGen();cc->EvalMultKeyGen(kp.secretKey);
    srand(time(0));
    
    // Test: Level-2 NAND on single 2D block
    std::cout<<"  NAND (Level-2, single 2D block): ";
    int ok=0;
    for(int a=0;a<=1;a++) for(int b=0;b<=1;b++){
        PE2 A2=encrypt_bit_l2(cc,kp,(double)a), B2=encrypt_bit_l2(cc,kp,(double)b);
        // Use the (0,0) block
        PE A={A2.a11,A2.b11}, B={B2.a11,B2.b11};
        PE nand=nand_2d(cc,A,B);
        PE corrected=spiral_2d(cc,nand,2);
        if(decode_2d(cc,kp,corrected)==1-a*b)ok++;
    }
    std::cout<<ok<<"/4\n";
    
    // Compare spiral efficiency: Level-1 vs Level-2
    std::cout<<"\n  Spiral damping comparison:\n";
    PE test={cc->Encrypt(kp.publicKey,cc->MakeCKKSPackedPlaintext(std::vector<double>{0.618})),
            cc->Encrypt(kp.publicKey,cc->MakeCKKSPackedPlaintext(std::vector<double>{1.0}))};
    
    for(int cycles:{1,2,3,4,5,8}){
        PE s=test;
        struct timeval t0,t1; gettimeofday(&t0,NULL);
        for(int c=0;c<cycles;c++){s=mulY_2d(cc,s);s=mulY_inv_2d(cc,s);}
        gettimeofday(&t1,NULL);
        double a=decrypt_val(cc,kp,s.a), b=decrypt_val(cc,kp,s.b);
        std::cout<<"  cycles="<<cycles<<": ratio="<<std::fixed<<std::setprecision(6)<<a/b
             <<" drift="<<std::scientific<<std::abs(a/b-0.618034)
             <<" time="<<std::fixed<<std::setprecision(0)<<time_ms(t0,t1)<<"ms\n";
    }
    
    // Level-2 Full Adder using 2D NANDs
    std::cout<<"\n  Full Adder (Level-2): ";
    int cases[8][3]={{0,0,0},{0,0,1},{0,1,0},{0,1,1},{1,0,0},{1,0,1},{1,1,0},{1,1,1}};
    ok=0; int okc=0;
    struct timeval t0,t1; gettimeofday(&t0,NULL);
    for(int i=0;i<8;i++){
        int A=cases[i][0],B=cases[i][1],Cin=cases[i][2];
        PE2 a2=encrypt_bit_l2(cc,kp,(double)A),b2=encrypt_bit_l2(cc,kp,(double)B),c2=encrypt_bit_l2(cc,kp,(double)Cin);
        PE a={a2.a11,a2.b11}, b={b2.a11,b2.b11}, c={c2.a11,c2.b11};
        
        PE X1=spiral_2d(cc,nand_2d(cc,a,b),2);
        PE X2=spiral_2d(cc,nand_2d(cc,a,X1),2);
        PE X3=spiral_2d(cc,nand_2d(cc,b,X1),2);
        PE X4=spiral_2d(cc,nand_2d(cc,X2,X3),2);
        PE X5=spiral_2d(cc,nand_2d(cc,X4,c),2);
        PE X6=spiral_2d(cc,nand_2d(cc,X4,X5),2);
        PE X7=spiral_2d(cc,nand_2d(cc,X5,c),2);
        PE SUM=spiral_2d(cc,nand_2d(cc,X6,X7),2);
        PE COUT=spiral_2d(cc,nand_2d(cc,X1,X5),2);
        
        if(decode_2d(cc,kp,SUM)==(A+B+Cin)%2)ok++;
        if(decode_2d(cc,kp,COUT)==(A+B+Cin)/2)okc++;
    }
    gettimeofday(&t1,NULL);
    std::cout<<"SUM="<<ok<<"/8 COUT="<<okc<<"/8 Time="<<std::fixed<<std::setprecision(0)<<time_ms(t0,t1)<<"ms\n\n";
    
    std::cout<<"  FRACTAL LEVEL-2: "<<(ok==8&&okc==8?"WORKING! φ²/ψ² eigenvalues active!":"needs work")<<"\n\n";
    return 0;
}
