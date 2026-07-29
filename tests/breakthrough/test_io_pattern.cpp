// ╔══════════════════════════════════════════════════════════════════╗
// ║  iO PATTERN CHECK — Per-input advantage analysis               ║
// ║  Which inputs leak? Where does the attacker win?               ║
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
    std::cout << "  ║  iO PATTERN CHECK — Per-input advantage analysis            ║\n";
    std::cout << "  ╚══════════════════════════════════════════════════════════════╝\n";
    std::cout << "  Started: " << ctime(&st) << "\n";

    CCParams<CryptoContextCKKSRNS> p;
    p.SetMultiplicativeDepth(35); p.SetScalingModSize(50); p.SetBatchSize(1024);
    p.SetRingDim(8192); p.SetSecretKeyDist(UNIFORM_TERNARY); p.SetSecurityLevel(HEStd_NotSet);
    auto cc = GenCryptoContext(p); cc->Enable(PKE); cc->Enable(KEYSWITCH); cc->Enable(LEVELEDSHE);
    auto kp = cc->KeyGen(); cc->EvalMultKeyGen(kp.secretKey);
    
    int inputs[8][3] = {{0,0,0},{0,0,1},{0,1,0},{0,1,1},{1,0,0},{1,0,1},{1,1,0},{1,1,1}};
    
    std::cout << "  ┌──────────────────────────────────────────────────────┐\n";
    std::cout << "  │  PER-INPUT ANALYSIS — 50 trials each, φ/ψ values     │\n";
    std::cout << "  ├─────┬─────┬─────┬──────────┬──────────┬──────────────┤\n";
    std::cout << "  │  x  │  y  │  z  │ φ>ψ wins │ Attacker  │  Leak?       │\n";
    std::cout << "  ├─────┼─────┼─────┼──────────┼──────────┼──────────────┤\n";
    
    for (int inp = 0; inp < 8; inp++) {
        int x= inputs[inp][0], y= inputs[inp][1], z= inputs[inp][2];
        int phi_bigger = 0, attacker_wins = 0;
        const int T = 200;
        
        for (int t = 0; t < T; t++) {
            std::vector<double> dv = {(double)x, (double)y, (double)z};
            DualGate X = make_input(cc, kp, dv[0]);
            DualGate Y = make_input(cc, kp, dv[1]);
            DualGate Z = make_input(cc, kp, dv[2]);
            
            bool pia = (rng() % 2 == 0);
            
            // Compute both circuits in RAW
            DualGate phi_and = direct_and(cc, X, Y);
            DualGate phi_out = direct_or(cc, phi_and, Z);
            DualGate psi_or1 = direct_or(cc, X, Z);
            DualGate psi_or2 = direct_or(cc, Y, Z);
            DualGate psi_out = direct_and(cc, psi_or1, psi_or2);
            
            DualGate fg = pia ? encode_dual(cc, phi_out, psi_out) 
                               : encode_dual(cc, psi_out, phi_out);
            
            double vp = decode_dual(fg, cc, kp, PHI);
            double vs = decode_dual(fg, cc, kp, PSI);
            
            if (vp > vs) phi_bigger++;
            if ((vp > vs) == pia) attacker_wins++;
        }
        
        double phi_pct = 100.0 * phi_bigger / T;
        double att_pct = 100.0 * attacker_wins / T;
        double leak = std::abs(phi_pct - 50.0);
        
        std::cout << "  │  " << x << "  │  " << y << "  │  " << z 
                  << "  │ " << std::fixed << std::setprecision(0) << std::setw(5) << phi_pct << "%   │ "
                  << std::setw(5) << att_pct << "%    │ "
                  << (leak > 10 ? "⚠️ LEAK!" : att_pct > 65 || att_pct < 35 ? "⚠️ " : "OK  ")
                  << "        │\n";
    }
    
    std::cout << "  ├─────┴─────┴─────┴──────────┴──────────┴──────────────┤\n";
    std::cout << "  │  φ>ψ = attacker uses φ-value comparison as heuristic │\n";
    std::cout << "  │  Attacker% = how often heuristic matches reality     │\n";
    std::cout << "  │  50% = perfect indistinguishability                  │\n";
    std::cout << "  └──────────────────────────────────────────────────────┘\n\n";
    
    time_t et = time(0);
    std::cout << "  Ended: " << ctime(&et) << "\n";
    return 0;
}
