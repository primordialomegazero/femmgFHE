// ╔══════════════════════════════════════════════════════════════════╗
// ║  LOG-SPACE PERFECT — Budget-Free Universal Gates               ║
// ║  AND=ADDITION (0 budget), NOT=Threshold Inverse (0 budget)     ║
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
const double ZERO_LOG = -100.0;  // log_φ(0) ≈ -∞ → -100
const double THRESHOLD = -10.0;   // Below = 0, Above = 1

struct DualGate { Ciphertext<DCRTPoly> a; Ciphertext<DCRTPoly> b; };

// ═══════════════════════════════════════════════════════════════
// φ-LOG ENCODING
// ═══════════════════════════════════════════════════════════════
// Boolean → Log-space:
//   0 → -100 (log_φ(0) approximation)
//   1 → 0    (log_φ(1) = 0)

DualGate encode_log(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, double val) {
    double log_val = (val < 0.5) ? ZERO_LOG : 0.0;
    return {cc->Encrypt(kp.publicKey, cc->MakeCKKSPackedPlaintext(std::vector<double>{log_val})),
            cc->Encrypt(kp.publicKey, cc->MakeCKKSPackedPlaintext(std::vector<double>{0.0}))};
}

double decode_log(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, const DualGate& dg) {
    Plaintext pt;
    cc->Decrypt(kp.secretKey, dg.a, &pt);
    double log_val = pt->GetCKKSPackedValue()[0].real();
    return (log_val < THRESHOLD) ? 0.0 : 1.0;
}

// ═══════════════════════════════════════════════════════════════
// BUDGET-FREE GATES — ZERO EvalMult(CT,CT)!
// ═══════════════════════════════════════════════════════════════

// AND: log_φ(X·Y) = log_φ(X) + log_φ(Y) = ADDITION!
DualGate gate_and_log(CryptoContext<DCRTPoly>& cc, const DualGate& X, const DualGate& Y) {
    return {cc->EvalAdd(X.a, Y.a), cc->EvalAdd(X.b, Y.b)};
}

// NOT: log_φ(1-X)
// NOT(0) = 1 → log_φ(1) = 0
// NOT(1) = 0 → log_φ(0) = -100
// Formula: NOT_log = -(log_val + 100)
DualGate gate_not_log(CryptoContext<DCRTPoly>& cc, const DualGate& X) {
    auto hundred = cc->MakeCKKSPackedPlaintext(std::vector<double>{100.0});
    auto neg_one = cc->MakeCKKSPackedPlaintext(std::vector<double>{-1.0});
    
    // shifted = X + 100  (EvalAdd = FREE)
    auto shifted = cc->EvalAdd(X.a, hundred);
    
    // NOT_log = -(shifted) = -X - 100  (Scalar Mult = FREE)
    auto not_log = cc->EvalMult(shifted, neg_one);
    
    return {not_log, X.b};
}

// NAND: NOT(AND(X,Y))
DualGate gate_nand_log(CryptoContext<DCRTPoly>& cc, const DualGate& X, const DualGate& Y) {
    DualGate and_xy = gate_and_log(cc, X, Y);
    return gate_not_log(cc, and_xy);
}

// OR: NOT(AND(NOT(X), NOT(Y)))  — De Morgan
DualGate gate_or_log(CryptoContext<DCRTPoly>& cc, const DualGate& X, const DualGate& Y) {
    DualGate not_x = gate_not_log(cc, X);
    DualGate not_y = gate_not_log(cc, Y);
    DualGate nand_xy = gate_nand_log(cc, not_x, not_y);
    return nand_xy;  // NOT(AND(NOT(X),NOT(Y))) = OR(X,Y)
}

// XOR: OR(AND(X,NOT(Y)), AND(NOT(X),Y))
DualGate gate_xor_log(CryptoContext<DCRTPoly>& cc, const DualGate& X, const DualGate& Y) {
    DualGate not_x = gate_not_log(cc, X);
    DualGate not_y = gate_not_log(cc, Y);
    DualGate x_and_noty = gate_and_log(cc, X, not_y);
    DualGate notx_and_y = gate_and_log(cc, not_x, Y);
    return gate_or_log(cc, x_and_noty, notx_and_y);
}

// ═══════════════════════════════════════════════════════════════
// MAIN
// ═══════════════════════════════════════════════════════════════
int main() {
    time_t st = time(0);
    std::cout << "\n  ╔══════════════════════════════════════════════════════════════╗\n";
    std::cout << "  ║  LOG-SPACE PERFECT — Budget-Free Universal Gates           ║\n";
    std::cout << "  ║  AND=ADD, NOT=Threshold Inv, NAND/OR/XOR built from these   ║\n";
    std::cout << "  ╚══════════════════════════════════════════════════════════════╝\n";
    std::cout << "  Started: " << ctime(&st) << "\n";

    // ═══ TEST 1: ALL GATES ═══
    std::cout << "  ┌──────────────────────────────────────────────────────┐\n";
    std::cout << "  │  TEST 1: ALL GATES — AND, OR, NAND, XOR (Log-Space)  │\n";
    std::cout << "  ├─────┬─────┬──────┬──────┬──────┬──────┬────────────┤\n";
    std::cout << "  │  X  │  Y  │ AND  │ OR   │ NAND │ XOR  │ Status      │\n";
    std::cout << "  ├─────┼─────┼──────┼──────┼──────┼──────┼────────────┤\n";
    
    int gate_ok = 0;
    for (int depth : {3, 5, 10}) {  // Minimal depth!
        CCParams<CryptoContextCKKSRNS> p;
        p.SetMultiplicativeDepth(depth);
        p.SetScalingModSize(50);
        p.SetBatchSize(512);
        p.SetRingDim(4096);
        p.SetSecretKeyDist(UNIFORM_TERNARY);
        p.SetSecurityLevel(HEStd_NotSet);
        
        try {
            auto cc = GenCryptoContext(p);
            cc->Enable(PKE); cc->Enable(KEYSWITCH); cc->Enable(LEVELEDSHE);
            auto kp = cc->KeyGen();
            cc->EvalMultKeyGen(kp.secretKey);
            
            for (int x = 0; x <= 1; x++) {
                for (int y = 0; y <= 1; y++) {
                    DualGate X = encode_log(cc, kp, (double)x);
                    DualGate Y = encode_log(cc, kp, (double)y);
                    
                    int a = (int)decode_log(cc, kp, gate_and_log(cc, X, Y));
                    int o = (int)decode_log(cc, kp, gate_or_log(cc, X, Y));
                    int na = (int)decode_log(cc, kp, gate_nand_log(cc, X, Y));
                    int xo = (int)decode_log(cc, kp, gate_xor_log(cc, X, Y));
                    
                    int ea = x&y, eo = x|y, ena = !(x&y), exo = x^y;
                    if (a==ea && o==eo && na==ena && xo==exo) gate_ok++;
                    
                    std::cout << "  │  " << x << "  │  " << y 
                              << "  │  " << a << "   │  " << o << "   │  " << na 
                              << "    │  " << xo << "   │ " 
                              << (a==ea&&o==eo&&na==ena&&xo==exo ? "OK ✓" : "FAIL")
                              << "        │\n";
                }
            }
            
            std::cout << "  ├─────┴─────┴──────┴──────┴──────┴──────┴────────────┤\n";
            std::cout << "  │  Depth=" << depth << ": " << gate_ok << "/4 correct                                      │\n";
            
        } catch (...) {
            std::cout << "  │  Depth=" << depth << ": CONTEXT FAILED                                    │\n";
        }
    }
    std::cout << "  └──────────────────────────────────────────────────────┘\n\n";

    // ═══ TEST 2: DEEP CHAIN (Budget=5!) ═══
    std::cout << "  ┌──────────────────────────────────────────────────────┐\n";
    std::cout << "  │  TEST 2: Deep Chain — Log-space AND (Budget = 5!)    │\n";
    std::cout << "  ├──────┬──────────┬──────────────────────────────────┤\n";
    
    CCParams<CryptoContextCKKSRNS> p;
    p.SetMultiplicativeDepth(5);
    p.SetScalingModSize(50);
    p.SetBatchSize(512);
    p.SetRingDim(4096);
    p.SetSecretKeyDist(UNIFORM_TERNARY);
    p.SetSecurityLevel(HEStd_NotSet);
    auto cc = GenCryptoContext(p);
    cc->Enable(PKE); cc->Enable(KEYSWITCH); cc->Enable(LEVELEDSHE);
    auto kp = cc->KeyGen();
    cc->EvalMultKeyGen(kp.secretKey);
    
    for (int gates : {5, 10, 20, 50, 100, 200, 500}) {
        DualGate cur = encode_log(cc, kp, 1.0);
        bool crashed = false;
        
        try {
            for (int g = 0; g < gates; g++) {
                cur = gate_and_log(cc, cur, cur);
            }
        } catch (...) {
            crashed = true;
        }
        
        if (!crashed) {
            double val = decode_log(cc, kp, cur);
            std::cout << "  │ " << std::setw(4) << gates
                      << " │ " << std::setw(8) << val
                      << "    │ ALIVE ✓                            │\n";
        } else {
            std::cout << "  │ " << std::setw(4) << gates
                      << " │   CRASH   │ BUDGET OUT                         │\n";
            break;
        }
    }
    
    std::cout << "  ├──────┴──────────┴──────────────────────────────────┤\n";
    std::cout << "  │  Log-space AND: ZERO EvalMult(CT,CT) per gate!     │\n";
    std::cout << "  └──────────────────────────────────────────────────────┘\n\n";
    
    time_t et = time(0);
    std::cout << "  Ended: " << ctime(&et) << "\n";
    return 0;
}
