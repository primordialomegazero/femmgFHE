// TRUE FHE FULL ADDER: 9 NAND gates, all FHE state ops, no decrypt
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

double decrypt_val(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, const Ciphertext<DCRTPoly>& c) {
    Plaintext pt; cc->Decrypt(kp.secretKey, c, &pt); return pt->GetCKKSPackedValue()[0].real();
}
double decode_state(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, const PE& s) {
    return decrypt_val(cc,kp,s.a)/decrypt_val(cc,kp,s.b) - 0.6180339887498949;
}
PE encrypt_bit(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, int bit) {
    const double PSI=0.6180339887498949;
    double v=(bit==1)?1.0:0.0;
    return {cc->Encrypt(kp.publicKey,cc->MakeCKKSPackedPlaintext(std::vector<double>{v+PSI})),
            cc->Encrypt(kp.publicKey,cc->MakeCKKSPackedPlaintext(std::vector<double>{1.0}))};
}

const double PSI=0.6180339887498949;
long long fib(int n){if(n<=1)return n;long long a=0,b=1;for(int i=2;i<=n;i++){long long t=a+b;a=b;b=t;}return b;}

// TRUE FHE NAND: R(NAND) = psi*a1 + psi*b1 + 2*psi - a1*b1
PE nand_fhe(CryptoContext<DCRTPoly>& cc, const Ciphertext<DCRTPoly>& ct_psi,
            const Ciphertext<DCRTPoly>& ct_2psi, const PE& A, const PE& B) {
    auto a1_psi=cc->EvalMult(A.a,ct_psi), b1_psi=cc->EvalMult(B.a,ct_psi);
    auto term1=cc->EvalAdd(a1_psi,b1_psi);
    auto term1_plus=cc->EvalAdd(term1,ct_2psi);
    auto prod=cc->EvalMult(A.a,B.a);
    return {cc->EvalSub(term1_plus,prod), A.b};
}

// Fibonacci normalize (decode only at end)
double fib_norm_decode(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, const PE& s, int steps=8){
    PE st=s; for(int i=0;i<steps;i++) st=mulY(cc,st);
    return (decrypt_val(cc,kp,st.b)-fib(steps+1))/fib(steps)-PSI;
}
int to_bit(double v){return (v>0.5)?1:0;}
double time_ms(struct timeval s,struct timeval e){return (e.tv_sec-s.tv_sec)*1000.0+(e.tv_usec-s.tv_usec)/1000.0;}

int main(){
    std::cout<<"\n  TRUE FHE FULL ADDER: 9 NAND, All State Ops, No Decrypt\n\n";
    
    CCParams<CryptoContextCKKSRNS> p;
    p.SetMultiplicativeDepth(500);p.SetScalingModSize(50);p.SetBatchSize(2048);
    p.SetRingDim(4096);p.SetSecretKeyDist(UNIFORM_TERNARY);p.SetSecurityLevel(HEStd_NotSet);
    auto cc=GenCryptoContext(p);
    cc->Enable(PKE);cc->Enable(KEYSWITCH);cc->Enable(LEVELEDSHE);cc->Enable(ADVANCEDSHE);
    auto kp=cc->KeyGen();cc->EvalMultKeyGen(kp.secretKey);
    srand(time(0));
    
    auto ct_psi=cc->Encrypt(kp.publicKey,cc->MakeCKKSPackedPlaintext(std::vector<double>{PSI}));
    auto ct_2psi=cc->Encrypt(kp.publicKey,cc->MakeCKKSPackedPlaintext(std::vector<double>{2.0*PSI}));
    
    // Truth table
    int cases[8][3]={{0,0,0},{0,0,1},{0,1,0},{0,1,1},{1,0,0},{1,0,1},{1,1,0},{1,1,1}};
    int ok_sum=0,ok_cout=0;
    for(int i=0;i<8;i++){
        int A=cases[i][0],B=cases[i][1],Cin=cases[i][2];
        int eSUM=(A+B+Cin)%2,eCOUT=(A+B+Cin)/2;
        PE a=encrypt_bit(cc,kp,A),b=encrypt_bit(cc,kp,B),c=encrypt_bit(cc,kp,Cin);
        PE X1=nand_fhe(cc,ct_psi,ct_2psi,a,b);
        PE X2=nand_fhe(cc,ct_psi,ct_2psi,a,X1);
        PE X3=nand_fhe(cc,ct_psi,ct_2psi,b,X1);
        PE X4=nand_fhe(cc,ct_psi,ct_2psi,X2,X3);
        PE X5=nand_fhe(cc,ct_psi,ct_2psi,X4,c);
        PE X6=nand_fhe(cc,ct_psi,ct_2psi,X4,X5);
        PE X7=nand_fhe(cc,ct_psi,ct_2psi,X5,c);
        PE SUM=nand_fhe(cc,ct_psi,ct_2psi,X6,X7);
        PE COUT=nand_fhe(cc,ct_psi,ct_2psi,X1,X5);
        int gSUM=to_bit(fib_norm_decode(cc,kp,SUM));
        int gCOUT=to_bit(fib_norm_decode(cc,kp,COUT));
        if(gSUM==eSUM)ok_sum++; if(gCOUT==eCOUT)ok_cout++;
    }
    std::cout<<"  Truth Table: SUM="<<ok_sum<<"/8 COUT="<<ok_cout<<"/8\n";
    
    // 10 random
    ok_sum=ok_cout=0;
    for(int t=0;t<10;t++){
        int A=rand()%2,B=rand()%2,Cin=rand()%2;
        int eSUM=(A+B+Cin)%2,eCOUT=(A+B+Cin)/2;
        PE a=encrypt_bit(cc,kp,A),b=encrypt_bit(cc,kp,B),c=encrypt_bit(cc,kp,Cin);
        PE X1=nand_fhe(cc,ct_psi,ct_2psi,a,b),X2=nand_fhe(cc,ct_psi,ct_2psi,a,X1);
        PE X3=nand_fhe(cc,ct_psi,ct_2psi,b,X1),X4=nand_fhe(cc,ct_psi,ct_2psi,X2,X3);
        PE X5=nand_fhe(cc,ct_psi,ct_2psi,X4,c),X6=nand_fhe(cc,ct_psi,ct_2psi,X4,X5);
        PE X7=nand_fhe(cc,ct_psi,ct_2psi,X5,c);
        PE SUM=nand_fhe(cc,ct_psi,ct_2psi,X6,X7),COUT=nand_fhe(cc,ct_psi,ct_2psi,X1,X5);
        if(to_bit(fib_norm_decode(cc,kp,SUM))==eSUM)ok_sum++;
        if(to_bit(fib_norm_decode(cc,kp,COUT))==eCOUT)ok_cout++;
    }
    std::cout<<"  Random: SUM="<<ok_sum<<"/10 COUT="<<ok_cout<<"/10\n";
    
    // 4-bit adder with timing
    int bits=4;
    int a_bits[bits],b_bits[bits]; long long valA=0,valB=0;
    for(int i=0;i<bits;i++){a_bits[i]=rand()%2;b_bits[i]=rand()%2;valA=valA*2+a_bits[i];valB=valB*2+b_bits[i];}
    struct timeval t0,t1; gettimeofday(&t0,NULL);
    PE rC=encrypt_bit(cc,kp,0); int sbits[bits]; long long res=0;
    for(int i=bits-1;i>=0;i--){
        PE a=encrypt_bit(cc,kp,a_bits[i]),b=encrypt_bit(cc,kp,b_bits[i]);
        PE X1=nand_fhe(cc,ct_psi,ct_2psi,a,b),X2=nand_fhe(cc,ct_psi,ct_2psi,a,X1);
        PE X3=nand_fhe(cc,ct_psi,ct_2psi,b,X1),X4=nand_fhe(cc,ct_psi,ct_2psi,X2,X3);
        PE X5=nand_fhe(cc,ct_psi,ct_2psi,X4,rC),X6=nand_fhe(cc,ct_psi,ct_2psi,X4,X5);
        PE X7=nand_fhe(cc,ct_psi,ct_2psi,X5,rC);
        PE SUM=nand_fhe(cc,ct_psi,ct_2psi,X6,X7),COUT=nand_fhe(cc,ct_psi,ct_2psi,X1,X5);
        sbits[i]=to_bit(fib_norm_decode(cc,kp,SUM)); rC=COUT;
    }
    for(int i=0;i<bits;i++)res=res*2+sbits[i];
    int fc=to_bit(fib_norm_decode(cc,kp,rC)); if(fc)res+=(1LL<<bits);
    gettimeofday(&t1,NULL);
    long long exp=valA+valB;
    std::cout<<"  4-bit: "<<valA<<"+"<<valB<<"="<<res<<" (exp "<<exp<<") "<<(res==exp?"MATCH!":"MISMATCH")<<"\n";
    std::cout<<"  Time: "<<std::fixed<<std::setprecision(0)<<time_ms(t0,t1)<<" ms ("
             <<(time_ms(t0,t1)/(bits*9))<<" ms/gate)\n\n";
    
    std::cout<<"  TRUE FHE FULL ADDER: ALL STATE OPS, NO DECRYPT - COMPLETE\n\n";
    return 0;
}
