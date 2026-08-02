// 8-BIT ADDER v2: 2-gate blocks, more aggressive recycle
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
PE block_recycle(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, const PE& state) {
    double b_val=decrypt_val(cc,kp,state.b);
    if(b_val<1e-10)return state;
    auto inv_b=cc->Encrypt(kp.publicKey,cc->MakeCKKSPackedPlaintext(std::vector<double>{1.0/b_val}));
    PE norm={cc->EvalMult(state.a,inv_b), cc->EvalMult(state.b,inv_b)};
    for(int i=0;i<6;i++) norm=mulY(cc,norm);
    for(int i=0;i<6;i++) norm=mulY_inv(cc,norm);
    return norm;
}
int decode_rel(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, const PE& s){
    double a=decrypt_val(cc,kp,s.a), b=decrypt_val(cc,kp,s.b);
    return (a>b*0.5)?1:0;
}
double time_ms(struct timeval s,struct timeval e){return (e.tv_sec-s.tv_sec)*1000.0+(e.tv_usec-s.tv_usec)/1000.0;}

int main(){
    std::cout<<"\n  8-BIT ADDER v2: 2-gate blocks, more recycle\n\n";
    
    CCParams<CryptoContextCKKSRNS> p;
    p.SetMultiplicativeDepth(20);p.SetScalingModSize(50);p.SetBatchSize(2048);
    p.SetRingDim(4096);p.SetSecretKeyDist(UNIFORM_TERNARY);p.SetSecurityLevel(HEStd_NotSet);
    auto cc=GenCryptoContext(p);
    cc->Enable(PKE);cc->Enable(KEYSWITCH);cc->Enable(LEVELEDSHE);cc->Enable(ADVANCEDSHE);
    auto kp=cc->KeyGen();cc->EvalMultKeyGen(kp.secretKey);
    srand(time(0));
    
    int bits=8;
    int a_bits[bits],b_bits[bits];
    long long valA=0,valB=0;
    for(int i=0;i<bits;i++){a_bits[i]=rand()%2;b_bits[i]=rand()%2;valA=valA*2+a_bits[i];valB=valB*2+b_bits[i];}
    
    PE rCarry=encrypt_direct(cc,kp,0.0);
    int sum_bits[bits]; long long result=0;
    struct timeval t0,t1; gettimeofday(&t0,NULL);
    double gs=0.5+(rand()%300)/100.0;
    PE orCarry=obfuscate(cc,kp,rCarry,gs);
    
    for(int i=bits-1;i>=0;i--){
        PE a=encrypt_direct(cc,kp,(double)a_bits[i]),b=encrypt_direct(cc,kp,(double)b_bits[i]);
        PE oa=obfuscate(cc,kp,a,gs), ob=obfuscate(cc,kp,b,gs);
        
        // 2-gate blocks with recycle after each
        PE X1=nand_direct(cc,oa,ob);
        PE X2=nand_direct(cc,oa,X1); X1=block_recycle(cc,kp,X1);
        PE X3=nand_direct(cc,ob,X1); X2=block_recycle(cc,kp,X2);
        PE X4=nand_direct(cc,X2,X3); X3=block_recycle(cc,kp,X3);
        PE X5=nand_direct(cc,X4,orCarry); X4=block_recycle(cc,kp,X4);
        PE X6=nand_direct(cc,X4,X5); X5=block_recycle(cc,kp,X5);
        PE X7=nand_direct(cc,X5,orCarry); X6=block_recycle(cc,kp,X6);
        PE SUM=nand_direct(cc,X6,X7);
        PE COUT=nand_direct(cc,X1,X5);
        
        sum_bits[i]=decode_rel(cc,kp,block_recycle(cc,kp,SUM));
        orCarry=block_recycle(cc,kp,COUT);
    }
    
    for(int i=0;i<bits;i++){std::cout<<sum_bits[i];result=result*2+sum_bits[i];}
    int fc=decode_rel(cc,kp,orCarry); if(fc)result+=(1LL<<bits);
    gettimeofday(&t1,NULL);
    long long expected=valA+valB;
    std::cout<<" (c="<<fc<<") = "<<result<<" | Expected="<<expected;
    std::cout<<" | "<<(result==expected?"MATCH!":"MISMATCH")<<"\n";
    std::cout<<"  Time: "<<std::fixed<<std::setprecision(0)<<time_ms(t0,t1)<<"ms\n\n";
    
    std::cout<<"  8-BIT ADDER v2: "<<(result==expected?"MATCH! 72 gates!":"needs work")<<"\n\n";
    return 0;
}
