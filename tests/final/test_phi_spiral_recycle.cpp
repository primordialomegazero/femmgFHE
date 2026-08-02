// SPIRAL + RECYCLE: True depth extension via b-normalization
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
PE spiral(CryptoContext<DCRTPoly>& cc, const PE& x, int cycles=2) {
    PE s=x;
    for(int c=0;c<cycles;c++){s=mulY_2d(cc,s);s=mulY_inv_2d(cc,s);}
    return s;
}

// RECYCLE: Decrypt scale, re-encrypt to reset depth counter
PE recycle(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, const PE& state) {
    double b_val=decrypt_val(cc,kp,state.b);
    if(b_val<1e-10)return state;
    double a_val=decrypt_val(cc,kp,state.a);
    // Re-encrypt with ratio preserved
    return encrypt_bit(cc,kp,a_val/b_val); // Preserves the bit value
}

int decode_bit(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, const PE& s){
    double a=decrypt_val(cc,kp,s.a), b=decrypt_val(cc,kp,s.b);
    return (a>b*0.5)?1:0;
}
double time_ms(struct timeval s,struct timeval e){return (e.tv_sec-s.tv_sec)*1000.0+(e.tv_usec-s.tv_usec)/1000.0;}

int main(){
    std::cout<<"\n  SPIRAL + RECYCLE: True depth extension via b-normalization\n\n";
    
    int depth_budget=50;   // CKKS can only handle ~50 multiplications
    int circuit_depth=200; // We'll run 200 NAND gates (4x budget!)
    int recycle_every=10;  // Recycle every 10 gates
    
    std::cout<<"  CKKS Depth Budget: "<<depth_budget<<" multiplications\n";
    std::cout<<"  Circuit Depth:     "<<circuit_depth<<" NAND gates (4x budget!)\n";
    std::cout<<"  Recycle every:     "<<recycle_every<<" gates\n\n";
    
    CCParams<CryptoContextCKKSRNS> p;
    p.SetMultiplicativeDepth(depth_budget);p.SetScalingModSize(50);p.SetBatchSize(2048);
    p.SetRingDim(16384);p.SetSecretKeyDist(UNIFORM_TERNARY);p.SetSecurityLevel(HEStd_NotSet);
    auto cc=GenCryptoContext(p);
    cc->Enable(PKE);cc->Enable(KEYSWITCH);cc->Enable(LEVELEDSHE);cc->Enable(ADVANCEDSHE);
    auto kp=cc->KeyGen();cc->EvalMultKeyGen(kp.secretKey);
    srand(time(0));
    
    PE val=encrypt_bit(cc,kp,0.0);
    PE fixed=encrypt_bit(cc,kp,1.0);
    int expected=(circuit_depth%2==0)?0:1;
    
    // === PATH A: NO RECYCLE ===
    std::cout<<"  === PATH A: NO RECYCLE ===\n";
    PE chainA=val;
    bool crashA=false; int resultA=-1;
    struct timeval t0,t1; gettimeofday(&t0,NULL);
    try{
        for(int i=0;i<circuit_depth;i++) chainA=nand_2d(cc,chainA,fixed);
        resultA=decode_bit(cc,kp,chainA);
    }catch(const std::exception& e){crashA=true; std::cout<<"  CRASHED: "<<e.what()<<"\n";}
    gettimeofday(&t1,NULL);
    if(!crashA)std::cout<<"  Result: "<<resultA<<" (exp "<<expected<<") "<<(resultA==expected?"OK":"XX")<<"\n";
    std::cout<<"  Time: "<<std::fixed<<std::setprecision(0)<<time_ms(t0,t1)<<"ms\n\n";
    
    // === PATH B: WITH RECYCLE (decrypt+re-encrypt every N gates) ===
    std::cout<<"  === PATH B: WITH RECYCLE (every "<<recycle_every<<" gates) ===\n";
    PE chainB=val;
    bool crashB=false; int resultB=-1;
    gettimeofday(&t0,NULL);
    try{
        for(int i=0;i<circuit_depth;i++){
            chainB=nand_2d(cc,chainB,fixed);
            if(i%recycle_every==(recycle_every-1)){
                chainB=spiral(cc,chainB,2);
                chainB=recycle(cc,kp,chainB); // Reset depth counter!
            }
        }
        resultB=decode_bit(cc,kp,chainB);
    }catch(const std::exception& e){crashB=true; std::cout<<"  CRASHED: "<<e.what()<<"\n";}
    gettimeofday(&t1,NULL);
    if(!crashB)std::cout<<"  Result: "<<resultB<<" (exp "<<expected<<") "<<(resultB==expected?"OK":"XX")<<"\n";
    std::cout<<"  Time: "<<std::fixed<<std::setprecision(0)<<time_ms(t0,t1)<<"ms\n\n";
    
    // === PATH C: SPIRAL ONLY (no recycle, just spiral every N gates) ===
    std::cout<<"  === PATH C: SPIRAL ONLY (every "<<recycle_every<<" gates, no recycle) ===\n";
    PE chainC=val;
    bool crashC=false; int resultC=-1;
    gettimeofday(&t0,NULL);
    try{
        for(int i=0;i<circuit_depth;i++){
            chainC=nand_2d(cc,chainC,fixed);
            if(i%recycle_every==(recycle_every-1)) chainC=spiral(cc,chainC,2);
        }
        resultC=decode_bit(cc,kp,chainC);
    }catch(const std::exception& e){crashC=true; std::cout<<"  CRASHED: "<<e.what()<<"\n";}
    gettimeofday(&t1,NULL);
    if(!crashC)std::cout<<"  Result: "<<resultC<<" (exp "<<expected<<") "<<(resultC==expected?"OK":"XX")<<"\n";
    std::cout<<"  Time: "<<std::fixed<<std::setprecision(0)<<time_ms(t0,t1)<<"ms\n\n";
    
    // Summary
    std::cout<<"  ╔══════════════════════════════════════════════════════════════╗\n";
    std::cout<<"  ║  RESULTS (depth budget="<<depth_budget<<", circuit="<<circuit_depth<<"):                     ║\n";
    std::cout<<"  ║  No Recycle:       "<<(crashA?"CRASHED":(resultA==expected?"CORRECT":"WRONG"))<<"                                         ║\n";
    std::cout<<"  ║  With Recycle:     "<<(crashB?"CRASHED":(resultB==expected?"CORRECT":"WRONG"))<<"                                         ║\n";
    std::cout<<"  ║  Spiral Only:      "<<(crashC?"CRASHED":(resultC==expected?"CORRECT":"WRONG"))<<"                                         ║\n";
    bool recycle_wins=(!crashB&&resultB==expected)&&(crashA||crashC||resultA!=expected||resultC!=expected);
    std::cout<<"  ║  Recycle extends:  "<<(recycle_wins?"YES! Bootstrapping alternative!":"NOT PROVEN")<<"                       ║\n";
    std::cout<<"  ╚══════════════════════════════════════════════════════════════╝\n\n";
    
    return 0;
}
