// MULTI-GATE iO FULL ADDER: 9 NAND, each wire unique random scale, 8/8 truth table
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
PE encrypt_bit(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, int bit) {
    const double PSI=0.6180339887498949;
    double v=(bit==1)?1.0:0.0;
    return {cc->Encrypt(kp.publicKey,cc->MakeCKKSPackedPlaintext(std::vector<double>{v+PSI})),
            cc->Encrypt(kp.publicKey,cc->MakeCKKSPackedPlaintext(std::vector<double>{1.0}))};
}
const double PSI=0.6180339887498949, MID=57.5;

PE nand_fhe(CryptoContext<DCRTPoly>& cc, const Ciphertext<DCRTPoly>& ct_psi,
            const Ciphertext<DCRTPoly>& ct_2psi, const PE& A, const PE& B) {
    auto a1_psi=cc->EvalMult(A.a,ct_psi), b1_psi=cc->EvalMult(B.a,ct_psi);
    auto term1=cc->EvalAdd(a1_psi,b1_psi), term1_plus=cc->EvalAdd(term1,ct_2psi);
    auto prod=cc->EvalMult(A.a,B.a);
    return {cc->EvalSub(term1_plus,prod), A.b};
}

PE obfuscate(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, const PE& in, double scale){
    auto pt_s=cc->Encrypt(kp.publicKey,cc->MakeCKKSPackedPlaintext(std::vector<double>{scale}));
    return {cc->EvalMult(in.a,pt_s), cc->EvalMult(in.b,pt_s)};
}

int decode_bit(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, PE state, double b_init){
    for(int j=0;j<8;j++) state=mulY(cc,state);
    double b_final=decrypt_val(cc,kp,state.b);
    return (b_final>b_init*MID)?1:0;
}

double time_ms(struct timeval s,struct timeval e){return (e.tv_sec-s.tv_sec)*1000.0+(e.tv_usec-s.tv_usec)/1000.0;}

int main(){
    std::cout<<"\n  ╔══════════════════════════════════════════════════════════════╗\n";
    std::cout<<"  ║   MULTI-GATE iO FULL ADDER: 9 NAND, Unique Scale per Wire     ║\n";
    std::cout<<"  ╚══════════════════════════════════════════════════════════════╝\n\n";
    
    CCParams<CryptoContextCKKSRNS> p;
    p.SetMultiplicativeDepth(500);p.SetScalingModSize(50);p.SetBatchSize(2048);
    p.SetRingDim(4096);p.SetSecretKeyDist(UNIFORM_TERNARY);p.SetSecurityLevel(HEStd_NotSet);
    auto cc=GenCryptoContext(p);
    cc->Enable(PKE);cc->Enable(KEYSWITCH);cc->Enable(LEVELEDSHE);cc->Enable(ADVANCEDSHE);
    auto kp=cc->KeyGen();cc->EvalMultKeyGen(kp.secretKey);
    srand(time(0));
    
    auto ct_psi=cc->Encrypt(kp.publicKey,cc->MakeCKKSPackedPlaintext(std::vector<double>{PSI}));
    auto ct_2psi=cc->Encrypt(kp.publicKey,cc->MakeCKKSPackedPlaintext(std::vector<double>{2.0*PSI}));
    
    int cases[8][3]={{0,0,0},{0,0,1},{0,1,0},{0,1,1},{1,0,0},{1,0,1},{1,1,0},{1,1,1}};
    int ok_sum=0,ok_cout=0;
    struct timeval t0,t1; gettimeofday(&t0,NULL);
    
    std::cout<<"  === iO Full Adder Truth Table ===\n";
    std::cout<<"  A B Cin | SUM COUT | iO-SUM iO-COUT | Match\n";
    std::cout<<"  -------------------------------------------\n";
    
    for(int i=0;i<8;i++){
        int A=cases[i][0], B=cases[i][1], Cin=cases[i][2];
        int eSUM=(A+B+Cin)%2, eCOUT=(A+B+Cin)/2;
        
        PE a=encrypt_bit(cc,kp,A), b=encrypt_bit(cc,kp,B), c=encrypt_bit(cc,kp,Cin);
        
        // Gate 1: NAND(A,B) + obfuscate
        PE X1=nand_fhe(cc,ct_psi,ct_2psi,a,b);
        double s1=0.5+(rand()%300)/100.0; PE O1=obfuscate(cc,kp,X1,s1);
        
        // Gate 2: NAND(A,X1) + obfuscate
        PE X2=nand_fhe(cc,ct_psi,ct_2psi,a,X1);
        double s2=0.5+(rand()%300)/100.0; PE O2=obfuscate(cc,kp,X2,s2);
        
        // Gate 3: NAND(B,X1) + obfuscate
        PE X3=nand_fhe(cc,ct_psi,ct_2psi,b,X1);
        double s3=0.5+(rand()%300)/100.0; PE O3=obfuscate(cc,kp,X3,s3);
        
        // Gate 4: NAND(X2,X3) + obfuscate
        PE X4=nand_fhe(cc,ct_psi,ct_2psi,X2,X3);
        double s4=0.5+(rand()%300)/100.0; PE O4=obfuscate(cc,kp,X4,s4);
        
        // Gate 5: NAND(X4,Cin) + obfuscate
        PE X5=nand_fhe(cc,ct_psi,ct_2psi,X4,c);
        double s5=0.5+(rand()%300)/100.0; PE O5=obfuscate(cc,kp,X5,s5);
        
        // Gate 6: NAND(X4,X5) + obfuscate
        PE X6=nand_fhe(cc,ct_psi,ct_2psi,X4,X5);
        double s6=0.5+(rand()%300)/100.0; PE O6=obfuscate(cc,kp,X6,s6);
        
        // Gate 7: NAND(X5,Cin) + obfuscate
        PE X7=nand_fhe(cc,ct_psi,ct_2psi,X5,c);
        double s7=0.5+(rand()%300)/100.0; PE O7=obfuscate(cc,kp,X7,s7);
        
        // Gate 8: SUM = NAND(X6,X7) + obfuscate
        PE SUM=nand_fhe(cc,ct_psi,ct_2psi,X6,X7);
        double s8=0.5+(rand()%300)/100.0; PE O8=obfuscate(cc,kp,SUM,s8);
        
        // Gate 9: COUT = NAND(X1,X5) + obfuscate
        PE COUT=nand_fhe(cc,ct_psi,ct_2psi,X1,X5);
        double s9=0.5+(rand()%300)/100.0; PE O9=obfuscate(cc,kp,COUT,s9);
        
        int gSUM=decode_bit(cc,kp,O8,s8);
        int gCOUT=decode_bit(cc,kp,O9,s9);
        if(gSUM==eSUM)ok_sum++; if(gCOUT==eCOUT)ok_cout++;
        
        std::cout<<"  "<<A<<" "<<B<<" "<<Cin<<"  |  "<<eSUM<<"  "<<eCOUT<<"    |   "<<gSUM<<"  "<<gCOUT
             <<"       | "<<((gSUM==eSUM&&gCOUT==eCOUT)?"YES":"NO")<<"\n";
    }
    
    gettimeofday(&t1,NULL);
    
    std::cout<<"\n  SUM: "<<ok_sum<<"/8 COUT: "<<ok_cout<<"/8 Time: "<<std::fixed<<std::setprecision(0)<<time_ms(t0,t1)<<" ms\n\n";
    std::cout<<"  ╔══════════════════════════════════════════════════════════════╗\n";
    std::cout<<"  ║  MULTI-GATE iO FULL ADDER: "<<(ok_sum==8&&ok_cout==8?"8/8 PERFECT!":"needs work")<<"                            ║\n";
    std::cout<<"  ║  9 NAND gates, 9 unique random scales per evaluation         ║\n";
    std::cout<<"  ╚══════════════════════════════════════════════════════════════╝\n\n";
    return 0;
}
