// ╔══════════════════════════════════════════════════════════════════╗
// ║  PURE FHE CHAIN — No Decrypt, No Re-encrypt                    ║
// ║  Homomorphic threshold cleaning via polynomial approximation    ║
// ║  Architecture: Dan Fernandez / Primordial Omega Zero            ║
// ╚══════════════════════════════════════════════════════════════════╝
#include <iostream>
#include <iomanip>
#include <cmath>
#include <sys/time.h>
#include "openfhe.h"
using namespace lbcrypto;

const double PHI = 1.6180339887498948482;

struct DualGate { Ciphertext<DCRTPoly> a; Ciphertext<DCRTPoly> b; };

// ═══════════════════════════════════════════════════════════════
// HOMOMORPHIC THRESHOLD CLEANING
// ═══════════════════════════════════════════════════════════════
// Approximate step function: f(x) ≈ 1/(1 + e^(-k*(x-0.5)))
// For k=20: f(0)≈0, f(1)≈1, sharp transition at 0.5
// Polynomial approximation: 0.5 + 0.5*tanh(k*(x-0.5))

Ciphertext<DCRTPoly> clean_bool_fhe(CryptoContext<DCRTPoly>& cc, 
                                     const Ciphertext<DCRTPoly>& c) {
    // Step 1: Shift by -0.5
    auto half = cc->MakeCKKSPackedPlaintext(std::vector<double>{0.5});
    auto shifted = cc->EvalSub(c, half);  // x - 0.5
    
    // Step 2: Scale by k=10 (steepness)
    auto k = cc->MakeCKKSPackedPlaintext(std::vector<double>{10.0});
    auto scaled = cc->EvalMult(shifted, k);  // k*(x-0.5)
    
    // Step 3: Polynomial approximation of tanh
    // tanh(z) ≈ z - z³/3 + 2z⁵/15 (Taylor series)
    auto z2 = cc->EvalMult(scaled, scaled);         // z²
    auto z3 = cc->EvalMult(z2, scaled);             // z³
    auto z5 = cc->EvalMult(z3, z2);                 // z⁵
    
    auto third = cc->MakeCKKSPackedPlaintext(std::vector<double>{-1.0/3.0});
    auto term1 = cc->EvalMult(z3, third);            // -z³/3
    
    auto fifth = cc->MakeCKKSPackedPlaintext(std::vector<double>{2.0/15.0});
    auto term2 = cc->EvalMult(z5, fifth);            // 2z⁵/15
    
    auto tanh_approx = cc->EvalAdd(scaled, cc->EvalAdd(term1, term2));
    
    // Step 4: Scale back to [0,1]: (tanh + 1) / 2
    auto one = cc->MakeCKKSPackedPlaintext(std::vector<double>{1.0});
    auto plus_one = cc->EvalAdd(tanh_approx, one);
    auto half_pt = cc->MakeCKKSPackedPlaintext(std::vector<double>{0.5});
    auto result = cc->EvalMult(plus_one, half_pt);
    
    return result;
}

// ═══════════════════════════════════════════════════════════════
// OBSERVER GATE — Built-in mirror
// ═══════════════════════════════════════════════════════════════
DualGate gate_and(CryptoContext<DCRTPoly>& cc, const DualGate& X, const DualGate& Y) {
    auto a_out = cc->EvalMult(X.a, Y.a);
    auto sum = cc->EvalAdd(cc->EvalAdd(cc->EvalMult(X.a, Y.b), cc->EvalMult(X.b, Y.a)), cc->EvalMult(X.b, Y.b));
    return {a_out, cc->EvalMult(cc->MakeCKKSPackedPlaintext(std::vector<double>{-1.0}), sum)};
}

// ═══════════════════════════════════════════════════════════════
// UTILITY
// ═══════════════════════════════════════════════════════════════
DualGate make_input(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, double val) {
    return {cc->Encrypt(kp.publicKey, cc->MakeCKKSPackedPlaintext(std::vector<double>{val})),
            cc->Encrypt(kp.publicKey, cc->MakeCKKSPackedPlaintext(std::vector<double>{0.0}))};
}

double decrypt_val(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, 
                   const Ciphertext<DCRTPoly>& c) {
    Plaintext pt; cc->Decrypt(kp.secretKey, c, &pt);
    return pt->GetCKKSPackedValue()[0].real();
}

// ═══════════════════════════════════════════════════════════════
// MAIN
// ═══════════════════════════════════════════════════════════════
int main() {
    time_t st = time(0);
    std::cout << "\n  ╔══════════════════════════════════════════════╗\n";
    std::cout << "  ║  PURE FHE CHAIN — No Decrypt, No Re-encrypt ║\n";
    std::cout << "  ╚══════════════════════════════════════════════╝\n";
    std::cout << "  Started: " << ctime(&st) << "\n\n";

    // ═══ TEST 1: Clean Function Accuracy ═══
    std::cout << "  ┌────────────────────────────────────────────┐\n";
    std::cout << "  │  TEST 1: clean_bool_fhe() accuracy         │\n";
    std::cout << "  ├──────┬──────────┬────────────────────────┤\n";
    std::cout << "  │ Input│ Cleaned  │ Status                  │\n";
    std::cout << "  ├──────┼──────────┼────────────────────────┤\n";
    
    CCParams<CryptoContextCKKSRNS> p;
    p.SetMultiplicativeDepth(30); p.SetScalingModSize(50); p.SetBatchSize(512);
    p.SetRingDim(8192); p.SetSecretKeyDist(UNIFORM_TERNARY); p.SetSecurityLevel(HEStd_NotSet);
    auto cc = GenCryptoContext(p);
    cc->Enable(PKE); cc->Enable(KEYSWITCH); cc->Enable(LEVELEDSHE);
    auto kp = cc->KeyGen(); cc->EvalMultKeyGen(kp.secretKey);
    
    int clean_ok = 0;
    for (double input : {0.0, 0.25, 0.5, 0.75, 1.0}) {
        auto enc = cc->Encrypt(kp.publicKey, cc->MakeCKKSPackedPlaintext(std::vector<double>{input}));
        auto cleaned = clean_bool_fhe(cc, enc);
        double result = decrypt_val(cc, kp, cleaned);
        int expected = (input > 0.5) ? 1 : 0;
        int actual = (result > 0.5) ? 1 : 0;
        if (expected == actual) clean_ok++;
        
        std::cout << "  │ " << std::fixed << std::setprecision(2) << std::setw(4) << input
                  << " │ " << std::setw(8) << result
                  << " │ " << (expected == actual ? "OK ✓" : "FAIL")
                  << "                  │\n";
    }
    std::cout << "  ├──────┴──────────┴────────────────────────┤\n";
    std::cout << "  │  Clean accuracy: " << clean_ok << "/5                              │\n";
    std::cout << "  └────────────────────────────────────────────┘\n\n";

    // ═══ TEST 2: Pure FHE Chain ═══
    std::cout << "  ┌────────────────────────────────────────────┐\n";
    std::cout << "  │  TEST 2: Pure FHE Chain (NO DECRYPT!)      │\n";
    std::cout << "  ├──────┬──────────┬────────────────────────┤\n";
    
    for (int ringdim : {4096, 8192}) {
        for (int depth : {30, 60}) {
            try {
                CCParams<CryptoContextCKKSRNS> p2;
                p2.SetMultiplicativeDepth(depth); p2.SetScalingModSize(50); p2.SetBatchSize(512);
                p2.SetRingDim(ringdim); p2.SetSecretKeyDist(UNIFORM_TERNARY); p2.SetSecurityLevel(HEStd_NotSet);
                auto cc2 = GenCryptoContext(p2);
                cc2->Enable(PKE); cc2->Enable(KEYSWITCH); cc2->Enable(LEVELEDSHE);
                auto kp2 = cc2->KeyGen(); cc2->EvalMultKeyGen(kp2.secretKey);
                
                DualGate cur = make_input(cc2, kp2, 1.0);
                int max_gates = 0;
                
                for (int g = 1; g <= 200; g++) {
                    try {
                        DualGate one = make_input(cc2, kp2, 1.0);
                        cur = gate_and(cc2, cur, one);
                        
                        // Pure FHE cleaning!
                        cur.a = clean_bool_fhe(cc2, cur.a);
                        
                        max_gates = g;
                    } catch (...) { break; }
                }
                
                double final_val = decrypt_val(cc2, kp2, cur.a);
                std::cout << "  │ R" << ringdim << "D" << std::setw(2) << depth
                          << "│ " << std::setw(4) << max_gates
                          << " │ " << std::fixed << std::setprecision(4) << final_val
                          << "   │ " << (std::abs(final_val - 1.0) < 0.1 ? "ALIVE ✓" : "DRIFT")
                          << "              │\n";
                
            } catch (...) {
                std::cout << "  │ R" << ringdim << "D" << std::setw(2) << depth
                          << "│ FAIL │    -     │ CONTEXT FAILED          │\n";
            }
        }
    }
    
    std::cout << "  ├──────┴──────────┴────────────────────────┤\n";
    std::cout << "  │  NO DECRYPT IN LOOP! Pure FHE!             │\n";
    std::cout << "  └────────────────────────────────────────────┘\n\n";
    
    time_t et = time(0);
    std::cout << "  Ended: " << ctime(&et) << "\n";
    return 0;
}
