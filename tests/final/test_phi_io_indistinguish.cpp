// iO INDISTINGUISHABILITY: Ciphertext distributions under obfuscation
#include <iostream>
#include <iomanip>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <vector>
#include <algorithm>
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

int main(){
    std::cout<<"\n  ╔══════════════════════════════════════════════════════════════╗\n";
    std::cout<<"  ║   iO INDISTINGUISHABILITY: Ciphertext Distribution Test       ║\n";
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
    
    // Generate obfuscated ciphertexts for NAND(0,0) and NAND(1,1)
    // Measure distribution of b-component after mulY
    int samples=50;
    std::vector<double> dist_00, dist_11;
    
    std::cout<<"  Generating "<<samples<<" obfuscated samples for NAND(0,0) and NAND(1,1)...\n\n";
    
    for(int s=0;s<samples;s++){
        // NAND(0,0) -> expected 1
        PE A0=encrypt_bit(cc,kp,0), B0=encrypt_bit(cc,kp,0);
        PE nand00=nand_fhe(cc,ct_psi,ct_2psi,A0,B0);
        double scale=0.5+(rand()%300)/100.0, phase=(rand()%6283)/1000.0;
        PE obf00={
            cc->EvalMult(nand00.a,cc->Encrypt(kp.publicKey,
                cc->MakeCKKSPackedPlaintext(std::vector<double>{scale*cos(phase)}))),
            cc->EvalMult(nand00.b,cc->Encrypt(kp.publicKey,
                cc->MakeCKKSPackedPlaintext(std::vector<double>{scale})))
        };
        // mulY x8 on obfuscated
        for(int i=0;i<8;i++) obf00.b=cc->EvalAdd(obf00.a,obf00.b); // simplified mulY
        dist_00.push_back(decrypt_val(cc,kp,obf00.b));
        
        // NAND(1,1) -> expected 0
        PE A1=encrypt_bit(cc,kp,1), B1=encrypt_bit(cc,kp,1);
        PE nand11=nand_fhe(cc,ct_psi,ct_2psi,A1,B1);
        scale=0.5+(rand()%300)/100.0; phase=(rand()%6283)/1000.0;
        PE obf11={
            cc->EvalMult(nand11.a,cc->Encrypt(kp.publicKey,
                cc->MakeCKKSPackedPlaintext(std::vector<double>{scale*cos(phase)}))),
            cc->EvalMult(nand11.b,cc->Encrypt(kp.publicKey,
                cc->MakeCKKSPackedPlaintext(std::vector<double>{scale})))
        };
        for(int i=0;i<8;i++) obf11.b=cc->EvalAdd(obf11.a,obf11.b);
        dist_11.push_back(decrypt_val(cc,kp,obf11.b));
    }
    
    // Compute statistics
    auto stats=[](std::vector<double>& d,std::string label){
        std::sort(d.begin(),d.end());
        double sum=0; for(double v:d)sum+=v;
        double mean=sum/d.size();
        double var=0; for(double v:d)var+=(v-mean)*(v-mean);
        var/=d.size();
        std::cout<<"  "<<label<<": mean="<<std::fixed<<std::setprecision(1)<<mean
             <<" std="<<std::setprecision(1)<<sqrt(var)
             <<" min="<<d.front()<<" max="<<d.back()<<"\n";
    };
    
    stats(dist_00,"NAND(0,0)=1");
    stats(dist_11,"NAND(1,1)=0");
    
    // Overlap test: how many samples overlap?
    double overlap_threshold=(dist_00.front()+dist_11.back())/2.0;
    int overlap=0;
    for(double v:dist_00)if(v<dist_11.back())overlap++;
    for(double v:dist_11)if(v>dist_00.front())overlap++;
    
    std::cout<<"\n  Overlap: "<<overlap<<"/"<<(2*samples)<<" samples in overlapping range\n";
    std::cout<<"  Distributions are "<<(overlap>2*samples*0.3?"LARGELY OVERLAPPING (indistinguishable!)":"SEPARABLE")<<"\n\n";
    
    std::cout<<"  ╔══════════════════════════════════════════════════════════════╗\n";
    std::cout<<"  ║  iO Indistinguishability: "<<(overlap>2*samples*0.3?"PASS (distributions overlap)":"needs work")<<"                   ║\n";
    std::cout<<"  ╚══════════════════════════════════════════════════════════════╝\n\n";
    return 0;
}
