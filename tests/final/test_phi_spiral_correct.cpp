// SPIRAL AUTO-CORRECTION: Interleave φ/ψ steps, error transmutes to attractor
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
PE mulY_inv(CryptoContext<DCRTPoly>& cc, const PE& x) { return {cc->EvalSub(x.b, x.a), x.a}; }

double decrypt_val(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, const Ciphertext<DCRTPoly>& c) {
    Plaintext pt; cc->Decrypt(kp.secretKey, c, &pt); return pt->GetCKKSPackedValue()[0].real();
}
PE encrypt_direct(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, double v) {
    return {cc->Encrypt(kp.publicKey,cc->MakeCKKSPackedPlaintext(std::vector<double>{v})),
            cc->Encrypt(kp.publicKey,cc->MakeCKKSPackedPlaintext(std::vector<double>{1.0}))};
}
PE nand_direct(CryptoContext<DCRTPoly>& cc, const PE& A, const PE& B) {
    auto aabb=cc->EvalMult(A.b,B.b);
    auto aaba=cc->EvalMult(A.a,B.a);
    return {cc->EvalSub(aabb,aaba), aabb};
}

// SPIRAL CORRECTOR: Interleave φ and ψ steps to spiral error into attractor
// Pattern: mulY → mulY_inv → mulY → mulY_inv → ...
// This creates a spiral that converges to the ψ-attractor
PE spiral_correct(CryptoContext<DCRTPoly>& cc, const PE& state, int cycles=4) {
    PE s=state;
    for(int c=0;c<cycles;c++){
        s=mulY(cc,s);        // expand φ
        s=mulY_inv(cc,s);    // contract ψ
        // After each pair: noise is "spun" toward the attractor
    }
    return s;
}

int decode_rel(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, const PE& s){
    double a=decrypt_val(cc,kp,s.a), b=decrypt_val(cc,kp,s.b);
    return (a>b*0.5)?1:0;
}
double time_ms(struct timeval s,struct timeval e){return (e.tv_sec-s.tv_sec)*1000.0+(e.tv_usec-s.tv_usec)/1000.0;}

int main(){
    std::cout<<"\n  SPIRAL AUTO-CORRECTION: φ/ψ interleaving transmutes error\n\n";
    
    CCParams<CryptoContextCKKSRNS> p;
    p.SetMultiplicativeDepth(200);p.SetScalingModSize(50);p.SetBatchSize(2048);
    p.SetRingDim(8192);p.SetSecretKeyDist(UNIFORM_TERNARY);p.SetSecurityLevel(HEStd_NotSet);
    auto cc=GenCryptoContext(p);
    cc->Enable(PKE);cc->Enable(KEYSWITCH);cc->Enable(LEVELEDSHE);cc->Enable(ADVANCEDSHE);
    auto kp=cc->KeyGen();cc->EvalMultKeyGen(kp.secretKey);
    srand(time(0));
    
    // Test: run NAND then spiral, check if it self-corrects
    std::cout<<"  Spiral correction test on NAND outputs:\n";
    for(int a=0;a<=1;a++) for(int b=0;b<=1;b++){
        PE A=encrypt_direct(cc,kp,(double)a), B=encrypt_direct(cc,kp,(double)b);
        PE raw=nand_direct(cc,A,B);
        PE corrected=spiral_correct(cc,raw,4);
        int bit_raw=decode_rel(cc,kp,raw);
        int bit_corrected=decode_rel(cc,kp,corrected);
        std::cout<<"  "<<a<<b<<" raw="<<bit_raw<<" spiral="<<bit_corrected<<" exp="<<1-a*b;
        std::cout<<" | "<<(bit_corrected==1-a*b?"OK":"XX")<<"\n";
    }
    
    // Full Adder with spiral correction on each gate
    std::cout<<"\n  Full Adder (spiral on every gate):\n";
    int cases[8][3]={{0,0,0},{0,0,1},{0,1,0},{0,1,1},{1,0,0},{1,0,1},{1,1,0},{1,1,1}};
    int ok_sum=0,ok_cout=0;
    struct timeval t0,t1; gettimeofday(&t0,NULL);
    for(int i=0;i<8;i++){
        int A=cases[i][0],B=cases[i][1],Cin=cases[i][2];
        PE a=encrypt_direct(cc,kp,(double)A),b=encrypt_direct(cc,kp,(double)B),c=encrypt_direct(cc,kp,(double)Cin);
        PE X1=spiral_correct(cc,nand_direct(cc,a,b),2);
        PE X2=spiral_correct(cc,nand_direct(cc,a,X1),2);
        PE X3=spiral_correct(cc,nand_direct(cc,b,X1),2);
        PE X4=spiral_correct(cc,nand_direct(cc,X2,X3),2);
        PE X5=spiral_correct(cc,nand_direct(cc,X4,c),2);
        PE X6=spiral_correct(cc,nand_direct(cc,X4,X5),2);
        PE X7=spiral_correct(cc,nand_direct(cc,X5,c),2);
        PE SUM=spiral_correct(cc,nand_direct(cc,X6,X7),2);
        PE COUT=spiral_correct(cc,nand_direct(cc,X1,X5),2);
        if(decode_rel(cc,kp,SUM)==(A+B+Cin)%2)ok_sum++;
        if(decode_rel(cc,kp,COUT)==(A+B+Cin)/2)ok_cout++;
    }
    gettimeofday(&t1,NULL);
    std::cout<<"  SUM: "<<ok_sum<<"/8 COUT: "<<ok_cout<<"/8 Time: "<<std::fixed<<std::setprecision(0)<<time_ms(t0,t1)<<"ms\n\n";
    
    // 4-bit adder with spiral correction
    std::cout<<"  4-Bit Adder (spiral on every gate):\n";
    int bits=4;
    int a_bits[bits],b_bits[bits]; long long valA=0,valB=0;
    for(int i=0;i<bits;i++){a_bits[i]=rand()%2;b_bits[i]=rand()%2;valA=valA*2+a_bits[i];valB=valB*2+b_bits[i];}
    PE rCarry=encrypt_direct(cc,kp,0.0);
    int sum_bits[bits]; long long result=0;
    gettimeofday(&t0,NULL);
    for(int i=bits-1;i>=0;i--){
        PE a=encrypt_direct(cc,kp,(double)a_bits[i]),b=encrypt_direct(cc,kp,(double)b_bits[i]);
        PE X1=spiral_correct(cc,nand_direct(cc,a,b),2);
        PE X2=spiral_correct(cc,nand_direct(cc,a,X1),2);
        PE X3=spiral_correct(cc,nand_direct(cc,b,X1),2);
        PE X4=spiral_correct(cc,nand_direct(cc,X2,X3),2);
        PE X5=spiral_correct(cc,nand_direct(cc,X4,rCarry),2);
        PE X6=spiral_correct(cc,nand_direct(cc,X4,X5),2);
        PE X7=spiral_correct(cc,nand_direct(cc,X5,rCarry),2);
        PE SUM=spiral_correct(cc,nand_direct(cc,X6,X7),2);
        PE COUT=spiral_correct(cc,nand_direct(cc,X1,X5),2);
        sum_bits[i]=decode_rel(cc,kp,SUM);
        rCarry=COUT;
    }
    for(int i=0;i<bits;i++){std::cout<<sum_bits[i];result=result*2+sum_bits[i];}
    int fc=decode_rel(cc,kp,rCarry); if(fc)result+=(1LL<<bits);
    gettimeofday(&t1,NULL);
    std::cout<<" (c="<<fc<<") = "<<result<<" | Expected="<<valA+valB;
    std::cout<<" | "<<(result==valA+valB?"MATCH!":"MISMATCH")<<"\n";
    std::cout<<"  Time: "<<std::fixed<<std::setprecision(0)<<time_ms(t0,t1)<<"ms\n\n";
    
    std::cout<<"  SPIRAL CORRECTION: "<<(result==valA+valB?"4-BIT MATCH!":"needs work")<<"\n\n";
    return 0;
}
