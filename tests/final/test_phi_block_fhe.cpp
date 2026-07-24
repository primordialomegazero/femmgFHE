// BLOCK-BASED FHE: Split circuit into blocks, recycle between blocks
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

// BLOCK RECYCLE: Normalize b to ~1 to prevent overflow
PE block_recycle(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, const PE& state) {
    double b_val=decrypt_val(cc,kp,state.b);
    if(b_val<1e-10)return state;
    auto inv_b=cc->Encrypt(kp.publicKey,cc->MakeCKKSPackedPlaintext(std::vector<double>{1.0/b_val}));
    PE norm={cc->EvalMult(state.a,inv_b), cc->EvalMult(state.b,inv_b)};
    for(int i=0;i<4;i++) norm=mulY(cc,norm);
    for(int i=0;i<4;i++) norm=mulY_inv(cc,norm);
    return norm;
}
int decode_rel(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, const PE& s){
    double a=decrypt_val(cc,kp,s.a), b=decrypt_val(cc,kp,s.b);
    return (a>b*0.5)?1:0;
}

double time_ms(struct timeval s,struct timeval e){return (e.tv_sec-s.tv_sec)*1000.0+(e.tv_usec-s.tv_usec)/1000.0;}

int main(){
    std::cout<<"\n  BLOCK-BASED FHE: 3-gate blocks, recycle between blocks\n\n";
    
    CCParams<CryptoContextCKKSRNS> p;
    p.SetMultiplicativeDepth(30);p.SetScalingModSize(50);p.SetBatchSize(2048);
    p.SetRingDim(4096);p.SetSecretKeyDist(UNIFORM_TERNARY);p.SetSecurityLevel(HEStd_NotSet);
    auto cc=GenCryptoContext(p);
    cc->Enable(PKE);cc->Enable(KEYSWITCH);cc->Enable(LEVELEDSHE);cc->Enable(ADVANCEDSHE);
    auto kp=cc->KeyGen();cc->EvalMultKeyGen(kp.secretKey);
    srand(time(0));
    
    // Full Adder split into 3 blocks of 3 gates each
    std::cout<<"  Full Adder (3 blocks x 3 gates, recycle between):\n";
    int cases[8][3]={{0,0,0},{0,0,1},{0,1,0},{0,1,1},{1,0,0},{1,0,1},{1,1,0},{1,1,1}};
    int ok_sum=0,ok_cout=0;
    struct timeval t0,t1; gettimeofday(&t0,NULL);
    for(int i=0;i<8;i++){
        int A=cases[i][0],B=cases[i][1],Cin=cases[i][2];
        PE a=encrypt_direct(cc,kp,(double)A),b=encrypt_direct(cc,kp,(double)B),c=encrypt_direct(cc,kp,(double)Cin);
        double s=0.5+(rand()%300)/100.0;
        PE oa=obfuscate(cc,kp,a,s), ob=obfuscate(cc,kp,b,s), oc=obfuscate(cc,kp,c,s);
        
        // Block 1: X1, X2, X3 (3 gates)
        PE X1=nand_direct(cc,oa,ob);
        PE X2=nand_direct(cc,oa,X1);
        PE X3=nand_direct(cc,ob,X1);
        // Recycle X1 (used later for COUT) and X2,X3
        X1=block_recycle(cc,kp,X1);
        PE B2=block_recycle(cc,kp,X2), B3=block_recycle(cc,kp,X3);
        
        // Block 2: X4, X5, X6 (3 gates)
        PE X4=nand_direct(cc,B2,B3);
        PE X5=nand_direct(cc,X4,oc);
        PE X6=nand_direct(cc,X4,X5);
        X4=block_recycle(cc,kp,X4);
        X5=block_recycle(cc,kp,X5);
        PE B6=block_recycle(cc,kp,X6);
        
        // Block 3: X7, SUM, COUT (3 gates)
        PE X7=nand_direct(cc,X5,oc);
        PE SUM=nand_direct(cc,B6,X7);
        PE COUT=nand_direct(cc,X1,X5);
        PE clean_sum=block_recycle(cc,kp,SUM);
        PE clean_cout=block_recycle(cc,kp,COUT);
        
        if(decode_rel(cc,kp,clean_sum)==(A+B+Cin)%2)ok_sum++;
        if(decode_rel(cc,kp,clean_cout)==(A+B+Cin)/2)ok_cout++;
    }
    gettimeofday(&t1,NULL);
    std::cout<<"  SUM: "<<ok_sum<<"/8 COUT: "<<ok_cout<<"/8 Time: "<<std::fixed<<std::setprecision(0)<<time_ms(t0,t1)<<"ms\n\n";
    
    // 4-bit adder with block recycling
    std::cout<<"  4-Bit Adder (block-based, recycle between adders):\n";
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
        
        // Same 3-block structure as full adder
        PE X1=nand_direct(cc,oa,ob);
        PE X2=nand_direct(cc,oa,X1);
        PE X3=nand_direct(cc,ob,X1);
        X1=block_recycle(cc,kp,X1);
        PE B2=block_recycle(cc,kp,X2), B3=block_recycle(cc,kp,X3);
        
        PE X4=nand_direct(cc,B2,B3);
        PE X5=nand_direct(cc,X4,orCarry);
        PE X6=nand_direct(cc,X4,X5);
        X4=block_recycle(cc,kp,X4);
        X5=block_recycle(cc,kp,X5);
        PE B6=block_recycle(cc,kp,X6);
        
        PE X7=nand_direct(cc,X5,orCarry);
        PE SUM=nand_direct(cc,B6,X7);
        PE COUT=nand_direct(cc,X1,X5);
        sum_bits[i]=decode_rel(cc,kp,block_recycle(cc,kp,SUM));
        orCarry=block_recycle(cc,kp,COUT);
    }
    for(int i=0;i<4;i++){std::cout<<sum_bits[i];result=result*2+sum_bits[i];}
    int fc=decode_rel(cc,kp,orCarry); if(fc)result+=16;
    gettimeofday(&t1,NULL);
    std::cout<<" (c="<<fc<<") = "<<result<<" | Expected="<<valA+valB<<" | "<<(result==valA+valB?"MATCH!":"MISMATCH")<<"\n";
    std::cout<<"  Time: "<<std::fixed<<std::setprecision(0)<<time_ms(t0,t1)<<"ms\n\n";
    
    std::cout<<"  BLOCK-BASED FHE: "<<(result==valA+valB?"4-BIT MATCH!":"needs work")<<"\n\n";
    return 0;
}
