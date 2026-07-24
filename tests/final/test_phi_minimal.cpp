// MINIMAL: Obfuscate only inputs, accept only final output. No intermediate overhead.
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
PE accept_final(CryptoContext<DCRTPoly>& cc, const PE& state) {
    PE s=state;
    for(int i=0;i<6;i++) s=mulY(cc,s);
    for(int i=0;i<6;i++) s=mulY_inv(cc,s);
    return s;
}
int decode_rel(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, const PE& s){
    double a=decrypt_val(cc,kp,s.a), b=decrypt_val(cc,kp,s.b);
    return (a>b*0.5)?1:0;
}

struct FAR { PE sum; PE cout; };
FAR full_adder_minimal(CryptoContext<DCRTPoly>& cc, const PE& A, const PE& B, const PE& Cin){
    PE X1=nand_direct(cc,A,B);
    PE X2=nand_direct(cc,A,X1);
    PE X3=nand_direct(cc,B,X1);
    PE X4=nand_direct(cc,X2,X3);
    PE X5=nand_direct(cc,X4,Cin);
    PE X6=nand_direct(cc,X4,X5);
    PE X7=nand_direct(cc,X5,Cin);
    return {nand_direct(cc,X6,X7), nand_direct(cc,X1,X5)};
}

double time_ms(struct timeval s,struct timeval e){return (e.tv_sec-s.tv_sec)*1000.0+(e.tv_usec-s.tv_usec)/1000.0;}

int main(){
    std::cout<<"\n  MINIMAL: Obfuscate only inputs, accept only final output\n\n";
    
    CCParams<CryptoContextCKKSRNS> p;
    p.SetMultiplicativeDepth(100);p.SetScalingModSize(50);p.SetBatchSize(2048);
    p.SetRingDim(4096);p.SetSecretKeyDist(UNIFORM_TERNARY);p.SetSecurityLevel(HEStd_NotSet);
    auto cc=GenCryptoContext(p);
    cc->Enable(PKE);cc->Enable(KEYSWITCH);cc->Enable(LEVELEDSHE);cc->Enable(ADVANCEDSHE);
    auto kp=cc->KeyGen();cc->EvalMultKeyGen(kp.secretKey);
    srand(time(0));
    
    // Full Adder: obfuscate inputs, pure NAND chain, accept final outputs
    std::cout<<"  Full Adder (obf inputs, pure NAND, accept final):\n";
    int cases[8][3]={{0,0,0},{0,0,1},{0,1,0},{0,1,1},{1,0,0},{1,0,1},{1,1,0},{1,1,1}};
    int ok_sum=0,ok_cout=0;
    struct timeval t0,t1; gettimeofday(&t0,NULL);
    for(int i=0;i<8;i++){
        int A=cases[i][0],B=cases[i][1],Cin=cases[i][2];
        PE a=encrypt_direct(cc,kp,(double)A),b=encrypt_direct(cc,kp,(double)B),c=encrypt_direct(cc,kp,(double)Cin);
        double s=0.5+(rand()%300)/100.0;
        PE oa=obfuscate(cc,kp,a,s), ob=obfuscate(cc,kp,b,s), oc=obfuscate(cc,kp,c,s);
        FAR fa=full_adder_minimal(cc,oa,ob,oc);
        PE clean_sum=accept_final(cc,fa.sum), clean_cout=accept_final(cc,fa.cout);
        if(decode_rel(cc,kp,clean_sum)==(A+B+Cin)%2)ok_sum++;
        if(decode_rel(cc,kp,clean_cout)==(A+B+Cin)/2)ok_cout++;
    }
    gettimeofday(&t1,NULL);
    std::cout<<"  SUM: "<<ok_sum<<"/8 COUT: "<<ok_cout<<"/8 Time: "<<std::fixed<<std::setprecision(0)<<time_ms(t0,t1)<<"ms\n\n";
    
    // 4-bit: obfuscate inputs, pure NAND, raw carry, accept only final
    std::cout<<"  4-Bit Adder (minimal):\n";
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
        FAR fa=full_adder_minimal(cc,oa,ob,orCarry);
        sum_bits[i]=decode_rel(cc,kp,accept_final(cc,fa.sum));
        orCarry=fa.cout; // RAW carry, no accept
    }
    for(int i=0;i<4;i++){std::cout<<sum_bits[i];result=result*2+sum_bits[i];}
    int fc=decode_rel(cc,kp,accept_final(cc,orCarry)); if(fc)result+=16;
    gettimeofday(&t1,NULL);
    std::cout<<" (c="<<fc<<") = "<<result<<" | Expected="<<valA+valB<<" | "<<(result==valA+valB?"MATCH!":"MISMATCH")<<"\n";
    std::cout<<"  Time: "<<std::fixed<<std::setprecision(0)<<time_ms(t0,t1)<<"ms\n\n";
    
    std::cout<<"  MINIMAL: "<<(result==valA+valB?"4-BIT MATCH!":"needs work")<<"\n\n";
    return 0;
}
