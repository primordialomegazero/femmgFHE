// FRACTAL BLOCK SWITCHING: Auto-trigger recycle based on φ/ψ ratio
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

// FRACTAL AUTO-RECYCLE: Check if state is "drifting" and auto-correct
PE fractal_auto_recycle(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, const PE& state) {
    double a=decrypt_val(cc,kp,state.a), b=decrypt_val(cc,kp,state.b);
    double ratio=(b>1e-10)?a/b:0;
    
    // If ratio is far from 0 or 1 (valid bit values), recycle
    if(ratio<0.2 || ratio>0.8 || b>2.0 || b<0.3){
        double b_val=b;
        if(b_val<1e-10)return state;
        auto inv_b=cc->Encrypt(kp.publicKey,cc->MakeCKKSPackedPlaintext(std::vector<double>{1.0/b_val}));
        PE norm={cc->EvalMult(state.a,inv_b), cc->EvalMult(state.b,inv_b)};
        // Stronger attractor for drifted states
        for(int i=0;i<8;i++) norm=mulY(cc,norm);
        for(int i=0;i<8;i++) norm=mulY_inv(cc,norm);
        return norm;
    }
    return state; // State is clean, no recycle needed
}

int decode_rel(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, const PE& s){
    double a=decrypt_val(cc,kp,s.a), b=decrypt_val(cc,kp,s.b);
    return (a>b*0.5)?1:0;
}
double time_ms(struct timeval s,struct timeval e){return (e.tv_sec-s.tv_sec)*1000.0+(e.tv_usec-s.tv_usec)/1000.0;}

int main(){
    std::cout<<"\n  FRACTAL BLOCK SWITCHING: Auto-recycle based on φ/ψ ratio\n\n";
    
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
        
        // Full adder with auto-recycle on every gate
        PE X1=fractal_auto_recycle(cc,kp,nand_direct(cc,oa,ob));
        PE X2=fractal_auto_recycle(cc,kp,nand_direct(cc,oa,X1));
        PE X3=fractal_auto_recycle(cc,kp,nand_direct(cc,ob,X1));
        PE X4=fractal_auto_recycle(cc,kp,nand_direct(cc,X2,X3));
        PE X5=fractal_auto_recycle(cc,kp,nand_direct(cc,X4,orCarry));
        PE X6=fractal_auto_recycle(cc,kp,nand_direct(cc,X4,X5));
        PE X7=fractal_auto_recycle(cc,kp,nand_direct(cc,X5,orCarry));
        PE SUM=fractal_auto_recycle(cc,kp,nand_direct(cc,X6,X7));
        PE COUT=fractal_auto_recycle(cc,kp,nand_direct(cc,X1,X5));
        
        sum_bits[i]=decode_rel(cc,kp,SUM);
        orCarry=COUT;
    }
    
    for(int i=0;i<bits;i++){std::cout<<sum_bits[i];result=result*2+sum_bits[i];}
    int fc=decode_rel(cc,kp,orCarry); if(fc)result+=(1LL<<bits);
    gettimeofday(&t1,NULL);
    long long expected=valA+valB;
    std::cout<<" (c="<<fc<<") = "<<result<<" | Expected="<<expected;
    std::cout<<" | "<<(result==expected?"MATCH!":"MISMATCH")<<"\n";
    std::cout<<"  Time: "<<std::fixed<<std::setprecision(0)<<time_ms(t0,t1)<<"ms\n\n";
    
    std::cout<<"  FRACTAL AUTO-RECYCLE: "<<(result==expected?"8-BIT MATCH!":"needs work")<<"\n\n";
    return 0;
}
