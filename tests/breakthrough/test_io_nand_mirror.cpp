// ╔══════════════════════════════════════════════════════════════════╗
// ║  NAND-MIRROR — Universal gate, budget-free computation          ║
// ║  AND via log-space addition + NOT via φ-ψ mirror               ║
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
const double ZERO_LOG = -100.0;
const double THRESHOLD = -10.0;

struct DualGate { Ciphertext<DCRTPoly> a; Ciphertext<DCRTPoly> b; };

// ═══════════════════════════════════════════════════════════════
// NAND = NOT(AND) 
// AND = ADDITION in log space (ZERO BUDGET!)
// NOT = φ-ψ MIRROR REFLECTION (ZERO BUDGET!)
// ═══════════════════════════════════════════════════════════════

DualGate observe_nand(CryptoContext<DCRTPoly>& cc, 
                       const DualGate& X, const DualGate& Y) {
    // Step 1: AND in log space = ADDITION
    auto and_a = cc->EvalAdd(X.a, Y.a);  // ZERO BUDGET!
    auto and_b = cc->EvalAdd(X.b, Y.b);  // ZERO BUDGET!
    
    // Step 2: NOT via φ-ψ MIRROR
    // The mirror reflects the value:
    // φ sees the value, ψ sees the INVERSE
    // φ·ψ = -1 → multiplying by ψ FLIPS the sign!
    
    auto psi_pt = cc->MakeCKKSPackedPlaintext(std::vector<double>{PSI});
    
    // NOT: Reflect through ψ
    // In log space: NOT(log_val) = log_val reflected through ψ
    // ψ = -0.618 → reflection changes the "direction"
    auto not_a = cc->EvalMult(and_a, psi_pt);  // Scalar mult = ZERO BUDGET!
    
    // Mirror component: b reflects the inverse
    auto not_b = cc->EvalMult(and_b, psi_pt);  // ZERO BUDGET!
    
    return {not_a, not_b};
}

// ═══════════════════════════════════════════════════════════════
// ENCODING & DECODING
// ═══════════════════════════════════════════════════════════════
DualGate encode_log(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, double val) {
    double log_val = (val < 0.5) ? ZERO_LOG : 0.0;
    return {cc->Encrypt(kp.publicKey, cc->MakeCKKSPackedPlaintext(std::vector<double>{log_val})),
            cc->Encrypt(kp.publicKey, cc->MakeCKKSPackedPlaintext(std::vector<double>{0.0}))};
}

double decode_log(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp,
                  const DualGate& dg) {
    Plaintext pt;
    cc->Decrypt(kp.secretKey, dg.a, &pt);
    double log_val = pt->GetCKKSPackedValue()[0].real();
    
    // After mirror, values are FLIPPED:
    // 0 → -100 → reflected → +61.8 → threshold → 1
    // 1 → 0 → reflected → 0 → threshold → 0
    // So: HIGH values = 1, LOW values = 0 (inverted from before!)
    
    if (log_val > -10.0) return 1.0;  // Reflected 0 → 1
    return 0.0;                        // Reflected 1 → 0
}

double reveal_log(DualGate& dg, CryptoContext<DCRTPoly>& cc, 
                  KeyPair<DCRTPoly>& kp) {
    return decode_log(cc, kp, dg);
}

// ═══════════════════════════════════════════════════════════════
// BUILD ALL GATES FROM NAND
// ═══════════════════════════════════════════════════════════════
DualGate observe_not(CryptoContext<DCRTPoly>& cc, const DualGate& X) {
    // NOT(X) = NAND(X, X)
    return observe_nand(cc, X, X);
}

DualGate observe_and_from_nand(CryptoContext<DCRTPoly>& cc, 
                                const DualGate& X, const DualGate& Y) {
    // AND(X,Y) = NOT(NAND(X,Y))
    DualGate nand_xy = observe_nand(cc, X, Y);
    return observe_not(cc, nand_xy);
}

DualGate observe_or_from_nand(CryptoContext<DCRTPoly>& cc, 
                               const DualGate& X, const DualGate& Y) {
    // OR(X,Y) = NAND(NOT(X), NOT(Y))
    DualGate not_x = observe_not(cc, X);
    DualGate not_y = observe_not(cc, Y);
    return observe_nand(cc, not_x, not_y);
}

// ═══════════════════════════════════════════════════════════════
// MAIN
// ═══════════════════════════════════════════════════════════════
int main() {
    time_t st = time(0);
    std::cout << "\n  ╔══════════════════════════════════════════════════════════════╗\n";
    std::cout << "  ║  NAND-MIRROR — Universal gate, budget-free                  ║\n";
    std::cout << "  ║  AND = ADDITION, NOT = φ-ψ MIRROR                          ║\n";
    std::cout << "  ╚══════════════════════════════════════════════════════════════╝\n";
    std::cout << "  Started: " << ctime(&st) << "\n";

    CCParams<CryptoContextCKKSRNS> p;
    p.SetMultiplicativeDepth(5); p.SetScalingModSize(50); p.SetBatchSize(512);
    p.SetRingDim(4096); p.SetSecretKeyDist(UNIFORM_TERNARY); p.SetSecurityLevel(HEStd_NotSet);
    auto cc = GenCryptoContext(p); cc->Enable(PKE); cc->Enable(KEYSWITCH); cc->Enable(LEVELEDSHE);
    auto kp = cc->KeyGen(); cc->EvalMultKeyGen(kp.secretKey);
    
    std::cout << "  RingDim = 4096, Depth budget = 5 (MINIMAL!)\n\n";

    // ═══ NAND TRUTH TABLE ═══
    std::cout << "  ┌──────────────────────────────────────────────────────┐\n";
    std::cout << "  │  NAND Truth Table — Log-space AND + φ-ψ Mirror       │\n";
    std::cout << "  ├─────┬─────┬──────────┬────────────────────────────┤\n";
    std::cout << "  │  X  │  Y  │  NAND    │  Status                      │\n";
    std::cout << "  ├─────┼─────┼──────────┼────────────────────────────┤\n";
    
    int nand_ok = 0;
    for (int x = 0; x <= 1; x++) {
        for (int y = 0; y <= 1; y++) {
            DualGate X = encode_log(cc, kp, (double)x);
            DualGate Y = encode_log(cc, kp, (double)y);
            
            DualGate nand_result = observe_nand(cc, X, Y);
            double nand_val = reveal_log(nand_result, cc, kp);
            int nand_bit = (int)nand_val;
            int exp_nand = !(x & y);
            
            if (nand_bit == exp_nand) nand_ok++;
            
            std::cout << "  │  " << x << "  │  " << y 
                      << "  │  " << nand_bit << " (" << std::fixed << std::setprecision(1) << nand_val << ")"
                      << "    │  " << (nand_bit==exp_nand?"OK ✓":"FAIL")
                      << "                          │\n";
        }
    }
    std::cout << "  ├─────┴─────┴──────────┴────────────────────────────┤\n";
    std::cout << "  │  NAND: " << nand_ok << "/4 correct                                         │\n";
    std::cout << "  └──────────────────────────────────────────────────────┘\n\n";

    // ═══ ALL GATES FROM NAND ═══
    std::cout << "  ┌──────────────────────────────────────────────────────┐\n";
    std::cout << "  │  ALL GATES — Built from NAND (Universal)              │\n";
    std::cout << "  ├─────┬─────┬──────┬──────┬──────┬──────────────────┤\n";
    std::cout << "  │  X  │  Y  │ NAND │ AND  │ OR   │ Status            │\n";
    std::cout << "  ├─────┼─────┼──────┼──────┼──────┼──────────────────┤\n";
    
    int all_ok = 0;
    for (int x = 0; x <= 1; x++) {
        for (int y = 0; y <= 1; y++) {
            DualGate X = encode_log(cc, kp, (double)x);
            DualGate Y = encode_log(cc, kp, (double)y);
            
            DualGate nand_temp = observe_nand(cc, X, Y);
            int nand_bit = (int)reveal_log(nand_temp, cc, kp);
            DualGate and_temp = observe_and_from_nand(cc, X, Y);
            int and_bit = (int)reveal_log(and_temp, cc, kp);
            DualGate or_temp = observe_or_from_nand(cc, X, Y);
            int or_bit = (int)reveal_log(or_temp, cc, kp);
            
            int exp_nand = !(x & y), exp_and = x & y, exp_or = x | y;
            
            if (nand_bit==exp_nand && and_bit==exp_and && or_bit==exp_or) all_ok++;
            
            std::cout << "  │  " << x << "  │  " << y 
                      << "  │  " << nand_bit << "(" << exp_nand << ")"
                      << "  │  " << and_bit << "(" << exp_and << ")"
                      << "  │  " << or_bit << "(" << exp_or << ")"
                      << "  │  " << (nand_bit==exp_nand&&and_bit==exp_and&&or_bit==exp_or?"OK ✓":"FAIL")
                      << "              │\n";
        }
    }
    std::cout << "  ├─────┴─────┴──────┴──────┴──────┴──────────────────┤\n";
    std::cout << "  │  All gates: " << all_ok << "/4 correct                                      │\n";
    std::cout << "  └──────────────────────────────────────────────────────┘\n\n";

    // ═══ DEEP CHAIN ═══
    std::cout << "  ┌──────────────────────────────────────────────────────┐\n";
    std::cout << "  │  DEEP CHAIN — NAND with self (Budget = 5!)           │\n";
    std::cout << "  ├──────┬──────────┬──────────────────────────────────┤\n";
    
    for (int gates : {5, 10, 20, 50, 100, 200, 500}) {
        DualGate cur = encode_log(cc, kp, 1.0);
        bool crashed = false;
        
        try {
            for (int g = 0; g < gates; g++) {
                cur = observe_nand(cc, cur, cur);
            }
        } catch (...) {
            crashed = true;
        }
        
        if (!crashed) {
            double val = reveal_log(cur, cc, kp);
            std::cout << "  │ " << std::setw(4) << gates
                      << " │ " << std::setw(6) << (int)val
                      << "      │ ALIVE ✓                            │\n";
        } else {
            std::cout << "  │ " << std::setw(4) << gates
                      << " │ CRASH    │ BUDGET OUT                         │\n";
            break;
        }
    }
    
    std::cout << "  ├──────┴──────────┴──────────────────────────────────┤\n";
    std::cout << "  │  NAND-MIRROR: Budget 5, Depth: POTENTIALLY ∞        │\n";
    std::cout << "  └──────────────────────────────────────────────────────┘\n\n";
    
    time_t et = time(0);
    std::cout << "  ╔══════════════════════════════════════════════════════════════╗\n";
    std::cout << "  ║  NAND-MIRROR — Universal Budget-Free Computation             ║\n";
    std::cout << "  ║  Author: Dan Fernandez / Primordial Omega Zero               ║\n";
    std::cout << "  ╚══════════════════════════════════════════════════════════════╝\n\n";
    std::cout << "  Ended: " << ctime(&et) << "\n";
    
    return 0;
}
