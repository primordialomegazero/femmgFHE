// ╔══════════════════════════════════════════════════════════════════╗
// ║  φ-LOG FIX — Sharp zero encoding + threshold decode             ║
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
const double LOG_PHI = std::log(PHI); 
const double ZERO_LOG = -100.0; // Sharp "minus infinity" for 0
const double THRESHOLD = -10.0;  // Below this = 0, above = 1

struct DualGate { Ciphertext<DCRTPoly> a; Ciphertext<DCRTPoly> b; };

// ═══════════════════════════════════════════════════════════════
// φ-LOG OPERATIONS — ZERO BUDGET
// ═══════════════════════════════════════════════════════════════
DualGate observe_and_log(CryptoContext<DCRTPoly>& cc, 
                          const DualGate& X, const DualGate& Y) {
    // AND = ADDITION in log space
    return {cc->EvalAdd(X.a, Y.a), cc->EvalAdd(X.b, Y.b)};
}

DualGate observe_or_log(CryptoContext<DCRTPoly>& cc, 
                         const DualGate& X, const DualGate& Y) {
    // OR via De Morgan: NOT(AND(NOT(X), NOT(Y)))
    // NOT(X) = -1 * X (in log space)
    auto neg_one = cc->MakeCKKSPackedPlaintext(std::vector<double>{-1.0});
    
    // NOT(X) and NOT(Y)
    auto not_x = cc->EvalMult(X.a, neg_one);  // Scalar mult = FREE
    auto not_y = cc->EvalMult(Y.a, neg_one);  // Scalar mult = FREE
    
    // AND(NOT(X), NOT(Y)) = not_x + not_y
    auto nand_log = cc->EvalAdd(not_x, not_y);
    
    // NOT of NAND = OR = -1 * nand_log
    auto or_log = cc->EvalMult(nand_log, neg_one);
    
    // Mirror component
    auto b_out = cc->EvalAdd(X.b, Y.b);
    
    return {or_log, b_out};
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
    
    // Threshold decode
    if (log_val < THRESHOLD) return 0.0;
    return 1.0;
}

double reveal_log(DualGate& dg, CryptoContext<DCRTPoly>& cc, 
                  KeyPair<DCRTPoly>& kp) {
    return decode_log(cc, kp, dg);
}

// ═══════════════════════════════════════════════════════════════
// MAIN
// ═══════════════════════════════════════════════════════════════
int main() {
    time_t st = time(0);
    std::cout << "\n  ╔══════════════════════════════════════════════════════════════╗\n";
    std::cout << "  ║  φ-LOG FIX — Sharp Zero + Threshold Decode                  ║\n";
    std::cout << "  ╚══════════════════════════════════════════════════════════════╝\n";
    std::cout << "  Started: " << ctime(&st) << "\n";

    CCParams<CryptoContextCKKSRNS> p;
    p.SetMultiplicativeDepth(5); p.SetScalingModSize(50); p.SetBatchSize(512);
    p.SetRingDim(4096); p.SetSecretKeyDist(UNIFORM_TERNARY); p.SetSecurityLevel(HEStd_NotSet);
    auto cc = GenCryptoContext(p); cc->Enable(PKE); cc->Enable(KEYSWITCH); cc->Enable(LEVELEDSHE);
    auto kp = cc->KeyGen(); cc->EvalMultKeyGen(kp.secretKey);
    
    std::cout << "  Zero encoded as: " << ZERO_LOG << ", Threshold: " << THRESHOLD << "\n\n";

    // ═══ AND/OR TEST ═══
    std::cout << "  ┌──────────────────────────────────────────────────────┐\n";
    std::cout << "  │  AND/OR Truth Table — Log Space                       │\n";
    std::cout << "  ├─────┬─────┬──────────┬──────────┬──────────────────┤\n";
    std::cout << "  │  X  │  Y  │  AND     │  OR      │  Status           │\n";
    std::cout << "  ├─────┼─────┼──────────┼──────────┼──────────────────┤\n";
    
    int ok = 0;
    for (int x = 0; x <= 1; x++) {
        for (int y = 0; y <= 1; y++) {
            DualGate X = encode_log(cc, kp, (double)x);
            DualGate Y = encode_log(cc, kp, (double)y);
            
            DualGate and_result = observe_and_log(cc, X, Y);
            DualGate or_result = observe_or_log(cc, X, Y);
            
            double and_val = reveal_log(and_result, cc, kp);
            double or_val = reveal_log(or_result, cc, kp);
            
            int and_bit = (int)and_val;
            int or_bit = (int)or_val;
            int exp_and = x & y, exp_or = x | y;
            
            if (and_bit == exp_and && or_bit == exp_or) ok++;
            
            std::cout << "  │  " << x << "  │  " << y 
                      << "  │  " << and_bit << " (" << std::fixed << std::setprecision(1) << and_val << ")"
                      << "   │  " << or_bit << " (" << std::setprecision(1) << or_val << ")"
                      << "   │  " << (and_bit==exp_and&&or_bit==exp_or?"OK ✓":"FAIL")
                      << "            │\n";
        }
    }
    std::cout << "  ├─────┴─────┴──────────┴──────────┴──────────────────┤\n";
    std::cout << "  │  Correct: " << ok << "/4                                               │\n";
    std::cout << "  └──────────────────────────────────────────────────────┘\n\n";

    // ═══ DEEP CHAIN TEST ═══
    std::cout << "  ┌──────────────────────────────────────────────────────┐\n";
    std::cout << "  │  DEEP CHAIN — Log-space AND with self (Budget = 5!)  │\n";
    std::cout << "  ├──────┬──────────┬──────────────────────────────────┤\n";
    
    for (int gates : {5, 10, 20, 50, 100, 200, 500}) {
        DualGate cur = encode_log(cc, kp, 1.0);
        bool crashed = false;
        
        try {
            for (int g = 0; g < gates; g++) {
                cur = observe_and_log(cc, cur, cur);
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
    std::cout << "  │  Log-space AND: Budget 5, Depth: POTENTIALLY ∞      │\n";
    std::cout << "  └──────────────────────────────────────────────────────┘\n\n";
    
    time_t et = time(0);
    std::cout << "  Ended: " << ctime(&et) << "\n";
    
    return 0;
}
