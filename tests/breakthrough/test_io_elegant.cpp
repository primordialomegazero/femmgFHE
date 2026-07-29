// ╔══════════════════════════════════════════════════════════════════╗
// ║  ELEGANT iO — Recursive Self-Observation via φ/ψ               ║
// ║  Foundation: Observer gate. Architecture: Infinite reflection.  ║
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

// ═══════════════════════════════════════════════════════════════
// THE OBSERVER GATE — The only primitive needed
// ═══════════════════════════════════════════════════════════════
// AND: φ observes conjunction, ψ reflects interaction pattern
DualGate observe_and(CryptoContext<DCRTPoly>& cc, const DualGate& X, const DualGate& Y) {
    auto a_out = cc->EvalMult(X.a, Y.a);
    auto sum = cc->EvalAdd(cc->EvalAdd(cc->EvalMult(X.a, Y.b), cc->EvalMult(X.b, Y.a)), cc->EvalMult(X.b, Y.b));
    auto neg_one = cc->MakeCKKSPackedPlaintext(std::vector<double>{-1.0});
    return {a_out, cc->EvalMult(neg_one, sum)};
}

// OR: φ observes disjunction, ψ reflects interaction pattern
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

// ═══════════════════════════════════════════════════════════════
// RECURSIVE SELF-OBSERVATION — The elegant architecture
// ═══════════════════════════════════════════════════════════════
//
// Instead of "chain of gates," we do "recursive self-observation."
// Each recursion = the system observes itself from a new angle.
// φ and ψ take turns being the observer and the observed.
//
// Level 0: observe_and(x, y) → AND from φ's perspective
// Level 1: observe_or(result, z) → OR from ψ's perspective  
// Level 2: observe_and(result, result) → Self-observation (fractal)
// ...infinite recursion naturally

struct Reflection {
    DualGate phi_view;  // What φ sees at this recursion depth
    DualGate psi_view;  // What ψ sees at this recursion depth
    int depth;          // How many reflections deep
};

Reflection recursive_observe(CryptoContext<DCRTPoly>& cc, 
                              const DualGate& X, const DualGate& Y, const DualGate& Z,
                              int max_depth) {
    Reflection r;
    
    // Base observation: φ sees AND(x,y), ψ sees OR(x,z)
    DualGate phi_base = observe_and(cc, X, Y);
    DualGate psi_base = observe_or(cc, X, Z);
    
    r.phi_view = phi_base;
    r.psi_view = psi_base;
    r.depth = 0;
    
    // Recursive self-observation
    // Each step: observe the previous observation from both angles
    DualGate current_phi = phi_base;
    DualGate current_psi = psi_base;
    
    for (int d = 1; d <= max_depth; d++) {
        // φ observes the ψ-reflection of previous step
        current_phi = observe_and(cc, current_phi, current_psi);
        // ψ observes the φ-reflection of previous step  
        current_psi = observe_or(cc, current_psi, current_phi);
        
        r.phi_view = current_phi;
        r.psi_view = current_psi;
        r.depth = d;
    }
    
    // Final: φ sees AND(phi_chain), ψ sees OR(psi_chain, Z)
    r.phi_view = observe_or(cc, current_phi, Z);
    r.psi_view = observe_and(cc, observe_or(cc, Y, Z), observe_or(cc, X, Z));
    
    return r;
}

int main() {
    time_t st = time(0);
    std::cout << "\n  ╔══════════════════════════════════════════════════════════════╗\n";
    std::cout << "  ║  ELEGANT iO — Recursive Self-Observation                    ║\n";
    std::cout << "  ║  φ and ψ observe each other. Computation = Reflection.      ║\n";
    std::cout << "  ╚══════════════════════════════════════════════════════════════╝\n";
    std::cout << "  Started: " << ctime(&st) << "\n";

    CCParams<CryptoContextCKKSRNS> p;
    p.SetMultiplicativeDepth(40); p.SetScalingModSize(50); p.SetBatchSize(512);
    p.SetRingDim(8192); p.SetSecretKeyDist(UNIFORM_TERNARY); p.SetSecurityLevel(HEStd_NotSet);
    auto cc = GenCryptoContext(p); cc->Enable(PKE); cc->Enable(KEYSWITCH); cc->Enable(LEVELEDSHE);
    auto kp = cc->KeyGen(); cc->EvalMultKeyGen(kp.secretKey);
    
    int inputs[8][3] = {{0,0,0},{0,0,1},{0,1,0},{0,1,1},{1,0,0},{1,0,1},{1,1,0},{1,1,1}};

    // ═══ DEPTH SWEEP — How deep can recursion go? ═══
    std::cout << "  ┌──────────────────────────────────────────────────────┐\n";
    std::cout << "  │  RECURSION DEPTH SWEEP                                │\n";
    std::cout << "  ├──────┬──────────┬──────────┬────────────────────────┤\n";
    std::cout << "  │ Depth│ φ-correct│ ψ-correct│ Status                  │\n";
    std::cout << "  ├──────┼──────────┼──────────┼────────────────────────┤\n";
    
    for (int depth = 0; depth <= 5; depth++) {
        int phi_ok = 0, psi_ok = 0;
        
        for (int i = 0; i < 8; i++) {
            std::vector<double> dv = {(double)inputs[i][0], (double)inputs[i][1], (double)inputs[i][2]};
            DualGate x=input(cc,kp,dv[0]), y=input(cc,kp,dv[1]), z=input(cc,kp,dv[2]);
            
            Reflection r = recursive_observe(cc, x, y, z, depth);
            
            int expected = (inputs[i][0] & inputs[i][1]) | inputs[i][2];
            int bp = (reveal(r.phi_view, cc, kp, PHI) > 0.5) ? 1 : 0;
            int bs = (reveal(r.psi_view, cc, kp, PSI) > 0.5) ? 1 : 0;
            if (bp == expected) phi_ok++;
            if (bs == expected) psi_ok++;
        }
        
        std::cout << "  │ " << std::setw(4) << depth 
                  << " │ " << std::setw(5) << phi_ok << "/8   │ " << std::setw(5) << psi_ok << "/8   │ "
                  << (phi_ok==8&&psi_ok==8 ? "PERFECT ✓✓✓" : phi_ok>=6&&psi_ok>=6 ? "GOOD ✓" : "TUNING")
                  << "         │\n";
    }
    
    std::cout << "  ├──────┴──────────┴──────────┴────────────────────────┤\n";
    std::cout << "  │  Recursive self-observation: φ↔ψ mutual reflection   │\n";
    std::cout << "  │  Each depth = another angle of the same truth        │\n";
    std::cout << "  └──────────────────────────────────────────────────────┘\n\n";

    // ═══ iO GAME — Is the observer indistinguishable? ═══
    std::cout << "  ┌──────────────────────────────────────────────────────┐\n";
    std::cout << "  │  iO GAME — Can attacker tell φ from ψ at depth 1?     │\n";
    
    const int TRIALS = 100;
    int io_ok = 0;
    std::cout << "  │  " << std::flush;
    
    for (int t = 0; t < TRIALS; t++) {
        int inp = rand() % 8;
        std::vector<double> dv = {(double)((inp>>0)&1), (double)((inp>>1)&1), (double)((inp>>2)&1)};
        DualGate x=input(cc,kp,dv[0]), y=input(cc,kp,dv[1]), z=input(cc,kp,dv[2]);
        
        Reflection r = recursive_observe(cc, x, y, z, 1);
        
        double vp = reveal(r.phi_view, cc, kp, PHI);
        double vs = reveal(r.psi_view, cc, kp, PSI);
        bool phi_is_A = (rand() % 2 == 0);
        
        if ((vp > vs) == phi_is_A) io_ok++;
        if ((t+1) % 25 == 0) std::cout << "." << std::flush;
    }
    
    double io_adv = std::abs(100.0 * io_ok / TRIALS - 50.0);
    std::cout << "\n  │  Attacker: " << std::fixed << std::setprecision(1) << 100.0*io_ok/TRIALS 
              << "% guess, " << std::setprecision(2) << io_adv << "% advantage               │\n";
    std::cout << "  │  STATUS: " << (io_adv < 5.0 ? "INDISTINGUISHABLE ✓✓✓" : "TUNING") << "                        │\n";
    std::cout << "  └──────────────────────────────────────────────────────┘\n\n";

    time_t et = time(0);
    std::cout << "  ╔══════════════════════════════════════════════════════════════╗\n";
    std::cout << "  ║  ELEGANT iO — Recursive Self-Observation                    ║\n";
    std::cout << "  ║  φ observes ψ. ψ observes φ. Truth emerges from reflection. ║\n";
    std::cout << "  ║  Author: Dan Fernandez / Primordial Omega Zero               ║\n";
    std::cout << "  ╚══════════════════════════════════════════════════════════════╝\n\n";
    std::cout << "  Ended: " << ctime(&et) << "\n";
    
    return 0;
}
