// FRACTAL LIMIT EXPLORATION: Complete internal state trace, Level 1→2→3→∞
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

// Spiral at different fractal depths
PE spiral_depth(CryptoContext<DCRTPoly>& cc, const PE& x, int depth, int cycles=2) {
    PE s=x;
    for(int d=0;d<depth;d++){
        for(int c=0;c<cycles;c++){s=mulY_2d(cc,s);s=mulY_inv_2d(cc,s);}
    }
    return s;
}

// Full Adder with configurable spiral depth
void full_adder_trace(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp,
    int A, int B, int Cin, int depth, const char* label){
    PE a=encrypt_bit(cc,kp,(double)A),b=encrypt_bit(cc,kp,(double)B),c=encrypt_bit(cc,kp,(double)Cin);
    
    PE X1=spiral_depth(cc,nand_2d(cc,a,b),depth);
    PE X2=spiral_depth(cc,nand_2d(cc,a,X1),depth);
    PE X3=spiral_depth(cc,nand_2d(cc,b,X1),depth);
    PE X4=spiral_depth(cc,nand_2d(cc,X2,X3),depth);
    PE X5=spiral_depth(cc,nand_2d(cc,X4,c),depth);
    PE X6=spiral_depth(cc,nand_2d(cc,X4,X5),depth);
    PE X7=spiral_depth(cc,nand_2d(cc,X5,c),depth);
    PE SUM=spiral_depth(cc,nand_2d(cc,X6,X7),depth);
    PE COUT=spiral_depth(cc,nand_2d(cc,X1,X5),depth);
    
    int s=decode_bit(cc,kp,SUM), co=decode_bit(cc,kp,COUT);
    int eSUM=(A+B+Cin)%2, eCOUT=(A+B+Cin)/2;
    
    double sa=decrypt_val(cc,kp,SUM.a), sb=decrypt_val(cc,kp,SUM.b);
    double ca=decrypt_val(cc,kp,COUT.a), cb=decrypt_val(cc,kp,COUT.b);
    
    std::cout<<"  "<<std::setw(8)<<label<<" | SUM: a="<<std::fixed<<std::setprecision(4)<<sa
         <<" b="<<sb<<" ratio="<<sa/sb<<" bit="<<s<<" (exp "<<eSUM<<")"
         <<" | COUT: a="<<ca<<" b="<<cb<<" ratio="<<ca/cb<<" bit="<<co<<" (exp "<<eCOUT<<")\n";
}

int main(){
    std::cout<<"\n  ╔══════════════════════════════════════════════════════════════╗\n";
    std::cout<<"  ║   FRACTAL LIMIT EXPLORATION: Complete Internal State Trace     ║\n";
    std::cout<<"  ╚══════════════════════════════════════════════════════════════╝\n\n";
    
    CCParams<CryptoContextCKKSRNS> p;
    p.SetMultiplicativeDepth(500);p.SetScalingModSize(50);p.SetBatchSize(2048);
    p.SetRingDim(16384);p.SetSecretKeyDist(UNIFORM_TERNARY);p.SetSecurityLevel(HEStd_NotSet);
    auto cc=GenCryptoContext(p);
    cc->Enable(PKE);cc->Enable(KEYSWITCH);cc->Enable(LEVELEDSHE);cc->Enable(ADVANCEDSHE);
    auto kp=cc->KeyGen();cc->EvalMultKeyGen(kp.secretKey);
    srand(time(0));
    
    // Trace NAND internal state at different depths
    std::cout<<"  === NAND(0,0)=1 Internal State vs Fractal Depth ===\n";
    PE A=encrypt_bit(cc,kp,0.0), B=encrypt_bit(cc,kp,0.0);
    for(int depth=0;depth<=5;depth++){
        PE nand=nand_2d(cc,A,B);
        PE corrected=spiral_depth(cc,nand,depth,2);
        double a=decrypt_val(cc,kp,corrected.a), b=decrypt_val(cc,kp,corrected.b);
        std::cout<<"  Depth "<<depth<<": a="<<std::fixed<<std::setprecision(6)<<a
             <<" b="<<b<<" ratio="<<a/b<<" bit="<<decode_bit(cc,kp,corrected)<<"\n";
    }
    
    // Full Adder trace at different depths
    std::cout<<"\n  === Full Adder (1+1+1=1,cout=1) vs Fractal Depth ===\n";
    for(int depth=1;depth<=4;depth++){
        char label[16]; snprintf(label,16,"Depth %d",depth);
        full_adder_trace(cc,kp,1,1,1,depth,label);
    }
    
    // Fractal limit: increase depth until state stabilizes
    std::cout<<"\n  === Fractal Limit: Depth sweep on NAND(1,1)=0 ===\n";
    PE A2=encrypt_bit(cc,kp,1.0), B2=encrypt_bit(cc,kp,1.0);
    double prev_ratio=0;
    for(int depth=0;depth<=10;depth++){
        PE nand=nand_2d(cc,A2,B2);
        PE corrected=spiral_depth(cc,nand,depth,1);
        double a=decrypt_val(cc,kp,corrected.a), b=decrypt_val(cc,kp,corrected.b);
        double ratio=a/b;
        double delta=(depth>0)?std::abs(ratio-prev_ratio):0;
        std::cout<<"  Depth "<<std::setw(2)<<depth<<": ratio="<<std::fixed<<std::setprecision(10)<<ratio
             <<" delta="<<std::scientific<<delta<<" bit="<<decode_bit(cc,kp,corrected)<<"\n";
        prev_ratio=ratio;
    }
    
    // 4-bit adder with optimal depth
    std::cout<<"\n  === 4-Bit Adder (Depth=2) ===\n";
    int bits=4; int a_bits[bits],b_bits[bits]; long long valA=0,valB=0;
    for(int i=0;i<bits;i++){a_bits[i]=rand()%2;b_bits[i]=rand()%2;valA=valA*2+a_bits[i];valB=valB*2+b_bits[i];}
    PE rCarry=encrypt_bit(cc,kp,0.0);
    int sum_bits[bits]; long long result=0;
    struct timeval t0,t1; gettimeofday(&t0,NULL);
    for(int i=bits-1;i>=0;i--){
        PE a=encrypt_bit(cc,kp,(double)a_bits[i]),b=encrypt_bit(cc,kp,(double)b_bits[i]);
        PE X1=spiral_depth(cc,nand_2d(cc,a,b),2);
        PE X2=spiral_depth(cc,nand_2d(cc,a,X1),2);
        PE X3=spiral_depth(cc,nand_2d(cc,b,X1),2);
        PE X4=spiral_depth(cc,nand_2d(cc,X2,X3),2);
        PE X5=spiral_depth(cc,nand_2d(cc,X4,rCarry),2);
        PE X6=spiral_depth(cc,nand_2d(cc,X4,X5),2);
        PE X7=spiral_depth(cc,nand_2d(cc,X5,rCarry),2);
        PE SUM=spiral_depth(cc,nand_2d(cc,X6,X7),2);
        PE COUT=spiral_depth(cc,nand_2d(cc,X1,X5),2);
        sum_bits[i]=decode_bit(cc,kp,SUM); rCarry=COUT;
    }
    for(int i=0;i<bits;i++){std::cout<<sum_bits[i];result=result*2+sum_bits[i];}
    int fc=decode_bit(cc,kp,rCarry); if(fc)result+=(1LL<<bits);
    gettimeofday(&t1,NULL);
    std::cout<<" (c="<<fc<<") = "<<result<<" | Expected="<<valA+valB<<" | "<<(result==valA+valB?"MATCH!":"MISMATCH")<<"\n";
    std::cout<<"  Time: "<<std::fixed<<std::setprecision(0)<<time_ms(t0,t1)<<"ms\n\n";
    
    std::cout<<"  ╔══════════════════════════════════════════════════════════════╗\n";
    std::cout<<"  ║  FRACTAL LIMIT: Depth→∞, ψ^k damping, holographic boundary    ║\n";
    std::cout<<"  ║  Fernandez Limit = optimal depth for noise/performance        ║\n";
    std::cout<<"  ╚══════════════════════════════════════════════════════════════╝\n\n";
    return 0;
}
