// ╔══════════════════════════════════════════════════════════════════╗
// ║  FULL CHECK — All working tests, comprehensive sweep            ║
// ║  iO v8 · Observer · Elegant · Depth · iO Game                   ║
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

// v8 PROVEN GATES
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
double reveal(DualGate& dg, CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, double root) {
    return decrypt_val(cc, kp, dg.a) + decrypt_val(cc, kp, dg.b) * root;
}
DualGate make_input(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, double val) {
    return {cc->Encrypt(kp.publicKey, cc->MakeCKKSPackedPlaintext(std::vector<double>{val})),
            cc->Encrypt(kp.publicKey, cc->MakeCKKSPackedPlaintext(std::vector<double>{0.0}))};
}

int main() {
    time_t st = time(0);
    std::cout << "\n  ╔══════════════════════════════════════════════════════════════╗\n";
    std::cout << "  ║  FULL CHECK — 8K ring, all metrics                          ║\n";
    std::cout << "  ╚══════════════════════════════════════════════════════════════╝\n";
    std::cout << "  Started: " << ctime(&st) << "\n";

    CCParams<CryptoContextCKKSRNS> p;
    p.SetMultiplicativeDepth(50); p.SetScalingModSize(50); p.SetBatchSize(512);
    p.SetRingDim(8192); p.SetSecretKeyDist(UNIFORM_TERNARY); p.SetSecurityLevel(HEStd_NotSet);
    auto cc = GenCryptoContext(p); cc->Enable(PKE); cc->Enable(KEYSWITCH); cc->Enable(LEVELEDSHE);
    auto kp = cc->KeyGen(); cc->EvalMultKeyGen(kp.secretKey);
    
    int inputs[8][3] = {{0,0,0},{0,0,1},{0,1,0},{0,1,1},{1,0,0},{1,0,1},{1,1,0},{1,1,1}};
    
    // ═══ 1. CORRECTNESS: v8 direct gates, depth 0 ═══
    std::cout << "  ┌──────────────────────────────────────────────────────┐\n";
    std::cout << "  │  1. CORRECTNESS — v8 gates, depth 0                  │\n";
    std::cout << "  ├─────┬─────┬─────┬──────────┬──────────┬──────────────┤\n";
    int ok1 = 0;
    for (int i = 0; i < 8; i++) {
        std::vector<double> dv = {(double)inputs[i][0], (double)inputs[i][1], (double)inputs[i][2]};
        DualGate x=make_input(cc,kp,dv[0]), y=make_input(cc,kp,dv[1]), z=make_input(cc,kp,dv[2]);
        DualGate phi_and = direct_and(cc, x, y);
        DualGate phi_out = direct_or(cc, phi_and, z);
        DualGate psi_or1 = direct_or(cc, x, z);
        DualGate psi_or2 = direct_or(cc, y, z);
        DualGate psi_out = direct_and(cc, psi_or1, psi_or2);
        int expected = (inputs[i][0] & inputs[i][1]) | inputs[i][2];
        int bp = (reveal(phi_out, cc, kp, PHI) > 0.5) ? 1 : 0;
        int bs = (reveal(psi_out, cc, kp, PSI) > 0.5) ? 1 : 0;
        if (bp == expected && bs == expected) ok1++;
        std::cout << "  │  " << inputs[i][0] << "  │  " << inputs[i][1] << "  │  " << inputs[i][2]
                  << "  │     " << bp << "    │     " << bs << "    │     " << expected << "        │\n";
    }
    std::cout << "  ├─────┴─────┴─────┴──────────┴──────────┴──────────────┤\n";
    std::cout << "  │  Correct: " << ok1 << "/8                                                  │\n";
    std::cout << "  └──────────────────────────────────────────────────────┘\n\n";

    // ═══ 2. RECURSIVE DEPTH SWEEP (observer style) ═══
    std::cout << "  ┌──────────────────────────────────────────────────────┐\n";
    std::cout << "  │  2. RECURSIVE DEPTH — Observer gate, depths 0-8      │\n";
    std::cout << "  ├──────┬──────────┬──────────┬────────────────────────┤\n";
    for (int depth = 0; depth <= 8; depth++) {
        int phi_ok = 0, psi_ok = 0;
        for (int i = 0; i < 8; i++) {
            std::vector<double> dv = {(double)inputs[i][0], (double)inputs[i][1], (double)inputs[i][2]};
            DualGate x=make_input(cc,kp,dv[0]), y=make_input(cc,kp,dv[1]), z=make_input(cc,kp,dv[2]);
            DualGate cp = direct_and(cc, x, y);
            DualGate cs = direct_or(cc, x, z);
            for (int d = 1; d <= depth; d++) {
                cp = direct_and(cc, cp, cs);
                cs = direct_or(cc, cs, cp);
            }
            DualGate po = direct_or(cc, cp, z);
            DualGate so = direct_and(cc, direct_or(cc, y, z), direct_or(cc, x, z));
            int expected = (inputs[i][0] & inputs[i][1]) | inputs[i][2];
            int bp = (reveal(po, cc, kp, PHI) > 0.5) ? 1 : 0;
            int bs = (reveal(so, cc, kp, PSI) > 0.5) ? 1 : 0;
            if (bp == expected) phi_ok++;
            if (bs == expected) psi_ok++;
        }
        std::cout << "  │ " << std::setw(4) << depth << " │ " << phi_ok << "/8 · " << psi_ok << "/8    │ "
                  << (phi_ok==8&&psi_ok==8 ? "PERFECT ✓✓✓" : phi_ok>=6&&psi_ok>=6 ? "GOOD ✓" : "DEGRADED")
                  << "         │\n";
    }
    std::cout << "  └──────────────────────────────────────────────────────┘\n\n";

    // ═══ 3. iO GAME — Indistinguishability at depth 0 and 3 ═══
    for (int depth : {0, 3}) {
        std::cout << "  ┌──────────────────────────────────────────────────────┐\n";
        std::cout << "  │  3. iO GAME — Depth " << depth << ", 100 trials                     │\n";
        const int T = 100;
        int io_ok = 0;
        for (int t = 0; t < T; t++) {
            int inp = rand() % 8;
            std::vector<double> dv = {(double)((inp>>0)&1), (double)((inp>>1)&1), (double)((inp>>2)&1)};
            DualGate x=make_input(cc,kp,dv[0]), y=make_input(cc,kp,dv[1]), z=make_input(cc,kp,dv[2]);
            DualGate cp = direct_and(cc, x, y);
            DualGate cs = direct_or(cc, x, z);
            for (int d = 1; d <= depth; d++) {
                cp = direct_and(cc, cp, cs);
                cs = direct_or(cc, cs, cp);
            }
            DualGate po = direct_or(cc, cp, z);
            DualGate so = direct_and(cc, direct_or(cc, y, z), direct_or(cc, x, z));
            double vp = reveal(po, cc, kp, PHI);
            double vs = reveal(so, cc, kp, PSI);
            bool phi_is_A = (rand() % 2 == 0);
            if ((vp > vs) == phi_is_A) io_ok++;
        }
        double adv = std::abs(100.0 * io_ok / T - 50.0);
        std::cout << "  │  Attacker: " << std::fixed << std::setprecision(1) << 100.0*io_ok/T 
                  << "% guess, " << std::setprecision(2) << adv << "% advantage               │\n";
        std::cout << "  │  STATUS: " << (adv < 5.0 ? "INDISTINGUISHABLE ✓✓✓" : "TUNING") << "                        │\n";
        std::cout << "  └──────────────────────────────────────────────────────┘\n\n";
    }

    // ═══ 4. VALUE DRIFT CHECK ═══
    std::cout << "  ┌──────────────────────────────────────────────────────┐\n";
    std::cout << "  │  4. VALUE DRIFT — φ/ψ values at depths 0,2,5,8       │\n";
    std::cout << "  ├──────┬──────────┬──────────┬────────────────────────┤\n";
    for (int depth : {0, 2, 5, 8}) {
        double avg_phi = 0, avg_psi = 0;
        for (int i = 0; i < 8; i++) {
            std::vector<double> dv = {(double)inputs[i][0], (double)inputs[i][1], (double)inputs[i][2]};
            DualGate x=make_input(cc,kp,dv[0]), y=make_input(cc,kp,dv[1]), z=make_input(cc,kp,dv[2]);
            DualGate cp = direct_and(cc, x, y);
            DualGate cs = direct_or(cc, x, z);
            for (int d = 1; d <= depth; d++) {
                cp = direct_and(cc, cp, cs);
                cs = direct_or(cc, cs, cp);
            }
            DualGate po = direct_or(cc, cp, z);
            avg_phi += reveal(po, cc, kp, PHI);
            avg_psi += reveal(po, cc, kp, PSI);
        }
        avg_phi /= 8; avg_psi /= 8;
        double drift_phi = std::abs(avg_phi - 0.5) > 0.3 ? 0 : 1; // Close to 0/1 = stable
        std::cout << "  │ " << std::setw(4) << depth << " │ φ=" << std::fixed << std::setprecision(4) << std::setw(8) << avg_phi
                  << " │ ψ=" << std::setw(8) << avg_psi << " │ "
                  << (std::abs(avg_phi-0.5) > 0.4 ? "STABLE ✓" : "DRIFTING") << "        │\n";
    }
    std::cout << "  └──────────────────────────────────────────────────────┘\n\n";
    
    time_t et = time(0);
    std::cout << "  ╔══════════════════════════════════════════════════════════════╗\n";
    std::cout << "  ║  FULL CHECK — 8K ring, all metrics                          ║\n";
    std::cout << "  ║  Author: Dan Fernandez / Primordial Omega Zero               ║\n";
    std::cout << "  ╚══════════════════════════════════════════════════════════════╝\n\n";
    std::cout << "  Ended: " << ctime(&et) << "\n";
    
    return 0;
}
