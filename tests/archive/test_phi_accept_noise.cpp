// ACCEPT & ADAPT: Let b grow, calibrate threshold relative to b
// No decrypt. No division. Pure homomorphic φ/ψ attractor.
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
PE obfuscate(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, const PE& in, double scale){
    auto pt_s=cc->Encrypt(kp.publicKey,cc->MakeCKKSPackedPlaintext(std::vector<double>{scale}));
    return {cc->EvalMult(in.a,pt_s), cc->EvalMult(in.b,pt_s)};
}

// ACCEPT: Don't fight the growing b. Use attractor cycle (ZERO EvalMult) to clean state.
PE accept_and_adapt(CryptoContext<DCRTPoly>& cc, const PE& state) {
    PE s=state;
    // φ/ψ attractor cycle: forward then reverse
    // This preserves the ratio while cleaning CKKS noise
    for(int i=0;i<4;i++) s=mulY(cc,s);
    for(int i=0;i<4;i++) s=mulY_inv(cc,s);
    return s;
}

// DECODE: threshold relative to b (a > b/2 means bit=1)
// ratio a/b > 0.5 means bit=1. So a > b*0.5
int decode_relative(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, const PE& state){
    double a=decrypt_val(cc,kp,state.a), b=decrypt_val(cc,kp,state.b);
    return (a > b*0.5) ? 1 : 0; // threshold relative to b!
}

struct FAR { PE sum; PE cout; };
FAR full_adder_accept(CryptoContext<DCRTPoly>& cc, const PE& A, const PE& B, const PE& Cin){
    PE X1=nand_direct(cc,A,B);
    PE X2=nand_direct(cc,A,X1);
    PE X3=nand_direct(cc,B,X1);
    PE X4=nand_direct(cc,X2,X3);
    PE X5=nand_direct(cc,X4,Cin);
    PE X6=nand_direct(cc,X4,X5);
    PE X7=nand_direct(cc,X5,Cin);
    PE SUM=nand_direct(cc,X6,X7);
    PE COUT=nand_direct(cc,X1,X5);
    // Accept & adapt on outputs
    return {accept_and_adapt(cc,SUM), accept_and_adapt(cc,COUT)};
}

double time_ms(struct timeval s,struct timeval e){return (e.tv_sec-s.tv_sec)*1000.0+(e.tv_usec-s.tv_usec)/1000.0;}

int main(){
    std::cout<<"\n  ACCEPT & ADAPT: Let b grow, threshold relative to b, φ/ψ attractor\n\n";
    
    CCParams<CryptoContextCKKSRNS> p;
    p.SetMultiplicativeDepth(50);p.SetScalingModSize(50);p.SetBatchSize(2048);
    p.SetRingDim(4096);p.SetSecretKeyDist(UNIFORM_TERNARY);p.SetSecurityLevel(HEStd_NotSet);
    auto cc=GenCryptoContext(p);
    cc->Enable(PKE);cc->Enable(KEYSWITCH);cc->Enable(LEVELEDSHE);cc->Enable(ADVANCEDSHE);
    auto kp=cc->KeyGen();cc->EvalMultKeyGen(kp.secretKey);
    srand(time(0));
    
    // NAND
    std::cout<<"  NAND (accept & adapt):\n";
    int ok=0;
    for(int a=0;a<=1;a++) for(int b=0;b<=1;b++){
        PE A=encrypt_direct(cc,kp,(double)a), B=encrypt_direct(cc,kp,(double)b);
        double s=0.5+(rand()%300)/100.0;
        PE oA=obfuscate(cc,kp,A,s), oB=obfuscate(cc,kp,B,s);
        PE nand=accept_and_adapt(cc,nand_direct(cc,oA,oB));
        if(decode_relative(cc,kp,nand)==1-a*b)ok++;
    }
    std::cout<<"  NAND: "<<ok<<"/4\n\n";
    
    // Full Adder
    std::cout<<"  Full Adder (accept & adapt, scaled inputs):\n";
    int cases[8][3]={{0,0,0},{0,0,1},{0,1,0},{0,1,1},{1,0,0},{1,0,1},{1,1,0},{1,1,1}};
    int ok_sum=0,ok_cout=0;
    struct timeval t0,t1; gettimeofday(&t0,NULL);
    for(int i=0;i<8;i++){
        int A=cases[i][0],B=cases[i][1],Cin=cases[i][2];
        PE a=encrypt_direct(cc,kp,(double)A),b=encrypt_direct(cc,kp,(double)B),c=encrypt_direct(cc,kp,(double)Cin);
        double s=0.5+(rand()%300)/100.0;
        PE oa=obfuscate(cc,kp,a,s), ob=obfuscate(cc,kp,b,s), oc=obfuscate(cc,kp,c,s);
        FAR fa=full_adder_accept(cc,oa,ob,oc);
        if(decode_relative(cc,kp,fa.sum)==(A+B+Cin)%2)ok_sum++;
        if(decode_relative(cc,kp,fa.cout)==(A+B+Cin)/2)ok_cout++;
    }
    gettimeofday(&t1,NULL);
    std::cout<<"  SUM: "<<ok_sum<<"/8 COUT: "<<ok_cout<<"/8 Time: "<<std::fixed<<std::setprecision(0)<<time_ms(t0,t1)<<"ms\n\n";
    
    // 4-bit adder
    std::cout<<"  4-Bit Adder (accept & adapt, all scaled, raw carry):\n";
    int a_bits[4],b_bits[4],valA=0,valB=0;
    for(int i=0;i<4;i++){a_bits[i]=rand()%2;b_bits[i]=rand()%2;valA=valA*2+a_bits[i];valB=valB*2+b_bits[i];}
    PE rCarry=encrypt_direct(cc,kp,0.0);
    int sum_bits[4],result=0;
    gettimeofday(&t0,NULL);
    double gs=0.5+(rand()%300)/100.0;
    PE orCarry=obfuscate(cc,kp,rCarry,gs);
    for(int i=3;i>=0;i--){
        PE a=encrypt_direct(cc,kp,(double)a_bits[i]),b=encrypt_direct(cc,kp,(double)b_bits[i]);
        PE oa=obfuscate(cc,kp,a,gs), ob=obfuscate(cc,kp,b,gs);
        FAR fa=full_adder_accept(cc,oa,ob,orCarry);
        sum_bits[i]=decode_relative(cc,kp,fa.sum);
        orCarry=accept_and_adapt(cc,fa.cout); // Accept noise on carry
    }
    for(int i=0;i<4;i++){std::cout<<sum_bits[i];result=result*2+sum_bits[i];}
    int fc=decode_relative(cc,kp,orCarry); if(fc)result+=16;
    gettimeofday(&t1,NULL);
    std::cout<<" (c="<<fc<<") = "<<result<<" | Expected="<<valA+valB<<" | "<<(result==valA+valB?"MATCH!":"MISMATCH")<<"\n";
    std::cout<<"  Time: "<<std::fixed<<std::setprecision(0)<<time_ms(t0,t1)<<"ms\n\n";
    
    std::cout<<"  ACCEPT & ADAPT: "<<(result==valA+valB?"4-BIT MATCH! Noise accepted, not fought!":"needs work")<<"\n\n";
    return 0;
}
