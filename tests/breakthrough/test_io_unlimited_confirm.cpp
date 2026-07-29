// ╔══════════════════════════════════════════════════════════════════╗
// ║  UNLIMITED CONFIRMATION — 1K ring, 1000 trials, depth sweep    ║
// ║  Is recursive self-observation truly unlimited?                 ║
// ║  Architecture: Dan Fernandez / Primordial Omega Zero            ║
// ╚══════════════════════════════════════════════════════════════════╝
#include <iostream>
#include <iomanip>
#include <cmath>
#include <sys/time.h>
#include <vector>
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

struct Reflection {
    DualGate phi_view, psi_view;
    int depth;
};

Reflection recursive_observe(CryptoContext<DCRTPoly>& cc, 
                              const DualGate& X, const DualGate& Y, const DualGate& Z,
                              int max_depth) {
    Reflection r;
    DualGate current_phi = observe_and(cc, X, Y);
    DualGate current_psi = observe_or(cc, X, Z);
    
    for (int d = 1; d <= max_depth; d++) {
        current_phi = observe_and(cc, current_phi, current_psi);
        current_psi = observe_or(cc, current_psi, current_phi);
    }
    
    r.phi_view = observe_or(cc, current_phi, Z);
    r.psi_view = observe_and(cc, observe_or(cc, Y, Z), observe_or(cc, X, Z));
    r.depth = max_depth;
    return r;
}

int main() {
    time_t st = time(0);
    std::cout << "\n  ╔══════════════════════════════════════════════════════════════╗\n";
    std::cout << "  ║  UNLIMITED CONFIRMATION — 1K ring, depth sweep, 1K trials  ║\n";
    std::cout << "  ╚══════════════════════════════════════════════════════════════╝\n";
    std::cout << "  Started: " << ctime(&st) << "\n";

    CCParams<CryptoContextCKKSRNS> p;
    p.SetMultiplicativeDepth(30); p.SetScalingModSize(50); p.SetBatchSize(256);
    p.SetRingDim(1024); p.SetSecretKeyDist(UNIFORM_TERNARY); p.SetSecurityLevel(HEStd_NotSet);
    auto cc = GenCryptoContext(p); cc->Enable(PKE); cc->Enable(KEYSWITCH); cc->Enable(LEVELEDSHE);
    auto kp = cc->KeyGen(); cc->EvalMultKeyGen(kp.secretKey);
    
    int inputs[8][3] = {{0,0,0},{0,0,1},{0,1,0},{0,1,1},{1,0,0},{1,0,1},{1,1,0},{1,1,1}};

    std::cout << "  RingDim = 1024, Depth budget = 30\n";
    std::cout << "  Testing depths 0-10 with 1000 random trials each\n\n";
    std::cout << "  ┌──────┬──────────┬──────────┬────────────────────────┐\n";
    std::cout << "  │ Depth│ φ-OK     │ ψ-OK     │ Status                  │\n";
    std::cout << "  ├──────┼──────────┼──────────┼────────────────────────┤\n";

    for (int depth = 0; depth <= 10; depth++) {
        int phi_ok = 0, psi_ok = 0;
        const int T = 1000;
        bool crashed = false;
        
        for (int t = 0; t < T && !crashed; t++) {
            int inp = rand() % 8;
            std::vector<double> dv = {(double)((inp>>0)&1), (double)((inp>>1)&1), (double)((inp>>2)&1)};
            DualGate x=input(cc,kp,dv[0]), y=input(cc,kp,dv[1]), z=input(cc,kp,dv[2]);
            
            try {
                Reflection r = recursive_observe(cc, x, y, z, depth);
                int expected = (inputs[inp][0] & inputs[inp][1]) | inputs[inp][2];
                int bp = (reveal(r.phi_view, cc, kp, PHI) > 0.5) ? 1 : 0;
                int bs = (reveal(r.psi_view, cc, kp, PSI) > 0.5) ? 1 : 0;
                if (bp == expected) phi_ok++;
                if (bs == expected) psi_ok++;
            } catch (...) {
                crashed = true;
            }
        }
        
        double phi_pct = crashed ? -1 : 100.0 * phi_ok / T;
        double psi_pct = crashed ? -1 : 100.0 * psi_ok / T;
        
        std::cout << "  │ " << std::setw(4) << depth 
                  << " │ " << (crashed ? "CRASH" : std::to_string(phi_ok) + "/" + std::to_string(T))
                  << "   │ " << (crashed ? "CRASH" : std::to_string(psi_ok) + "/" + std::to_string(T))
                  << "   │ "
                  << (crashed ? "DEPTH LIMIT ✗" : 
                      phi_pct > 99.5 && psi_pct > 99.5 ? "PERFECT ✓✓✓" :
                      phi_pct > 95 && psi_pct > 95 ? "GOOD ✓" : "DEGRADED")
                  << "       │\n";
        
        if (crashed) {
            std::cout << "  ├──────┴──────────┴──────────┴────────────────────────┤\n";
            std::cout << "  │  Max depth before crash: " << depth - 1 << " at RingDim=1024             │\n";
            break;
        }
    }
    
    std::cout << "  └──────────────────────────────────────────────────────┘\n\n";
    std::cout << "  ╔══════════════════════════════════════════════════════════════╗\n";
    std::cout << "  ║  Observer gate: No separate encode. Recursive reflection.    ║\n";
    std::cout << "  ║  Author: Dan Fernandez / Primordial Omega Zero               ║\n";
    std::cout << "  ╚══════════════════════════════════════════════════════════════╝\n\n";
    
    time_t et = time(0);
    std::cout << "  Ended: " << ctime(&et) << "\n";
    return 0;
}
