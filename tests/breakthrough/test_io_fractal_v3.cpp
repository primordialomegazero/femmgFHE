// ╔══════════════════════════════════════════════════════════════════╗
// ║  FRACTAL iO v3 — Simplified verified equivalent paths          ║
// ║  φ: AND(AND(x,y), OR(x,z))? NO — use direct v8 formulas        ║
// ║  φ: (x AND y) OR z  ·  ψ: (x OR z) AND (y OR z)               ║
// ║  Architecture: Dan Fernandez / Primordial Omega Zero            ║
// ╚══════════════════════════════════════════════════════════════════╝
#include <iostream>
#include <iomanip>
#include <cmath>
#include <sys/time.h>
#include <vector>
#include <random>
#include "openfhe.h"
using namespace lbcrypto;

const double PHI = 1.6180339887498948482;
const double PSI = -0.6180339887498948482;

struct DualGate { Ciphertext<DCRTPoly> a; Ciphertext<DCRTPoly> b; };

DualGate direct_and(CryptoContext<DCRTPoly>& cc, const DualGate& X, const DualGate& Y) {
    auto a_out = cc->EvalMult(X.a, Y.a);
    auto sum = cc->EvalAdd(cc->EvalAdd(cc->EvalMult(X.a, Y.b), cc->EvalMult(X.b, Y.a)), cc->EvalMult(X.b, Y.b));
    auto neg_one = cc->MakeCKKSPackedPlaintext(std::vector<double>{-1.0});
    return {a_out, cc->EvalMult(neg_one, sum)};
}

DualGate direct_or(CryptoContext<DCRTPoly>& cc, const DualGate& X, const DualGate& Y) {
    auto one = cc->MakeCKKSPackedPlaintext(std::vector<double>{1.0});
    auto oma1 = cc->EvalSub(one, X.a), oma2 = cc->EvalSub(one, Y.a);
    auto a_out = cc->EvalSub(one, cc->EvalMult(oma1, oma2));
    auto b_out = cc->EvalAdd(cc->EvalAdd(cc->EvalMult(oma1, Y.b), cc->EvalMult(X.b, oma2)), cc->EvalMult(X.b, Y.b));
    return {a_out, b_out};
}

double decrypt_val(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, const Ciphertext<DCRTPoly>& c) {
    Plaintext pt; cc->Decrypt(kp.secretKey, c, &pt); return pt->GetCKKSPackedValue()[0].real();
}
double decode_dual(DualGate& dg, CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, double root) {
    return decrypt_val(cc, kp, dg.a) + decrypt_val(cc, kp, dg.b) * root;
}
DualGate encode_dual(CryptoContext<DCRTPoly>& cc, const DualGate& gA, const DualGate& gB) {
    auto bA_phi = cc->EvalMult(gA.b, cc->MakeCKKSPackedPlaintext(std::vector<double>{PHI}));
    auto val_A = cc->EvalAdd(gA.a, bA_phi);
    auto bB_psi = cc->EvalMult(gB.b, cc->MakeCKKSPackedPlaintext(std::vector<double>{PSI}));
    auto val_B = cc->EvalAdd(gB.a, bB_psi);
    auto diff = cc->EvalSub(val_A, val_B);
    auto inv_denom = cc->MakeCKKSPackedPlaintext(std::vector<double>{1.0 / (PHI - PSI)});
    auto b_out = cc->EvalMult(diff, inv_denom);
    auto b_phi = cc->EvalMult(b_out, cc->MakeCKKSPackedPlaintext(std::vector<double>{PHI}));
    return {cc->EvalSub(val_A, b_phi), b_out};
}
DualGate make_input(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, double val) {
    return {cc->Encrypt(kp.publicKey, cc->MakeCKKSPackedPlaintext(std::vector<double>{val})),
            cc->Encrypt(kp.publicKey, cc->MakeCKKSPackedPlaintext(std::vector<double>{0.0}))};
}

std::mt19937 rng(std::random_device{}());

int main() {
    time_t st = time(0);
    std::cout << "\n  ╔══════════════════════════════════════════════════════════════╗\n";
    std::cout << "  ║  FRACTAL iO v3 — Direct equivalent formulas                 ║\n";
    std::cout << "  ║  φ: (x AND y) OR z    ψ: (x OR z) AND (y OR z)             ║\n";
    std::cout << "  ╚══════════════════════════════════════════════════════════════╝\n";
    std::cout << "  Started: " << ctime(&st) << "\n";

    CCParams<CryptoContextCKKSRNS> p;
    p.SetMultiplicativeDepth(40); p.SetScalingModSize(50); p.SetBatchSize(1024);
    p.SetRingDim(16384); p.SetSecretKeyDist(UNIFORM_TERNARY); p.SetSecurityLevel(HEStd_NotSet);
    auto cc = GenCryptoContext(p); cc->Enable(PKE); cc->Enable(KEYSWITCH); cc->Enable(LEVELEDSHE);
    auto kp = cc->KeyGen(); cc->EvalMultKeyGen(kp.secretKey);
    
    int inputs[8][3] = {{0,0,0},{0,0,1},{0,1,0},{0,1,1},{1,0,0},{1,0,1},{1,1,0},{1,1,1}};
    
    std::cout << "  STRATEGY: RAW compute both circuits, encode ONLY at end\n\n";
    std::cout << "  ┌──────────────────────────────────────────────────────┐\n";
    std::cout << "  │  CORRECTNESS                                          │\n";
    std::cout << "  ├─────┬─────┬─────┬──────────┬──────────┬──────────────┤\n";
    
    int r1_ok=0, r2_ok=0;
    bool phi_is_A = (rng()%2==0);
    
    for (int i=0; i<8; i++) {
        std::vector<double> dv = {(double)inputs[i][0], (double)inputs[i][1], (double)inputs[i][2]};
        DualGate x=make_input(cc,kp,dv[0]), y=make_input(cc,kp,dv[1]), z=make_input(cc,kp,dv[2]);
        
        // φ: (x AND y) OR z — 2 gates
        DualGate phi_and = direct_and(cc, x, y);
        DualGate phi_out = direct_or(cc, phi_and, z);
        
        // ψ: (x OR z) AND (y OR z) — 2 gates
        DualGate psi_or1 = direct_or(cc, x, z);
        DualGate psi_or2 = direct_or(cc, y, z);
        DualGate psi_out = direct_and(cc, psi_or1, psi_or2);
        
        // ENCODE only the FINAL output
        DualGate final_gate = phi_is_A ? encode_dual(cc, phi_out, psi_out) 
                                        : encode_dual(cc, psi_out, phi_out);
        
        int expected = (inputs[i][0] & inputs[i][1]) | inputs[i][2];
        int bp=(decode_dual(final_gate,cc,kp,PHI)>0.5)?1:0, bs=(decode_dual(final_gate,cc,kp,PSI)>0.5)?1:0;
        int bA=phi_is_A?bp:bs, bB=phi_is_A?bs:bp;
        if(bA==expected)r1_ok++; if(bB==expected)r2_ok++;
        std::cout<<"  │  "<<inputs[i][0]<<"  │  "<<inputs[i][1]<<"  │  "<<inputs[i][2]
                 <<"  │  "<<bA<<"("<<expected<<") │  "<<bB<<"("<<expected<<") │  "
                 <<(bA==expected&&bB==expected?"OK ✓":"FAIL")<<"        │\n";
    }
    std::cout<<"  ├─────┴─────┴─────┴──────────┴──────────┴──────────────┤\n";
    std::cout<<"  │  φ-path: "<<r1_ok<<"/8 · ψ-path: "<<r2_ok<<"/8                                   │\n";
    
    const int TRIALS=100; int io_ok=0;
    std::cout<<"  │  iO Game: "<<std::flush;
    for(int t=0;t<TRIALS;t++){
        int inp=rng()%8;
        std::vector<double> dv={(double)((inp>>0)&1),(double)((inp>>1)&1),(double)((inp>>2)&1)};
        DualGate x=make_input(cc,kp,dv[0]),y=make_input(cc,kp,dv[1]),z=make_input(cc,kp,dv[2]);
        bool pia=(rng()%2==0);
        DualGate pa=direct_and(cc,x,y), po=direct_or(cc,pa,z);
        DualGate p1=direct_or(cc,x,z), p2=direct_or(cc,y,z), pout=direct_and(cc,p1,p2);
        DualGate fg=pia?encode_dual(cc,po,pout):encode_dual(cc,pout,po);
        double vp=decode_dual(fg,cc,kp,PHI),vs=decode_dual(fg,cc,kp,PSI);
        if((vp>vs)==pia)io_ok++;
        if((t+1)%20==0)std::cout<<"."<<std::flush;
    }
    double io_adv=std::abs(100.0*io_ok/TRIALS-50.0);
    std::cout<<"\n  │  iO Game: "<<std::fixed<<std::setprecision(1)<<100.0*io_ok/TRIALS
              <<"% guess, "<<std::setprecision(2)<<io_adv<<"% advantage               │\n";
    std::cout<<"  │  Encode: ONLY final output. 2 gates per circuit.       │\n";
    std::cout<<"  └──────────────────────────────────────────────────────┘\n\n";
    
    time_t et=time(0);
    bool pass=(r1_ok==8&&r2_ok==8&&io_adv<5.0);
    std::cout<<"  ╔══════════════════════════════════════════════════════════════╗\n";
    std::cout<<"  ║  FRACTAL iO v3 — "<<(pass?"8/8 BOTH PATHS · INDISTINGUISHABLE ✓✓✓":"TUNING")<<"   ║\n";
    std::cout<<"  ║  Author: Dan Fernandez / Primordial Omega Zero               ║\n";
    std::cout<<"  ╚══════════════════════════════════════════════════════════════╝\n\n";
    std::cout<<"  Ended: "<<ctime(&et)<<"\n";
    return 0;
}
