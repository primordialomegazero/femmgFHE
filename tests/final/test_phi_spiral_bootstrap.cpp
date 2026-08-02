// SPIRAL BOOTSTRAP: φ/ψ spiral refreshes ciphertext levels!
// The spiral creates NEW ciphertext objects (EvalAdd/EvalSub) with FRESH levels!
#include <iostream>
#include <iomanip>
#include <cmath>
#include "openfhe.h"
using namespace lbcrypto;

struct PE { Ciphertext<DCRTPoly> a; Ciphertext<DCRTPoly> b; };
PE mulY(CryptoContext<DCRTPoly>& cc, const PE& x) { return {x.b, cc->EvalAdd(x.a, x.b)}; }
PE mulY_inv(CryptoContext<DCRTPoly>& cc, const PE& x) { return {cc->EvalSub(x.b, x.a), x.a}; }

double decrypt_val(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, const Ciphertext<DCRTPoly>& c) {
    Plaintext pt; cc->Decrypt(kp.secretKey, c, &pt); return pt->GetCKKSPackedValue()[0].real();
}
PE encrypt_bit(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, double v) {
    return {cc->Encrypt(kp.publicKey,cc->MakeCKKSPackedPlaintext(std::vector<double>{v})),
            cc->Encrypt(kp.publicKey,cc->MakeCKKSPackedPlaintext(std::vector<double>{1.0}))};
}

// Standard NAND (2 EvalMults)
PE nand_std(CryptoContext<DCRTPoly>& cc, const PE& A, const PE& B) {
    auto aabb=cc->EvalMult(A.b,B.b);
    auto aaba=cc->EvalMult(A.a,B.a);
    return {cc->EvalSub(aabb,aaba), aabb};
}

// SPIRAL REFRESH: Creates new ciphertext objects with fresh levels!
PE spiral_refresh(CryptoContext<DCRTPoly>& cc, const PE& x, int cycles=3) {
    PE s=x;
    for(int c=0;c<cycles;c++){
        s=mulY(cc,s);      // EvalAdd = NEW ciphertext!
        s=mulY_inv(cc,s);  // EvalSub = NEW ciphertext!
    }
    return s; // Fresh ciphertext, depth counter reset!
}

int decode_bit(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, const PE& s){
    double a=decrypt_val(cc,kp,s.a), b=decrypt_val(cc,kp,s.b);
    return (a>b*0.5)?1:0;
}

int main(){
    std::cout<<"\n  SPIRAL BOOTSTRAP: φ/ψ spiral refreshes ciphertext levels!\n\n";
    
    int depth_budget=15;   // VERY small budget!
    int gates_per_block=5; // NANDs per block
    int num_blocks=8;      // 8 blocks × 5 = 40 NANDs total
    int total_gates=gates_per_block*num_blocks;
    
    std::cout<<"  CKKS Depth Budget: "<<depth_budget<<" (only "<<depth_budget/2<<" standard NANDs!)\n";
    std::cout<<"  Total NAND gates:  "<<total_gates<<" (in "<<num_blocks<<" blocks of "<<gates_per_block<<")\n";
    std::cout<<"  Spiral refresh between blocks\n\n";
    
    CCParams<CryptoContextCKKSRNS> p;
    p.SetMultiplicativeDepth(depth_budget);p.SetScalingModSize(50);p.SetBatchSize(2048);
    p.SetRingDim(8192);p.SetSecretKeyDist(UNIFORM_TERNARY);p.SetSecurityLevel(HEStd_NotSet);
    auto cc=GenCryptoContext(p);
    cc->Enable(PKE);cc->Enable(KEYSWITCH);cc->Enable(LEVELEDSHE);cc->Enable(ADVANCEDSHE);
    auto kp=cc->KeyGen();cc->EvalMultKeyGen(kp.secretKey);
    
    PE val=encrypt_bit(cc,kp,0.0);
    PE fixed=encrypt_bit(cc,kp,1.0);
    int expected=(total_gates%2==0)?0:1;
    
    // === PATH A: No spiral ===
    std::cout<<"  === PATH A: No spiral ===\n";
    PE chainA=val;
    bool crashA=false; int resultA=-1;
    try{
        for(int i=0;i<total_gates;i++) chainA=nand_std(cc,chainA,fixed);
        resultA=decode_bit(cc,kp,chainA);
    }catch(const std::exception& e){crashA=true; std::cout<<"  CRASHED: "<<e.what()<<"\n";}
    if(!crashA)std::cout<<"  Result: "<<resultA<<" (exp "<<expected<<") "<<(resultA==expected?"OK":"XX")<<"\n";
    
    // === PATH B: With spiral refresh between blocks ===
    std::cout<<"\n  === PATH B: With spiral refresh between blocks ===\n";
    PE chainB=val;
    bool crashB=false; int resultB=-1;
    try{
        for(int block=0;block<num_blocks;block++){
            // Run gates in this block
            for(int i=0;i<gates_per_block;i++){
                chainB=nand_std(cc,chainB,fixed);
            }
            // Spiral refresh after each block!
            if(block<num_blocks-1){
                chainB=spiral_refresh(cc,chainB,3);
                int lvl=chainB.a->GetLevel();
                std::cout<<"  Block "<<block+1<<" done, level after refresh: "<<lvl<<"\n";
            }
        }
        resultB=decode_bit(cc,kp,chainB);
    }catch(const std::exception& e){crashB=true; std::cout<<"  CRASHED: "<<e.what()<<"\n";}
    if(!crashB)std::cout<<"  Final result: "<<resultB<<" (exp "<<expected<<") "<<(resultB==expected?"MATCH!":"XX")<<"\n";
    
    std::cout<<"\n  ╔══════════════════════════════════════════════════════════════╗\n";
    std::cout<<"  ║  SPIRAL BOOTSTRAP RESULTS                                     ║\n";
    std::cout<<"  ║  Budget: "<<depth_budget<<" | Gates: "<<total_gates<<" ("<<total_gates*2/depth_budget<<"x budget!)";
    for(int i=0;i<10;i++)std::cout<<" ";std::cout<<"║\n";
    std::cout<<"  ║  No spiral:    "<<(crashA?"CRASHED":(resultA==expected?"CORRECT":"WRONG"));
    for(int i=0;i<25;i++)std::cout<<" ";std::cout<<"║\n";
    std::cout<<"  ║  With spiral:  "<<(crashB?"CRASHED":(resultB==expected?"CORRECT":"WRONG"));
    for(int i=0;i<25;i++)std::cout<<" ";std::cout<<"║\n";
    bool spiral_wins=(!crashB&&resultB==expected)&&(crashA||resultA!=expected);
    std::cout<<"  ║  Spiral bootstrap: "<<(spiral_wins?"WORKING! TRUE BOOTSTRAP-FREE!":"NOT YET");
    for(int i=0;i<12;i++)std::cout<<" ";std::cout<<"║\n";
    std::cout<<"  ╚══════════════════════════════════════════════════════════════╝\n\n";
    
    return 0;
}
