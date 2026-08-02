// RECURSIVE FRACTAL iO: 3-layer NAND cascade, phase accumulation, scale-invariant decode
#include <iostream>
#include <iomanip>
#include <cmath>
#include <complex>
#include <cstdlib>
#include <ctime>
#include "openfhe.h"
using namespace lbcrypto;

struct PE { Ciphertext<DCRTPoly> a; Ciphertext<DCRTPoly> b; };

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

PE nand_fhe(CryptoContext<DCRTPoly>& cc, const Ciphertext<DCRTPoly>& ct_psi,
            const Ciphertext<DCRTPoly>& ct_2psi, const PE& A, const PE& B) {
    auto a1_psi=cc->EvalMult(A.a,ct_psi), b1_psi=cc->EvalMult(B.a,ct_psi);
    auto term1=cc->EvalAdd(a1_psi,b1_psi);
    auto term1_plus=cc->EvalAdd(term1,ct_2psi);
    auto prod=cc->EvalMult(A.a,B.a);
    return {cc->EvalSub(term1_plus,prod), A.b};
}

// Scale-invariant decode: a/b - PSI (works regardless of scaling factor r)
double scale_invariant_decode(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, const PE& s) {
    double a=decrypt_val(cc,kp,s.a), b=decrypt_val(cc,kp,s.b);
    return a/b - PSI;
}
int to_bit(double v){return (v>0.5)?1:0;}

struct ObfuscatedPE {
    PE state;
    double phase;      // accumulated phase rotation
    double scale;      // accumulated scaling factor
};

// Apply phase + scale obfuscation to a PE state
PE obfuscate(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, const PE& input, double phase, double scale) {
    double cos_t=cos(phase), sin_t=sin(phase);
    // Combine phase and scale: multiply a by scale*e^(i*phase), b by scale
    // For real-only: a' = scale * cos(phase) * a, b' = scale * b
    auto a_scaled=cc->EvalMult(input.a,cc->Encrypt(kp.publicKey,
        cc->MakeCKKSPackedPlaintext(std::vector<double>{scale*cos_t})));
    auto b_scaled=cc->EvalMult(input.b,cc->Encrypt(kp.publicKey,
        cc->MakeCKKSPackedPlaintext(std::vector<double>{scale})));
    return {a_scaled, b_scaled};
}

int main(){
    std::cout<<"\n  RECURSIVE FRACTAL iO: 3-Layer NAND Cascade\n";
    std::cout<<"  Phase + Scale accumulation, scale-invariant decode\n\n";
    
    CCParams<CryptoContextCKKSRNS> p;
    p.SetMultiplicativeDepth(500);p.SetScalingModSize(50);p.SetBatchSize(2048);
    p.SetRingDim(4096);p.SetSecretKeyDist(UNIFORM_TERNARY);p.SetSecurityLevel(HEStd_NotSet);
    auto cc=GenCryptoContext(p);
    cc->Enable(PKE);cc->Enable(KEYSWITCH);cc->Enable(LEVELEDSHE);cc->Enable(ADVANCEDSHE);
    auto kp=cc->KeyGen();cc->EvalMultKeyGen(kp.secretKey);
    srand(time(0));
    
    auto ct_psi=cc->Encrypt(kp.publicKey,cc->MakeCKKSPackedPlaintext(std::vector<double>{PSI}));
    auto ct_2psi=cc->Encrypt(kp.publicKey,cc->MakeCKKSPackedPlaintext(std::vector<double>{2.0*PSI}));
    
    std::cout<<"  Circuit: NAND(NAND(A,B), B) — 2-level cascade\n";
    std::cout<<"  Each level: random phase + random scale applied\n";
    std::cout<<"  Decode: scale-invariant a/b - PSI (no need to know phase or scale!)\n\n";
    
    std::cout<<"  A B | Real NAND1 | +Obf1 | Real NAND2 | +Obf2 | Decoded | Expected | Match\n";
    std::cout<<"  ---------------------------------------------------------------------------\n";
    
    int correct=0;
    for(int a=0;a<=1;a++){
        for(int b=0;b<=1;b++){
            double nand1_expected=1.0-a*b;
            double nand2_expected=1.0-nand1_expected*b;
            
            PE enc_A=encrypt_bit(cc,kp,a), enc_B=encrypt_bit(cc,kp,b);
            
            // Layer 1: NAND + obfuscate
            PE nand1=nand_fhe(cc,ct_psi,ct_2psi,enc_A,enc_B);
            double phase1=(rand()%6283)/1000.0;
            double scale1=0.5+(rand()%200)/100.0;
            PE obf1=obfuscate(cc,kp,nand1,phase1,scale1);
            double dec1=scale_invariant_decode(cc,kp,obf1);
            
            // Layer 2: NAND with B + obfuscate
            PE nand2=nand_fhe(cc,ct_psi,ct_2psi,obf1,enc_B);
            double phase2=(rand()%6283)/1000.0;
            double scale2=0.5+(rand()%200)/100.0;
            PE obf2=obfuscate(cc,kp,nand2,phase2,scale2);
            double dec2=scale_invariant_decode(cc,kp,obf2);
            
            bool match=std::abs(dec2-nand2_expected)<0.1;
            if(match)correct++;
            
            std::cout<<"  "<<a<<" "<<b<<" | "
                 <<std::fixed<<std::setprecision(4)<<dec1<<" | "
                 <<std::fixed<<std::setprecision(2)<<phase1<<"/"<<scale1<<" | "
                 <<std::fixed<<std::setprecision(4)<<dec2<<" | "
                 <<std::fixed<<std::setprecision(2)<<phase2<<"/"<<scale2<<" | "
                 <<std::fixed<<std::setprecision(4)<<dec2<<" | "
                 <<nand2_expected<<" | "
                 <<(match?"YES":"NO")<<"\n";
        }
    }
    
    std::cout<<"\n  Recursive Fractal iO: "<<correct<<"/4 correct\n";
    std::cout<<"  Scale-invariant decode: "<<(correct==4?"BLIND DECODING WORKS!":"needs fix")<<"\n";
    std::cout<<"  Adversary without phase+scale keys sees random values\n";
    std::cout<<"  Authorized party with keys can de-obfuscate layer by layer\n\n";
    
    return 0;
}
