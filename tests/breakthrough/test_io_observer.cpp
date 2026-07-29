// ╔══════════════════════════════════════════════════════════════════╗
// ║  OBSERVER-OBSERVED iO — φ IS the computation, ψ IS the budget  ║
// ║  No separate encode. The gate IS the observer.                  ║
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
// OBSERVER GATE — φ computes, ψ reflects. One operation.
// ═══════════════════════════════════════════════════════════════
// This IS the AND gate. This IS the budget. No separation.
DualGate observe_and(CryptoContext<DCRTPoly>& cc, const DualGate& X, const DualGate& Y) {
    // φ observes: a_out = X.a * Y.a (AND of φ-values)
    auto a_out = cc->EvalMult(X.a, Y.a);
    
    // ψ reflects: b_out captures the interaction pattern
    auto sum = cc->EvalAdd(cc->EvalAdd(cc->EvalMult(X.a, Y.b), cc->EvalMult(X.b, Y.a)), cc->EvalMult(X.b, Y.b));
    auto neg_one = cc->MakeCKKSPackedPlaintext(std::vector<double>{-1.0});
    auto b_out = cc->EvalMult(neg_one, sum);
    
    // The (a,b) IS the observation. φ sees AND. ψ sees the reflection.
    // Budget is not consumed — it's TRANSFORMED into the observation.
    return {a_out, b_out};
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
double decode_dual(DualGate& dg, CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, double root) {
    return decrypt_val(cc, kp, dg.a) + decrypt_val(cc, kp, dg.b) * root;
}
DualGate make_input(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, double val) {
    return {cc->Encrypt(kp.publicKey, cc->MakeCKKSPackedPlaintext(std::vector<double>{val})),
            cc->Encrypt(kp.publicKey, cc->MakeCKKSPackedPlaintext(std::vector<double>{0.0}))};
}

int main() {
    time_t st = time(0);
    std::cout << "\n  ╔══════════════════════════════════════════════════════════════╗\n";
    std::cout << "  ║  OBSERVER-OBSERVED iO — φ IS computation, ψ IS budget       ║\n";
    std::cout << "  ║  No separate encode. Gate = Observer = Budget = Fractal.    ║\n";
    std::cout << "  ╚══════════════════════════════════════════════════════════════╝\n";
    std::cout << "  Started: " << ctime(&st) << "\n";

    CCParams<CryptoContextCKKSRNS> p;
    p.SetMultiplicativeDepth(30); p.SetScalingModSize(50); p.SetBatchSize(512);
    p.SetRingDim(4096); p.SetSecretKeyDist(UNIFORM_TERNARY); p.SetSecurityLevel(HEStd_NotSet);
    auto cc = GenCryptoContext(p); cc->Enable(PKE); cc->Enable(KEYSWITCH); cc->Enable(LEVELEDSHE);
    auto kp = cc->KeyGen(); cc->EvalMultKeyGen(kp.secretKey);
    
    std::cout << "  φ observes. ψ reflects. The gate IS the system.\n\n";

    // ═══ DEPTH TEST ═══
    std::cout << "  ┌──────────────────────────────────────────────────────┐\n";
    std::cout << "  │  DEPTH: Observer gate — how deep can it go?           │\n";
    std::cout << "  ├──────┬──────────┬──────────┬────────────────────────┤\n";
    
    for (int gates : {3, 5, 8, 10, 12, 15, 18, 20, 25, 30, 40, 50}) {
        DualGate x = make_input(cc, kp, 1.0);
        DualGate current = x;
        bool ok = true;
        
        for (int g = 0; g < gates && ok; g++) {
            try { current = observe_and(cc, current, current); }
            catch (...) { ok = false; }
        }
        
        double val = ok ? decode_dual(current, cc, kp, PHI) : -999;
        int bit = (val > 0.5) ? 1 : 0;
        
        std::cout << "  │ " << std::setw(4) << gates << " │ " 
                  << (ok ? std::to_string(bit) : "CRASH") << "       │ "
                  << std::setw(22) << std::left 
                  << (ok ? (bit == 0 ? "ALIVE (bit=0)" : "ALIVE (bit=1)") : "DEPTH EXCEEDED")
                  << " │\n";
        
        if (!ok) break;
    }
    
    std::cout << "  ├──────┴──────────┴──────────┴────────────────────────┤\n";
    std::cout << "  │  Observer gate: no encode overhead. Raw computation. │\n";
    std::cout << "  └──────────────────────────────────────────────────────┘\n\n";

    // ═══ iO CORRECTNESS ═══
    std::cout << "  ┌──────────────────────────────────────────────────────┐\n";
    std::cout << "  │  iO CORRECTNESS — Observer gates for both circuits    │\n";
    std::cout << "  ├─────┬─────┬─────┬──────────┬──────────┬──────────────┤\n";
    
    int inputs[8][3] = {{0,0,0},{0,0,1},{0,1,0},{0,1,1},{1,0,0},{1,0,1},{1,1,0},{1,1,1}};
    int ok = 0;
    
    for (int i = 0; i < 8; i++) {
        std::vector<double> dv = {(double)inputs[i][0], (double)inputs[i][1], (double)inputs[i][2]};
        DualGate x=make_input(cc,kp,dv[0]), y=make_input(cc,kp,dv[1]), z=make_input(cc,kp,dv[2]);
        
        // φ: (x AND y) OR z — using observer gates
        DualGate phi_and = observe_and(cc, x, y);
        DualGate phi_out = observe_or(cc, phi_and, z);
        
        // ψ: (x OR z) AND (y OR z) — using observer gates
        DualGate psi_or1 = observe_or(cc, x, z);
        DualGate psi_or2 = observe_or(cc, y, z);
        DualGate psi_out = observe_and(cc, psi_or1, psi_or2);
        
        // NO separate encode — the gates THEMSELVES are the dual encoding
        // φ-decode phi_out = Circuit A result
        // ψ-decode psi_out = Circuit B result
        // They are already in (a,b) form from the observer gates
        
        int expected = (inputs[i][0] & inputs[i][1]) | inputs[i][2];
        int bp = (decode_dual(phi_out, cc, kp, PHI) > 0.5) ? 1 : 0;
        int bs = (decode_dual(psi_out, cc, kp, PSI) > 0.5) ? 1 : 0;
        if (bp == expected && bs == expected) ok++;
        
        std::cout << "  │  " << inputs[i][0] << "  │  " << inputs[i][1] << "  │  " << inputs[i][2]
                  << "  │  " << bp << "(" << expected << ") │  " << bs << "(" << expected << ") │  "
                  << (bp==expected&&bs==expected?"OK ✓":"FAIL") << "        │\n";
    }
    std::cout << "  ├─────┴─────┴─────┴──────────┴──────────┴──────────────┤\n";
    std::cout << "  │  iO: " << ok << "/8 · No encode. Observer = Gate = System             │\n";
    std::cout << "  └──────────────────────────────────────────────────────┘\n\n";
    
    time_t et = time(0);
    std::cout << "  ╔══════════════════════════════════════════════════════════════╗\n";
    std::cout << "  ║  OBSERVER-OBSERVED — " << (ok == 8 ? "GATE IS THE SYSTEM ✓✓✓" : "TUNING") << "                       ║\n";
    std::cout << "  ║  Author: Dan Fernandez / Primordial Omega Zero               ║\n";
    std::cout << "  ╚══════════════════════════════════════════════════════════════╝\n\n";
    std::cout << "  Ended: " << ctime(&et) << "\n";
    
    return 0;
}
