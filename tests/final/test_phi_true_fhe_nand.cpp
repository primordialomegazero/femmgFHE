// TRUE FHE NAND: Homomorphic on states, no decrypt, no ratio extraction
#include <iostream>
#include <iomanip>
#include <cmath>
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
    auto a1_psi = cc->EvalMult(A.a, ct_psi);
    auto b1_psi = cc->EvalMult(B.a, ct_psi);
    auto term1 = cc->EvalAdd(a1_psi, b1_psi);
    auto term1_plus = cc->EvalAdd(term1, ct_2psi);
    auto prod = cc->EvalMult(A.a, B.a);
    auto nand_a = cc->EvalSub(term1_plus, prod);
    return {nand_a, A.b};
}

double fib_normalize_decode(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, 
                            const PE& state, int steps=8) {
    PE s=state;
    for(int i=0;i<steps;i++) s=mulY(cc,s);
    double b_val=decrypt_val(cc,kp,s.b);
    return (b_val-fib(steps+1))/fib(steps)-PSI;
}

int to_bit(double v){return (v>0.5)?1:0;}

int main(){
    std::cout<<"\n  TRUE FHE NAND: Homomorphic on States, No Decrypt\n\n";
    
    CCParams<CryptoContextCKKSRNS> p;
    p.SetMultiplicativeDepth(200);p.SetScalingModSize(50);p.SetBatchSize(2048);
    p.SetRingDim(4096);p.SetSecretKeyDist(UNIFORM_TERNARY);p.SetSecurityLevel(HEStd_NotSet);
    auto cc=GenCryptoContext(p);
    cc->Enable(PKE);cc->Enable(KEYSWITCH);cc->Enable(LEVELEDSHE);cc->Enable(ADVANCEDSHE);
    auto kp=cc->KeyGen();cc->EvalMultKeyGen(kp.secretKey);
    
    auto ct_psi=cc->Encrypt(kp.publicKey,cc->MakeCKKSPackedPlaintext(std::vector<double>{PSI}));
    auto ct_2psi=cc->Encrypt(kp.publicKey,cc->MakeCKKSPackedPlaintext(std::vector<double>{2.0*PSI}));
    
    std::cout<<"  NAND via FHE state ops (no decrypt):\n";
    std::cout<<"  A B | Decoded | Expected | Error\n";
    std::cout<<"  -----------------------------------\n";
    for(int a=0;a<=1;a++){
        for(int b=0;b<=1;b++){
            PE enc_A=encrypt_bit(cc,kp,a), enc_B=encrypt_bit(cc,kp,b);
            PE enc_NAND=nand_fhe(cc,ct_psi,ct_2psi,enc_A,enc_B);
            double dec=decode_state(cc,kp,enc_NAND);
            double exp=1.0-a*b;
            std::cout<<"  "<<a<<" "<<b<<" | "<<std::fixed<<std::setprecision(8)<<dec
                 <<" | "<<exp<<"      | "<<std::scientific<<std::setprecision(1)<<std::abs(dec-exp)<<"\n";
        }
    }
    
    std::cout<<"\n  After Fibonacci normalize (8 mulY steps):\n";
    std::cout<<"  A B | Normalized | Expected | Error\n";
    std::cout<<"  -------------------------------------\n";
    for(int a=0;a<=1;a++){
        for(int b=0;b<=1;b++){
            PE enc_A=encrypt_bit(cc,kp,a), enc_B=encrypt_bit(cc,kp,b);
            PE enc_NAND=nand_fhe(cc,ct_psi,ct_2psi,enc_A,enc_B);
            double norm=fib_normalize_decode(cc,kp,enc_NAND,8);
            double exp=1.0-a*b;
            std::cout<<"  "<<a<<" "<<b<<" | "<<std::fixed<<std::setprecision(8)<<norm
                 <<" | "<<exp<<"      | "<<std::scientific<<std::setprecision(1)<<std::abs(norm-exp)<<"\n";
        }
    }
    
    std::cout<<"\n  TRUE FHE NAND: COMPLETE\n\n";
    return 0;
}
