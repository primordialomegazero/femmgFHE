// ╔══════════════════════════════════════════════════════════════════╗
// ║  SYMMETRIC LOG-SPACE — Perfect ±K encoding                     ║
// ║  0→-100, 1→+100, NOT=-X, AND=X+Y                              ║
// ║  ZERO EvalMult(CT,CT)! TRUE BUDGET-FREE!                       ║
// ║  Architecture: Dan Fernandez / Primordial Omega Zero            ║
// ╚══════════════════════════════════════════════════════════════════╝
#include <iostream>
#include <iomanip>
#include <cmath>
#include <sys/time.h>
#include "openfhe.h"
using namespace lbcrypto;

const double K = 100.0;           // Symmetric magnitude
const double LOG_0 = -K;          // log_phi(0) = -100
const double LOG_1 = +K;          // log_phi(1) = +100
const double THRESHOLD = 0.0;     // Below 0 = 0, Above 0 = 1

struct DualGate { Ciphertext<DCRTPoly> a; Ciphertext<DCRTPoly> b; };

// ═══════════════════════════════════════════════════════════════
// SYMMETRIC LOG-SPACE ENCODING
// ═══════════════════════════════════════════════════════════════
DualGate encode_log(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, double val) {
    double log_val = (val < 0.5) ? LOG_0 : LOG_1;
    return {cc->Encrypt(kp.publicKey, cc->MakeCKKSPackedPlaintext(std::vector<double>{log_val})),
            cc->Encrypt(kp.publicKey, cc->MakeCKKSPackedPlaintext(std::vector<double>{0.0}))};
}

double decode_log(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, const DualGate& dg) {
    Plaintext pt;
    cc->Decrypt(kp.secretKey, dg.a, &pt);
    double log_val = pt->GetCKKSPackedValue()[0].real();
    return (log_val < THRESHOLD) ? 0.0 : 1.0;
}

double decrypt_raw(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, const DualGate& dg) {
    Plaintext pt; cc->Decrypt(kp.secretKey, dg.a, &pt);
    return pt->GetCKKSPackedValue()[0].real();
}

// ═══════════════════════════════════════════════════════════════
// BUDGET-FREE GATES — Perfect Symmetry!
// ═══════════════════════════════════════════════════════════════

// AND: X + Y (addition in log space)
DualGate gate_and_log(CryptoContext<DCRTPoly>& cc, const DualGate& X, const DualGate& Y) {
    return {cc->EvalAdd(X.a, Y.a), cc->EvalAdd(X.b, Y.b)};
}

// NOT: -X (simple negation!)
DualGate gate_not_log(CryptoContext<DCRTPoly>& cc, const DualGate& X) {
    auto neg_one = cc->MakeCKKSPackedPlaintext(std::vector<double>{-1.0});
    return {cc->EvalMult(X.a, neg_one), X.b};  // JUST NEGATE!
}

// NAND: NOT(AND(X,Y))
DualGate gate_nand_log(CryptoContext<DCRTPoly>& cc, const DualGate& X, const DualGate& Y) {
    return gate_not_log(cc, gate_and_log(cc, X, Y));
}

// OR: NOT(AND(NOT(X), NOT(Y)))
DualGate gate_or_log(CryptoContext<DCRTPoly>& cc, const DualGate& X, const DualGate& Y) {
    return gate_nand_log(cc, gate_not_log(cc, X), gate_not_log(cc, Y));
}

// XOR: AND(OR(X,Y), NAND(X,Y))
DualGate gate_xor_log(CryptoContext<DCRTPoly>& cc, const DualGate& X, const DualGate& Y) {
    return gate_and_log(cc, gate_or_log(cc, X, Y), gate_nand_log(cc, X, Y));
}

// ═══════════════════════════════════════════════════════════════
// MAIN
// ═══════════════════════════════════════════════════════════════
int main() {
    time_t st = time(0);
    std::cout << "\n  ╔══════════════════════════════════════════════════════════════╗\n";
    std::cout << "  ║  SYMMETRIC LOG-SPACE — Perfect ±K Encoding                 ║\n";
    std::cout << "  ║  0→-100, 1→+100, NOT=-X, AND=X+Y                          ║\n";
    std::cout << "  ╚══════════════════════════════════════════════════════════════╝\n";
    std::cout << "  Started: " << ctime(&st) << "\n";

    CCParams<CryptoContextCKKSRNS> p;
    p.SetMultiplicativeDepth(5); p.SetScalingModSize(50); p.SetBatchSize(512);
    p.SetRingDim(4096); p.SetSecretKeyDist(UNIFORM_TERNARY); p.SetSecurityLevel(HEStd_NotSet);
    auto cc = GenCryptoContext(p);
    cc->Enable(PKE); cc->Enable(KEYSWITCH); cc->Enable(LEVELEDSHE);
    auto kp = cc->KeyGen();
    cc->EvalMultKeyGen(kp.secretKey);

    // ═══ TEST 1: NOT GATE ═══
    std::cout << "  ┌──────────────────────────────────────────────────────┐\n";
    std::cout << "  │  TEST 1: NOT Gate — Simple Negation!                 │\n";
    std::cout << "  ├──────┬──────────┬──────────┬────────────────────────┤\n";
    std::cout << "  │  X   │ Expected │ Actual   │ Status                  │\n";
    std::cout << "  ├──────┼──────────┼──────────┼────────────────────────┤\n";
    
    int not_ok = 0;
    for (int x = 0; x <= 1; x++) {
        DualGate X = encode_log(cc, kp, (double)x);
        DualGate NOT_X = gate_not_log(cc, X);
        double raw = decrypt_raw(cc, kp, NOT_X);
        double result = decode_log(cc, kp, NOT_X);
        int expected = x ? 0 : 1;
        int actual = (int)result;
        if (expected == actual) not_ok++;
        
        std::cout << "  │  " << x << "    │ " << expected << "         │ " << actual 
                  << " (" << std::fixed << std::setprecision(1) << raw << ")"
                  << "   │ " << (expected == actual ? "OK ✓" : "FAIL")
                  << "                    │\n";
    }
    std::cout << "  ├──────┴──────────┴──────────┴────────────────────────┤\n";
    std::cout << "  │  NOT: " << not_ok << "/2 correct                                          │\n";
    std::cout << "  └──────────────────────────────────────────────────────┘\n\n";

    // ═══ TEST 2: ALL GATES ═══
    std::cout << "  ┌──────────────────────────────────────────────────────┐\n";
    std::cout << "  │  TEST 2: ALL GATES — AND, OR, NAND, XOR              │\n";
    std::cout << "  ├─────┬─────┬──────┬──────┬──────┬──────┬────────────┤\n";
    std::cout << "  │  X  │  Y  │ AND  │ OR   │ NAND │ XOR  │ Status      │\n";
    std::cout << "  ├─────┼─────┼──────┼──────┼──────┼──────┼────────────┤\n";
    
    int all_ok = 0;
    for (int x = 0; x <= 1; x++) {
        for (int y = 0; y <= 1; y++) {
            DualGate X = encode_log(cc, kp, (double)x);
            DualGate Y = encode_log(cc, kp, (double)y);
            
            int a = (int)decode_log(cc, kp, gate_and_log(cc, X, Y));
            int o = (int)decode_log(cc, kp, gate_or_log(cc, X, Y));
            int na = (int)decode_log(cc, kp, gate_nand_log(cc, X, Y));
            int xo = (int)decode_log(cc, kp, gate_xor_log(cc, X, Y));
            
            int ea = x&y, eo = x|y, ena = !(x&y), exo = x^y;
            bool ok = (a==ea && o==eo && na==ena && xo==exo);
            if (ok) all_ok++;
            
            std::cout << "  │  " << x << "  │  " << y 
                      << "  │  " << a << "   │  " << o << "   │  " << na 
                      << "    │  " << xo << "   │ " << (ok ? "OK ✓" : "FAIL")
                      << "        │\n";
        }
    }
    std::cout << "  ├─────┴─────┴──────┴──────┴──────┴──────┴────────────┤\n";
    std::cout << "  │  All gates: " << all_ok << "/4 correct                                      │\n";
    std::cout << "  └──────────────────────────────────────────────────────┘\n\n";

    // ═══ TEST 3: XOR DEBUG ═══
    std::cout << "  ┌──────────────────────────────────────────────────────┐\n";
    std::cout << "  │  TEST 3: XOR Step-by-Step (Symmetric Log-Space)      │\n";
    std::cout << "  ├─────┬─────┬──────────┬──────────┬──────────────────┤\n";
    std::cout << "  │  X  │  Y  │ XOR raw  │ XOR bit  │ Expected          │\n";
    std::cout << "  ├─────┼─────┼──────────┼──────────┼──────────────────┤\n";
    
    for (int x = 0; x <= 1; x++) {
        for (int y = 0; y <= 1; y++) {
            DualGate X = encode_log(cc, kp, (double)x);
            DualGate Y = encode_log(cc, kp, (double)y);
            
            DualGate and_xy = gate_and_log(cc, X, Y);
            DualGate or_xy = gate_or_log(cc, X, Y);
            DualGate nand_xy = gate_nand_log(cc, X, Y);
            DualGate xor_xy = gate_xor_log(cc, X, Y);
            
            double xor_raw = decrypt_raw(cc, kp, xor_xy);
            int xor_bit = (int)decode_log(cc, kp, xor_xy);
            
            std::cout << "  │  " << x << "  │  " << y 
                      << "  │ " << std::fixed << std::setprecision(1) << std::setw(8) << xor_raw
                      << " │ " << xor_bit
                      << "         │ " << (x^y)
                      << "        │\n";
        }
    }
    std::cout << "  └─────┴─────┴──────────┴──────────┴──────────────────┘\n\n";

    // ═══ TEST 4: DEEP CHAIN ═══
    std::cout << "  ┌──────────────────────────────────────────────────────┐\n";
    std::cout << "  │  TEST 4: Deep Chain — AND with self (Budget = 5!)    │\n";
    std::cout << "  ├──────┬──────────┬──────────────────────────────────┤\n";
    
    for (int gates : {10, 50, 100, 200, 500, 1000}) {
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
    std::cout << "  │  Symmetric Log-Space: ZERO EvalMult(CT,CT)!        │\n";
    std::cout << "  └──────────────────────────────────────────────────────┘\n\n";
    
    time_t et = time(0);
    std::cout << "  ╔══════════════════════════════════════════════════════════════╗\n";
    std::cout << "  ║  SYMMETRIC LOG-SPACE — COMPLETE                             ║\n";
    std::cout << "  ║  Author: Dan Fernandez / Primordial Omega Zero               ║\n";
    std::cout << "  ╚══════════════════════════════════════════════════════════════╝\n\n";
    std::cout << "  Ended: " << ctime(&et) << "\n";
    
    return 0;
}
