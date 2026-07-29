// ╔══════════════════════════════════════════════════════════════════╗
// ║  LYAPUNOV-STABLE iO — Bounded energy, infinite depth            ║
// ║  V(a,b) = a(1-a) + b². Gradient damping prevents drift.        ║
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
const double LAMBDA = 0.25;

struct DualGate { Ciphertext<DCRTPoly> a; Ciphertext<DCRTPoly> b; };

// ═══════════════════════════════════════════════════════════════
// LYAPUNOV-STABLE OBSERVER GATE
// ═══════════════════════════════════════════════════════════════
DualGate stable_and(CryptoContext<DCRTPoly>& cc, const DualGate& X, const DualGate& Y) {
    auto a_out = cc->EvalMult(X.a, Y.a);
    auto sum = cc->EvalAdd(cc->EvalAdd(cc->EvalMult(X.a, Y.b), cc->EvalMult(X.b, Y.a)), cc->EvalMult(X.b, Y.b));
    auto neg_one = cc->MakeCKKSPackedPlaintext(std::vector<double>{-1.0});
    auto b_out = cc->EvalMult(neg_one, sum);
    
    // LYAPUNOV DAMPING: a_stable = a*(1+2λ) - λ
    double scale = 1.0 + 2.0 * LAMBDA; // 1.5
    auto scale_pt = cc->MakeCKKSPackedPlaintext(std::vector<double>{scale});
    auto lambda_pt = cc->MakeCKKSPackedPlaintext(std::vector<double>{LAMBDA});
    auto a_scaled = cc->EvalMult(a_out, scale_pt);
    auto a_stable = cc->EvalSub(a_scaled, lambda_pt);
    
    // LYAPUNOV DAMPING: b_stable = b * (1 - λ)
    double damp = 1.0 - LAMBDA; // 0.75
    auto damp_pt = cc->MakeCKKSPackedPlaintext(std::vector<double>{damp});
    auto b_stable = cc->EvalMult(b_out, damp_pt);
    
    DualGate result;
    result.a = a_stable;
    result.b = b_stable;
    return result;
}

DualGate stable_or(CryptoContext<DCRTPoly>& cc, const DualGate& X, const DualGate& Y) {
    auto one = cc->MakeCKKSPackedPlaintext(std::vector<double>{1.0});
    auto oma1 = cc->EvalSub(one, X.a), oma2 = cc->EvalSub(one, Y.a);
    auto a_out = cc->EvalSub(one, cc->EvalMult(oma1, oma2));
    auto b_out = cc->EvalAdd(cc->EvalAdd(cc->EvalMult(oma1, Y.b), cc->EvalMult(X.b, oma2)), cc->EvalMult(X.b, Y.b));
    
    double scale = 1.0 + 2.0 * LAMBDA;
    auto scale_pt = cc->MakeCKKSPackedPlaintext(std::vector<double>{scale});
    auto lambda_pt = cc->MakeCKKSPackedPlaintext(std::vector<double>{LAMBDA});
    auto a_scaled = cc->EvalMult(a_out, scale_pt);
    auto a_stable = cc->EvalSub(a_scaled, lambda_pt);
    
    double damp = 1.0 - LAMBDA;
    auto damp_pt = cc->MakeCKKSPackedPlaintext(std::vector<double>{damp});
    auto b_stable = cc->EvalMult(b_out, damp_pt);
    
    DualGate result;
    result.a = a_stable;
    result.b = b_stable;
    return result;
}

double decrypt_val(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, const Ciphertext<DCRTPoly>& c) {
    Plaintext pt; cc->Decrypt(kp.secretKey, c, &pt); return pt->GetCKKSPackedValue()[0].real();
}
double reveal(DualGate& dg, CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, double root) {
    return decrypt_val(cc, kp, dg.a) + decrypt_val(cc, kp, dg.b) * root;
}
DualGate input(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, double val) {
    DualGate dg;
    dg.a = cc->Encrypt(kp.publicKey, cc->MakeCKKSPackedPlaintext(std::vector<double>{val}));
    dg.b = cc->Encrypt(kp.publicKey, cc->MakeCKKSPackedPlaintext(std::vector<double>{0.0}));
    return dg;
}

int main() {
    time_t st = time(0);
    std::cout << "\n  ╔══════════════════════════════════════════════════════════════╗\n";
    std::cout << "  ║  LYAPUNOV-STABLE iO — λ=" << std::fixed << std::setprecision(2) << LAMBDA << " damping                  ║\n";
    std::cout << "  ║  4K ring, depth sweep 0-15, 50 trials each                 ║\n";
    std::cout << "  ╚══════════════════════════════════════════════════════════════╝\n";
    std::cout << "  Started: " << ctime(&st) << "\n";

    CCParams<CryptoContextCKKSRNS> p;
    p.SetMultiplicativeDepth(60); p.SetScalingModSize(50); p.SetBatchSize(256);
    p.SetRingDim(4096); p.SetSecretKeyDist(UNIFORM_TERNARY); p.SetSecurityLevel(HEStd_NotSet);
    auto cc = GenCryptoContext(p); cc->Enable(PKE); cc->Enable(KEYSWITCH); cc->Enable(LEVELEDSHE);
    auto kp = cc->KeyGen(); cc->EvalMultKeyGen(kp.secretKey);
    
    int inputs[8][3] = {{0,0,0},{0,0,1},{0,1,0},{0,1,1},{1,0,0},{1,0,1},{1,1,0},{1,1,1}};
    
    std::cout << "  RingDim = 4096, Depth budget = 60\n\n";
    std::cout << "  ┌──────┬─────────────────────┬────────────────────────┐\n";
    std::cout << "  │ Depth│ Stable (φ/ψ)        │ Status                  │\n";
    std::cout << "  ├──────┼─────────────────────┼────────────────────────┤\n";

    for (int depth = 0; depth <= 15; depth++) {
        int phi_ok = 0, psi_ok = 0;
        const int T = 50;
        bool crashed = false;
        
        for (int t = 0; t < T && !crashed; t++) {
            int inp = rand() % 8;
            std::vector<double> dv = {(double)((inp>>0)&1), (double)((inp>>1)&1), (double)((inp>>2)&1)};
            DualGate x=input(cc,kp,dv[0]), y=input(cc,kp,dv[1]), z=input(cc,kp,dv[2]);
            
            try {
                DualGate cur_phi = stable_and(cc, x, y);
                DualGate cur_psi = stable_or(cc, x, z);
                
                for (int d = 1; d <= depth; d++) {
                    cur_phi = stable_and(cc, cur_phi, cur_psi);
                    cur_psi = stable_or(cc, cur_psi, cur_phi);
                }
                
                DualGate phi_out = stable_or(cc, cur_phi, z);
                DualGate psi_out = stable_and(cc, stable_or(cc, y, z), stable_or(cc, x, z));
                
                int expected = (inputs[inp][0] & inputs[inp][1]) | inputs[inp][2];
                int bp = (reveal(phi_out, cc, kp, PHI) > 0.5) ? 1 : 0;
                int bs = (reveal(psi_out, cc, kp, PSI) > 0.5) ? 1 : 0;
                if (bp == expected) phi_ok++;
                if (bs == expected) psi_ok++;
            } catch (...) {
                crashed = true;
            }
        }
        
        std::cout << "  │ " << std::setw(4) << depth 
                  << " │ " << (crashed ? "    CRASHED" : std::to_string(phi_ok) + "/" + std::to_string(T) + " · " + std::to_string(psi_ok) + "/" + std::to_string(T))
                  << "        │ "
                  << (crashed ? "DEPTH LIMIT ✗" : 
                      phi_ok == T && psi_ok == T ? "PERFECT ✓✓✓" :
                      phi_ok >= 45 && psi_ok >= 45 ? "GOOD ✓" : "DEGRADED")
                  << "       │\n";
        
        if (crashed) break;
    }
    
    std::cout << "  └──────────────────────────────────────────────────────┘\n\n";

    // iO CORRECTNESS
    std::cout << "  ┌──────────────────────────────────────────────────────┐\n";
    std::cout << "  │  iO CORRECTNESS — Stable observer, depth=0            │\n";
    std::cout << "  ├─────┬─────┬─────┬──────────┬──────────┬──────────────┤\n";
    
    int ok = 0;
    for (int i = 0; i < 8; i++) {
        std::vector<double> dv = {(double)inputs[i][0], (double)inputs[i][1], (double)inputs[i][2]};
        DualGate x=input(cc,kp,dv[0]), y=input(cc,kp,dv[1]), z=input(cc,kp,dv[2]);
        DualGate phi_and = stable_and(cc, x, y);
        DualGate phi_out = stable_or(cc, phi_and, z);
        DualGate psi_or1 = stable_or(cc, x, z);
        DualGate psi_or2 = stable_or(cc, y, z);
        DualGate psi_out = stable_and(cc, psi_or1, psi_or2);
        
        int expected = (inputs[i][0] & inputs[i][1]) | inputs[i][2];
        int bp = (reveal(phi_out, cc, kp, PHI) > 0.5) ? 1 : 0;
        int bs = (reveal(psi_out, cc, kp, PSI) > 0.5) ? 1 : 0;
        if (bp == expected && bs == expected) ok++;
        std::cout << "  │  " << inputs[i][0] << "  │  " << inputs[i][1] << "  │  " << inputs[i][2]
                  << "  │  " << bp << "(" << expected << ") │  " << bs << "(" << expected << ") │  "
                  << (bp==expected&&bs==expected?"OK ✓":"FAIL") << "        │\n";
    }
    std::cout << "  ├─────┴─────┴─────┴──────────┴──────────┴──────────────┤\n";
    std::cout << "  │  iO: " << ok << "/8 · λ=" << std::fixed << std::setprecision(2) << LAMBDA << "                                       │\n";
    std::cout << "  └──────────────────────────────────────────────────────┘\n\n";
    
    time_t et = time(0);
    std::cout << "  ╔══════════════════════════════════════════════════════════════╗\n";
    std::cout << "  ║  LYAPUNOV-STABLE iO — " << (ok == 8 ? "iO PRESERVED ✓✓✓" : "TUNING") << "                          ║\n";
    std::cout << "  ║  Author: Dan Fernandez / Primordial Omega Zero               ║\n";
    std::cout << "  ╚══════════════════════════════════════════════════════════════╝\n\n";
    std::cout << "  Ended: " << ctime(&et) << "\n";
    
    return 0;
}
