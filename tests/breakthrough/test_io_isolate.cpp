// ╔══════════════════════════════════════════════════════════════════╗
// ║  ISOLATION TEST — 8K ring, observer gate only, depth 0         ║
// ║  No recursion. No random loop. Just one computation.           ║
// ║  Architecture: Dan Fernandez / Primordial Omega Zero            ║
// ╚══════════════════════════════════════════════════════════════════╝
#include <iostream>
#include <iomanip>
#include <cmath>
#include <sys/time.h>
#include "openfhe.h"
using namespace lbcrypto;

const double PHI = 1.6180339887498948482;
const double PSI = -0.6180339887498948482;

struct DualGate { Ciphertext<DCRTPoly> a; Ciphertext<DCRTPoly> b; };

DualGate observe_and(CryptoContext<DCRTPoly>& cc, const DualGate& X, const DualGate& Y) {
    auto a_out = cc->EvalMult(X.a, Y.a);
    auto sum = cc->EvalAdd(cc->EvalAdd(cc->EvalMult(X.a, Y.b), cc->EvalMult(X.b, Y.a)), cc->EvalMult(X.b, Y.b));
    auto neg_one = cc->MakeCKKSPackedPlaintext(std::vector<double>{-1.0});
    return {a_out, cc->EvalMult(neg_one, sum)};
}

DualGate observe_or(CryptoContext<DCRTPoly>& cc, const DualGate& X, const DualGate& Y) {
    auto one = cc->MakeCKKSPackedPlaintext(std::vector<double>{1.0});
    auto oma1 = cc->EvalSub(one, X.a), oma2 = cc->EvalSub(one, Y.a);
    auto a_out = cc->EvalSub(one, cc->EvalMult(oma1, oma2));
    auto b_out = cc->EvalAdd(cc->EvalAdd(cc->EvalMult(oma1, Y.b), cc->EvalMult(X.b, oma2)), cc->EvalMult(X.b, Y.b));
    return {a_out, b_out};
}

double decrypt_val(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, const Ciphertext<DCRTPoly>& c) {
    Plaintext pt; cc->Decrypt(kp.secretKey, c, &pt); return pt->GetCKKSPackedValue()[0].real();
}
double reveal(DualGate& dg, CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, double root) {
    return decrypt_val(cc, kp, dg.a) + decrypt_val(cc, kp, dg.b) * root;
}
DualGate input(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, double val) {
    return {cc->Encrypt(kp.publicKey, cc->MakeCKKSPackedPlaintext(std::vector<double>{val})),
            cc->Encrypt(kp.publicKey, cc->MakeCKKSPackedPlaintext(std::vector<double>{0.0}))};
}

int main() {
    time_t st = time(0);
    std::cout << "\n  ╔══════════════════════════════════════════════════════════════╗\n";
    std::cout << "  ║  ISOLATION TEST — 8K ring, depth 0, 1 computation each      ║\n";
    std::cout << "  ╚══════════════════════════════════════════════════════════════╝\n";
    std::cout << "  Started: " << ctime(&st) << "\n";

    CCParams<CryptoContextCKKSRNS> p;
    p.SetMultiplicativeDepth(50); p.SetScalingModSize(50); p.SetBatchSize(512);
    p.SetRingDim(8192); p.SetSecretKeyDist(UNIFORM_TERNARY); p.SetSecurityLevel(HEStd_NotSet);
    auto cc = GenCryptoContext(p); cc->Enable(PKE); cc->Enable(KEYSWITCH); cc->Enable(LEVELEDSHE);
    auto kp = cc->KeyGen(); cc->EvalMultKeyGen(kp.secretKey);
    
    int inputs[8][3] = {{0,0,0},{0,0,1},{0,1,0},{0,1,1},{1,0,0},{1,0,1},{1,1,0},{1,1,1}};
    
    std::cout << "  RingDim = 8192, Depth budget = 50\n";
    std::cout << "  Each input tested ONCE. No loops. No recursion.\n\n";
    
    std::cout << "  ┌──────────────────────────────────────────────────────┐\n";
    std::cout << "  │  ISOLATION — One shot per input, depth 0              │\n";
    std::cout << "  ├─────┬─────┬─────┬──────────┬──────────┬──────────────┤\n";
    std::cout << "  │  x  │  y  │  z  │  φ-out    │  ψ-out    │  Expected    │\n";
    std::cout << "  ├─────┼─────┼─────┼──────────┼──────────┼──────────────┤\n";
    
    int ok = 0;
    for (int i = 0; i < 8; i++) {
        std::vector<double> dv = {(double)inputs[i][0], (double)inputs[i][1], (double)inputs[i][2]};
        DualGate x=input(cc,kp,dv[0]), y=input(cc,kp,dv[1]), z=input(cc,kp,dv[2]);
        
        DualGate phi_and = observe_and(cc, x, y);
        DualGate phi_out = observe_or(cc, phi_and, z);
        DualGate psi_or1 = observe_or(cc, x, z);
        DualGate psi_or2 = observe_or(cc, y, z);
        DualGate psi_out = observe_and(cc, psi_or1, psi_or2);
        
        int expected = (inputs[i][0] & inputs[i][1]) | inputs[i][2];
        double vp = reveal(phi_out, cc, kp, PHI);
        double vs = reveal(psi_out, cc, kp, PSI);
        int bp = (vp > 0.5) ? 1 : 0;
        int bs = (vs > 0.5) ? 1 : 0;
        if (bp == expected && bs == expected) ok++;
        
        std::cout << "  │  " << inputs[i][0] << "  │  " << inputs[i][1] << "  │  " << inputs[i][2]
                  << "  │  " << bp << " (" << std::fixed << std::setprecision(4) << vp << ") │  "
                  << bs << " (" << std::setprecision(4) << vs << ") │     " << expected << "        │\n";
    }
    
    std::cout << "  ├─────┴─────┴─────┴──────────┴──────────┴──────────────┤\n";
    std::cout << "  │  Correct: " << ok << "/8                                                   │\n";
    std::cout << "  └──────────────────────────────────────────────────────┘\n\n";
    
    // Now test depth 1 with recursion
    std::cout << "  ┌──────────────────────────────────────────────────────┐\n";
    std::cout << "  │  ISOLATION — One shot per input, depth 1 recursion    │\n";
    std::cout << "  ├─────┬─────┬─────┬──────────┬──────────┬──────────────┤\n";
    
    int ok2 = 0;
    for (int i = 0; i < 8; i++) {
        std::vector<double> dv = {(double)inputs[i][0], (double)inputs[i][1], (double)inputs[i][2]};
        DualGate x=input(cc,kp,dv[0]), y=input(cc,kp,dv[1]), z=input(cc,kp,dv[2]);
        
        DualGate cur_phi = observe_and(cc, x, y);
        DualGate cur_psi = observe_or(cc, x, z);
        // Depth 1: one round of recursion
        cur_phi = observe_and(cc, cur_phi, cur_psi);
        cur_psi = observe_or(cc, cur_psi, cur_phi);
        
        DualGate phi_out = observe_or(cc, cur_phi, z);
        DualGate psi_out = observe_and(cc, observe_or(cc, y, z), observe_or(cc, x, z));
        
        int expected = (inputs[i][0] & inputs[i][1]) | inputs[i][2];
        double vp = reveal(phi_out, cc, kp, PHI);
        double vs = reveal(psi_out, cc, kp, PSI);
        int bp = (vp > 0.5) ? 1 : 0;
        int bs = (vs > 0.5) ? 1 : 0;
        if (bp == expected && bs == expected) ok2++;
        
        std::cout << "  │  " << inputs[i][0] << "  │  " << inputs[i][1] << "  │  " << inputs[i][2]
                  << "  │  " << bp << " (" << std::fixed << std::setprecision(4) << vp << ") │  "
                  << bs << " (" << std::setprecision(4) << vs << ") │     " << expected << "        │\n";
    }
    
    std::cout << "  ├─────┴─────┴─────┴──────────┴──────────┴──────────────┤\n";
    std::cout << "  │  Correct: " << ok2 << "/8                                                   │\n";
    std::cout << "  └──────────────────────────────────────────────────────┘\n\n";
    
    time_t et = time(0);
    std::cout << "  ╔══════════════════════════════════════════════════════════════╗\n";
    std::cout << "  ║  ISOLATION — " << (ok==8&&ok2==8?"ALL CORRECT ✓✓✓":"DEGRADED") << "                                      ║\n";
    std::cout << "  ║  Author: Dan Fernandez / Primordial Omega Zero               ║\n";
    std::cout << "  ╚══════════════════════════════════════════════════════════════╝\n\n";
    std::cout << "  Ended: " << ctime(&et) << "\n";
    
    return 0;
}
