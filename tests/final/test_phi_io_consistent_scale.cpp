// CONSISTENT SCALE iO: Same random scale for all wires, original NAND
#include <iostream>
#include <iomanip>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <sys/time.h>
#include "openfhe.h"
using namespace lbcrypto;

struct PE { Ciphertext<DCRTPoly> a; Ciphertext<DCRTPoly> b; };

double decrypt_val(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, const Ciphertext<DCRTPoly>& c) {
    Plaintext pt; cc->Decrypt(kp.secretKey, c, &pt); return pt->GetCKKSPackedValue()[0].real();
}
PE encrypt_bit(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, int bit) {
    const double PSI=0.6180339887498949; double v=(bit==1)?1.0:0.0;
    return {cc->Encrypt(kp.publicKey,cc->MakeCKKSPackedPlaintext(std::vector<double>{v+PSI})),
            cc->Encrypt(kp.publicKey,cc->MakeCKKSPackedPlaintext(std::vector<double>{1.0}))};
}
const double PSI=0.6180339887498949;
const double THRESHOLD=PSI+0.5;

PE nand_fhe(CryptoContext<DCRTPoly>& cc, const Ciphertext<DCRTPoly>& ct_psi,
            const Ciphertext<DCRTPoly>& ct_2psi, const PE& A, const PE& B) {
    auto a1_psi=cc->EvalMult(A.a,ct_psi), b1_psi=cc->EvalMult(B.a,ct_psi);
    auto term1=cc->EvalAdd(a1_psi,b1_psi), term1_plus=cc->EvalAdd(term1,ct_2psi);
    auto prod=cc->EvalMult(A.a,B.a); return {cc->EvalSub(term1_plus,prod), A.b};
}
PE obfuscate(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, const PE& in, double scale){
    auto pt_s=cc->Encrypt(kp.publicKey,cc->MakeCKKSPackedPlaintext(std::vector<double>{scale}));
    return {cc->EvalMult(in.a,pt_s), cc->EvalMult(in.b,pt_s)};
}
int decode_direct(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, const PE& state){
    return (decrypt_val(cc,kp,state.a)/decrypt_val(cc,kp,state.b)>THRESHOLD)?1:0;
}

struct FAR { PE sum; PE cout; };
FAR full_adder_pure(CryptoContext<DCRTPoly>& cc, const Ciphertext<DCRTPoly>& ct_psi,
    const Ciphertext<DCRTPoly>& ct_2psi, const PE& A, const PE& B, const PE& Cin){
    PE X1=nand_fhe(cc,ct_psi,ct_2psi,A,B);
    PE X2=nand_fhe(cc,ct_psi,ct_2psi,A,X1);
    PE X3=nand_fhe(cc,ct_psi,ct_2psi,B,X1);
    PE X4=nand_fhe(cc,ct_psi,ct_2psi,X2,X3);
    PE X5=nand_fhe(cc,ct_psi,ct_2psi,X4,Cin);
    PE X6=nand_fhe(cc,ct_psi,ct_2psi,X4,X5);
    PE X7=nand_fhe(cc,ct_psi,ct_2psi,X5,Cin);
    PE SUM=nand_fhe(cc,ct_psi,ct_2psi,X6,X7);
    PE COUT=nand_fhe(cc,ct_psi,ct_2psi,X1,X5);
    return {SUM, COUT};
}

double time_ms(struct timeval s,struct timeval e){return (e.tv_sec-s.tv_sec)*1000.0+(e.tv_usec-s.tv_usec)/1000.0;}

int main(){
    std::cout<<"\n  CONSISTENT SCALE iO: Same scale for all wires, original NAND\n\n";
    
    CCParams<CryptoContextCKKSRNS> p;
    p.SetMultiplicativeDepth(500);p.SetScalingModSize(50);p.SetBatchSize(2048);
    p.SetRingDim(4096);p.SetSecretKeyDist(UNIFORM_TERNARY);p.SetSecurityLevel(HEStd_NotSet);
    auto cc=GenCryptoContext(p);
    cc->Enable(PKE);cc->Enable(KEYSWITCH);cc->Enable(LEVELEDSHE);cc->Enable(ADVANCEDSHE);
    auto kp=cc->KeyGen();cc->EvalMultKeyGen(kp.secretKey);
    srand(time(0));
    
    auto ct_psi=cc->Encrypt(kp.publicKey,cc->MakeCKKSPackedPlaintext(std::vector<double>{PSI}));
    auto ct_2psi=cc->Encrypt(kp.publicKey,cc->MakeCKKSPackedPlaintext(std::vector<double>{2.0*PSI}));
    
    // NAND with consistent scale
    std::cout<<"  NAND (consistent scale on both inputs):\n";
    int ok_nand=0;
    for(int a=0;a<=1;a++) for(int b=0;b<=1;b++){
        PE A=encrypt_bit(cc,kp,a), B=encrypt_bit(cc,kp,b);
        double s=0.5+(rand()%300)/100.0; // SAME scale for both
        PE oA=obfuscate(cc,kp,A,s), oB=obfuscate(cc,kp,B,s);
        PE nand=nand_fhe(cc,ct_psi,ct_2psi,oA,oB);
        if(decode_direct(cc,kp,nand)==1-a*b)ok_nand++;
    }
    std::cout<<"  NAND: "<<ok_nand<<"/4\n\n";
    
    // Full Adder with consistent scale
    std::cout<<"  Full Adder (consistent scale on all 3 inputs):\n";
    int cases[8][3]={{0,0,0},{0,0,1},{0,1,0},{0,1,1},{1,0,0},{1,0,1},{1,1,0},{1,1,1}};
    int ok_sum=0,ok_cout=0;
    struct timeval t0,t1; gettimeofday(&t0,NULL);
    for(int i=0;i<8;i++){
        int A=cases[i][0],B=cases[i][1],Cin=cases[i][2];
        PE a=encrypt_bit(cc,kp,A),b=encrypt_bit(cc,kp,B),c=encrypt_bit(cc,kp,Cin);
        double s=0.5+(rand()%300)/100.0; // SAME scale for all inputs
        PE oa=obfuscate(cc,kp,a,s), ob=obfuscate(cc,kp,b,s), oc=obfuscate(cc,kp,c,s);
        FAR fa=full_adder_pure(cc,ct_psi,ct_2psi,oa,ob,oc);
        if(decode_direct(cc,kp,fa.sum)==(A+B+Cin)%2)ok_sum++;
        if(decode_direct(cc,kp,fa.cout)==(A+B+Cin)/2)ok_cout++;
    }
    gettimeofday(&t1,NULL);
    std::cout<<"  SUM: "<<ok_sum<<"/8 COUT: "<<ok_cout<<"/8 Time: "<<std::fixed<<std::setprecision(0)<<time_ms(t0,t1)<<"ms\n\n";
    
    // 4-bit adder with consistent scale
    std::cout<<"  4-Bit Adder (consistent scale, raw carry):\n";
    int a_bits[4],b_bits[4],valA=0,valB=0;
    for(int i=0;i<4;i++){a_bits[i]=rand()%2;b_bits[i]=rand()%2;valA=valA*2+a_bits[i];valB=valB*2+b_bits[i];}
    PE rCarry=encrypt_bit(cc,kp,0);
    int sum_bits[4],result=0;
    gettimeofday(&t0,NULL);
    double global_scale=0.5+(rand()%300)/100.0; // ONE global scale for entire circuit!
    PE orCarry=obfuscate(cc,kp,rCarry,global_scale);
    for(int i=3;i>=0;i--){
        PE a=encrypt_bit(cc,kp,a_bits[i]),b=encrypt_bit(cc,kp,b_bits[i]);
        PE oa=obfuscate(cc,kp,a,global_scale), ob=obfuscate(cc,kp,b,global_scale);
        FAR fa=full_adder_pure(cc,ct_psi,ct_2psi,oa,ob,orCarry);
        sum_bits[i]=decode_direct(cc,kp,fa.sum);
        orCarry=fa.cout;
    }
    for(int i=0;i<4;i++){std::cout<<sum_bits[i];result=result*2+sum_bits[i];}
    int fc=decode_direct(cc,kp,orCarry); if(fc)result+=16;
    gettimeofday(&t1,NULL);
    std::cout<<" (c="<<fc<<") = "<<result<<" | Expected="<<valA+valB<<" | "<<(result==valA+valB?"MATCH!":"MISMATCH")<<"\n";
    std::cout<<"  Time: "<<std::fixed<<std::setprecision(0)<<time_ms(t0,t1)<<"ms\n\n";
    
    bool all_ok=(ok_nand==4)&&(ok_sum==8)&&(ok_cout==8)&&(result==valA+valB);
    std::cout<<"  CONSISTENT SCALE iO: "<<(all_ok?"ALL TESTS PASSED!":"needs work")<<"\n\n";
    return 0;
}
