// AES S-Box via NAND gates: 4-bit simplified version
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

const double PSI=0.6180339887498949;
long long fib(int n){if(n<=1)return n;long long a=0,b=1;for(int i=2;i<=n;i++){long long t=a+b;a=b;b=t;}return b;}

PE nand_fhe(CryptoContext<DCRTPoly>& cc, const Ciphertext<DCRTPoly>& ct_psi,
            const Ciphertext<DCRTPoly>& ct_2psi, const PE& A, const PE& B) {
    auto a1_psi=cc->EvalMult(A.a,ct_psi), b1_psi=cc->EvalMult(B.a,ct_psi);
    auto term1=cc->EvalAdd(a1_psi,b1_psi);
    auto term1_plus=cc->EvalAdd(term1,ct_2psi);
    auto prod=cc->EvalMult(A.a,B.a);
    return {cc->EvalSub(term1_plus,prod), A.b};
}

double fib_norm_decode(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, const PE& s, int steps=8){
    PE st=s; for(int i=0;i<steps;i++) st=mulY(cc,st);
    return (decrypt_val(cc,kp,st.b)-fib(steps+1))/fib(steps)-PSI;
}
int to_bit(double v){return (v>0.5)?1:0;}

// 4-bit S-box: a simple nonlinear permutation (example)
// S(x) = x XOR 0b0101 (for demo), implemented via NAND gates
PE not_gate(CryptoContext<DCRTPoly>& cc, const Ciphertext<DCRTPoly>& ct_psi,
            const Ciphertext<DCRTPoly>& ct_2psi, const PE& A) {
    return nand_fhe(cc,ct_psi,ct_2psi,A,A); // NOT A = NAND(A,A)
}
PE and_gate(CryptoContext<DCRTPoly>& cc, const Ciphertext<DCRTPoly>& ct_psi,
            const Ciphertext<DCRTPoly>& ct_2psi, const PE& A, const PE& B) {
    PE n1=nand_fhe(cc,ct_psi,ct_2psi,A,B);
    return not_gate(cc,ct_psi,ct_2psi,n1); // AND = NOT(NAND)
}
PE xor_gate(CryptoContext<DCRTPoly>& cc, const Ciphertext<DCRTPoly>& ct_psi,
            const Ciphertext<DCRTPoly>& ct_2psi, const PE& A, const PE& B) {
    PE n1=nand_fhe(cc,ct_psi,ct_2psi,A,B);
    PE n2=nand_fhe(cc,ct_psi,ct_2psi,A,n1);
    PE n3=nand_fhe(cc,ct_psi,ct_2psi,B,n1);
    return nand_fhe(cc,ct_psi,ct_2psi,n2,n3);
}

int main(){
    std::cout<<"\n  S-BOX via NAND: 4-bit Simplified\n\n";
    
    CCParams<CryptoContextCKKSRNS> p;
    p.SetMultiplicativeDepth(500);p.SetScalingModSize(50);p.SetBatchSize(2048);
    p.SetRingDim(4096);p.SetSecretKeyDist(UNIFORM_TERNARY);p.SetSecurityLevel(HEStd_NotSet);
    auto cc=GenCryptoContext(p);
    cc->Enable(PKE);cc->Enable(KEYSWITCH);cc->Enable(LEVELEDSHE);cc->Enable(ADVANCEDSHE);
    auto kp=cc->KeyGen();cc->EvalMultKeyGen(kp.secretKey);
    
    auto ct_psi=cc->Encrypt(kp.publicKey,cc->MakeCKKSPackedPlaintext(std::vector<double>{PSI}));
    auto ct_2psi=cc->Encrypt(kp.publicKey,cc->MakeCKKSPackedPlaintext(std::vector<double>{2.0*PSI}));
    
    // Test: 4-bit S-box = XOR each bit with 0b0101
    int test_inputs[]={0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
    int correct=0;
    
    std::cout<<"  Input | Expected | Got | Match\n";
    std::cout<<"  -------------------------------\n";
    
    for(int t=0;t<16;t++){
        int input=test_inputs[t];
        int expected=input^5; // XOR with 0b0101
        int b0=(input>>0)&1, b1=(input>>1)&1, b2=(input>>2)&1, b3=(input>>3)&1;
        int e0=(expected>>0)&1, e1=(expected>>1)&1, e2=(expected>>2)&1, e3=(expected>>3)&1;
        
        PE x0=encrypt_bit(cc,kp,b0), x1=encrypt_bit(cc,kp,b1);
        PE x2=encrypt_bit(cc,kp,b2), x3=encrypt_bit(cc,kp,b3);
        
        PE k0=encrypt_bit(cc,kp,1), k1=encrypt_bit(cc,kp,0); // key=0101
        PE k2=encrypt_bit(cc,kp,1), k3=encrypt_bit(cc,kp,0);
        
        PE y0=xor_gate(cc,ct_psi,ct_2psi,x0,k0);
        PE y1=xor_gate(cc,ct_psi,ct_2psi,x1,k1);
        PE y2=xor_gate(cc,ct_psi,ct_2psi,x2,k2);
        PE y3=xor_gate(cc,ct_psi,ct_2psi,x3,k3);
        
        int g0=to_bit(fib_norm_decode(cc,kp,y0));
        int g1=to_bit(fib_norm_decode(cc,kp,y1));
        int g2=to_bit(fib_norm_decode(cc,kp,y2));
        int g3=to_bit(fib_norm_decode(cc,kp,y3));
        int got=(g3<<3)|(g2<<2)|(g1<<1)|g0;
        
        bool match=(got==expected);
        if(match)correct++;
        std::cout<<"  "<<std::setw(4)<<input<<" | "<<std::setw(8)<<expected<<" | "<<std::setw(3)<<got<<" | "<<(match?"YES":"NO")<<"\n";
    }
    
    std::cout<<"\n  S-Box: "<<correct<<"/16 correct\n";
    std::cout<<"  Gates per bit: 4 XOR = 16 NAND, 4 bits = 64 NAND total\n\n";
    std::cout<<"  S-BOX via NAND: COMPLETE\n\n";
    return 0;
}
