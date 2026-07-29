// ╔══════════════════════════════════════════════════════════════════╗
// ║  NATURAL FRACTAL iO — Double mirror recursion                  ║
// ║  Mirror 1: φ/ψ algebraic conjugates                           ║
// ║  Mirror 2: AND/OR operational mirrors                         ║
// ║  Combined: Infinite fractal via recursive mirror swapping      ║
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

// ═══════════════════════════════════════════════════════════════
// iO CORE — UNTOUCHED
// ═══════════════════════════════════════════════════════════════
DualGate unified_nand(CryptoContext<DCRTPoly>& cc, const DualGate& X, const DualGate& Y) {
    auto a1a2 = cc->EvalMult(X.a, Y.a), b1b2 = cc->EvalMult(X.b, Y.b);
    auto one = cc->MakeCKKSPackedPlaintext(std::vector<double>{1.0});
    auto a_out = cc->EvalSub(one, cc->EvalAdd(a1a2, b1b2));
    auto sum = cc->EvalAdd(cc->EvalAdd(cc->EvalMult(X.a, Y.b), cc->EvalMult(Y.a, X.b)), b1b2);
    auto neg_one = cc->MakeCKKSPackedPlaintext(std::vector<double>{-1.0});
    return {a_out, cc->EvalMult(neg_one, sum)};
}

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

// ═══════════════════════════════════════════════════════════════
// NATURAL FRACTAL — Double mirror recursion
// ═══════════════════════════════════════════════════════════════
//
// MIRROR 1: φ/ψ algebraic conjugates
// MIRROR 2: AND/OR operational mirrors
//
// Combined: φ(AND) ↔ ψ(OR) at every recursion level.
// Each recursion swaps the mirrors → natural fractal emerges.

struct FractalGate {
    DualGate gate;
    int depth;
};

FractalGate natural_fractal(CryptoContext<DCRTPoly>& cc, 
                             DualGate X, DualGate Y, 
                             int depth, bool phi_is_A) {
    if (depth <= 0) {
        // Base: φ=AND, ψ=OR (or swapped)
        DualGate gA = direct_and(cc, X, Y);
        DualGate gB = direct_or(cc, X, Y);
        return { phi_is_A ? encode_dual(cc, gA, gB) : encode_dual(cc, gB, gA), 0 };
    }
    
    // RECURSIVE MIRROR: swap X↔Y AND swap φ↔ψ
    // This creates the fractal: forward path AND, mirror path OR
    // At next level: forward path OR, mirror path AND
    // Ad infinitum → natural fractalization
    
    FractalGate left  = natural_fractal(cc, X, Y, depth - 1, phi_is_A);      // φ-path
    FractalGate right = natural_fractal(cc, Y, X, depth - 1, !phi_is_A);     // ψ-path (mirrored)
    
    // Combine the fractal branches
    DualGate combined_A = phi_is_A ? direct_and(cc, left.gate, right.gate) 
                                    : direct_or(cc, left.gate, right.gate);
    DualGate combined_B = phi_is_A ? direct_or(cc, left.gate, right.gate) 
                                    : direct_and(cc, left.gate, right.gate);
    
    return { phi_is_A ? encode_dual(cc, combined_A, combined_B) 
                       : encode_dual(cc, combined_B, combined_A), depth };
}

int main() {
    time_t st = time(0);
    std::cout << "\n  ╔══════════════════════════════════════════════════════════════╗\n";
    std::cout << "  ║  NATURAL FRACTAL iO — Double Mirror Recursion               ║\n";
    std::cout << "  ║  Mirror 1 (φ/ψ) × Mirror 2 (AND/OR) = Infinite Fractal     ║\n";
    std::cout << "  ╚══════════════════════════════════════════════════════════════╝\n";
    std::cout << "  Started: " << ctime(&st) << "\n";

    CCParams<CryptoContextCKKSRNS> p;
    p.SetMultiplicativeDepth(40); p.SetScalingModSize(50); p.SetBatchSize(1024);
    p.SetRingDim(8192); p.SetSecretKeyDist(UNIFORM_TERNARY); p.SetSecurityLevel(HEStd_NotSet);
    auto cc = GenCryptoContext(p); cc->Enable(PKE); cc->Enable(KEYSWITCH); cc->Enable(LEVELEDSHE);
    auto kp = cc->KeyGen(); cc->EvalMultKeyGen(kp.secretKey);
    
    int inputs[8][3] = {{0,0,0},{0,0,1},{0,1,0},{0,1,1},{1,0,0},{1,0,1},{1,1,0},{1,1,1}};
    
    std::cout << "  φ = " << std::fixed << std::setprecision(6) << PHI << " (forward mirror)\n";
    std::cout << "  ψ = " << PSI << " (reverse mirror)\n\n";
    
    std::cout << "  ┌──────────────────────────────────────────────────────┐\n";
    std::cout << "  │  FRACTAL DEPTH SWEEP — Natural mirror recursion       │\n";
    std::cout << "  ├──────────┬──────────┬──────────┬────────────────────┤\n";
    std::cout << "  │  Depth   │  φ-OK    │  ψ-OK    │  Status             │\n";
    std::cout << "  ├──────────┼──────────┼──────────┼────────────────────┤\n";
    
    for (int depth = 0; depth <= 3; depth++) {
        int r1_ok = 0, r2_ok = 0;
        bool phi_is_A = (rng() % 2 == 0);
        
        for (int i = 0; i < 8; i++) {
            std::vector<double> dv = {(double)inputs[i][0], (double)inputs[i][1], (double)inputs[i][2]};
            DualGate x = make_input(cc, kp, dv[0]);
            DualGate y = make_input(cc, kp, dv[1]);
            DualGate z = make_input(cc, kp, dv[2]);
            
            // Build fractal from (x,y) and combine with z
            FractalGate fg = natural_fractal(cc, x, y, depth, phi_is_A);
            DualGate out_A = direct_or(cc, fg.gate, z);  // φ: OR(fractal, z)
            DualGate out_B = direct_and(cc, fg.gate, z); // ψ: AND(fractal, z)
            DualGate output = phi_is_A ? encode_dual(cc, out_A, out_B) : encode_dual(cc, out_B, out_A);
            
            int expected = (inputs[i][0] & inputs[i][1]) | inputs[i][2];
            int bp = (decode_dual(output, cc, kp, PHI) > 0.5) ? 1 : 0;
            int bs = (decode_dual(output, cc, kp, PSI) > 0.5) ? 1 : 0;
            int bA = phi_is_A ? bp : bs, bB = phi_is_A ? bs : bp;
            if (bA == expected) r1_ok++; if (bB == expected) r2_ok++;
        }
        
        std::cout << "  │  " << std::setw(5) << depth << "    │  " << std::setw(5) << r1_ok << "/8   │  " 
                  << std::setw(5) << r2_ok << "/8   │  " 
                  << (r1_ok==8&&r2_ok==8 ? "PERFECT ✓✓✓" : r1_ok>=6&&r2_ok>=6 ? "GOOD ✓" : "TUNING") 
                  << "         │\n";
    }
    
    std::cout << "  ├──────────┴──────────┴──────────┴────────────────────┤\n";
    std::cout << "  │  Double mirror recursion: φ/ψ × AND/OR               │\n";
    std::cout << "  │  Each recursion = natural fractalization             │\n";
    std::cout << "  │  No hardcoded structure. Mirrors do the work.        │\n";
    std::cout << "  └──────────────────────────────────────────────────────┘\n\n";
    
    // ═══ iO GAME at best depth ═══
    std::cout << "  ┌──────────────────────────────────────────────────────┐\n";
    std::cout << "  │  iO GAME (depth=1, 150 trials)                        │\n";
    std::cout << "  ├──────────────────────────────────────────────────────┤\n";
    
    const int TRIALS = 150;
    int io_ok = 0;
    std::cout << "  │  " << std::flush;
    for (int t = 0; t < TRIALS; t++) {
        int inp = rng() % 8;
        std::vector<double> dv = {(double)((inp>>0)&1), (double)((inp>>1)&1), (double)((inp>>2)&1)};
        DualGate x = make_input(cc, kp, dv[0]);
        DualGate y = make_input(cc, kp, dv[1]);
        DualGate z = make_input(cc, kp, dv[2]);
        
        bool phi_is_A = (rng() % 2 == 0);
        FractalGate fg = natural_fractal(cc, x, y, 1, phi_is_A);
        DualGate out_A = direct_or(cc, fg.gate, z);
        DualGate out_B = direct_and(cc, fg.gate, z);
        DualGate output = phi_is_A ? encode_dual(cc, out_A, out_B) : encode_dual(cc, out_B, out_A);
        
        double vp = decode_dual(output, cc, kp, PHI);
        double vs = decode_dual(output, cc, kp, PSI);
        if ((vp > vs) == phi_is_A) io_ok++;
        if ((t+1) % 30 == 0) std::cout << "." << std::flush;
    }
    
    double io_adv = std::abs(100.0 * io_ok / TRIALS - 50.0);
    std::cout << "\n  │  iO Game: " << std::fixed << std::setprecision(1) << 100.0*io_ok/TRIALS 
              << "% guess, " << std::setprecision(2) << io_adv << "% advantage               │\n";
    std::cout << "  │  STATUS: " << (io_adv < 5.0 ? "INDISTINGUISHABLE ✓✓✓" : "TUNING") << "                        │\n";
    std::cout << "  └──────────────────────────────────────────────────────┘\n\n";
    
    time_t et = time(0);
    std::cout << "  ╔══════════════════════════════════════════════════════════════╗\n";
    std::cout << "  ║  NATURAL FRACTAL iO — Double Mirror Recursion               ║\n";
    std::cout << "  ║  Author: Dan Fernandez / Primordial Omega Zero               ║\n";
    std::cout << "  ╚══════════════════════════════════════════════════════════════╝\n\n";
    std::cout << "  Ended: " << ctime(&et) << "\n";
    
    return 0;
}
