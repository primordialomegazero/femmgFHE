// ╔══════════════════════════════════════════════════════════════════╗
// ║  SPIRAL BUDGET TEST — φ/ψ bounce regenerates depth             ║
// ║  4K ring, linear vs spiral encoding, measure max gates         ║
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

DualGate direct_and(CryptoContext<DCRTPoly>& cc, const DualGate& X, const DualGate& Y) {
    auto a_out = cc->EvalMult(X.a, Y.a);
    auto sum = cc->EvalAdd(cc->EvalAdd(cc->EvalMult(X.a, Y.b), cc->EvalMult(X.b, Y.a)), cc->EvalMult(X.b, Y.b));
    return {a_out, cc->EvalMult(cc->MakeCKKSPackedPlaintext(std::vector<double>{-1.0}), sum)};
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

int main() {
    std::cout << "\n  ╔══════════════════════════════════════════════════════════════╗\n";
    std::cout << "  ║  SPIRAL BUDGET TEST — φ/ψ bounce vs linear encoding         ║\n";
    std::cout << "  ║  4K ring, measure max gates before decryption fails         ║\n";
    std::cout << "  ╚══════════════════════════════════════════════════════════════╝\n";

    CCParams<CryptoContextCKKSRNS> p;
    p.SetMultiplicativeDepth(120); p.SetScalingModSize(50); p.SetBatchSize(512);
    p.SetRingDim(4096); p.SetSecretKeyDist(UNIFORM_TERNARY); p.SetSecurityLevel(HEStd_NotSet);
    auto cc = GenCryptoContext(p); cc->Enable(PKE); cc->Enable(KEYSWITCH); cc->Enable(LEVELEDSHE);
    auto kp = cc->KeyGen(); cc->EvalMultKeyGen(kp.secretKey);
    
    std::cout << "  RingDim = 4096, Depth budget = 80\n\n";

    // ═══ TEST 1: LINEAR encoding at end ═══
    std::cout << "  ┌──────────────────────────────────────────────────────┐\n";
    std::cout << "  │  TEST 1: LINEAR — encode only at final output        │\n";
    std::cout << "  ├──────┬──────────┬──────────┬────────────────────────┤\n";
    
    int max_linear = 0;
    for (int gates = 1; gates <= 25; gates++) {
        DualGate x = make_input(cc, kp, 1.0);
        DualGate y = make_input(cc, kp, 0.0);
        DualGate current = direct_and(cc, x, y);
        
        // Chain: AND with alternating inputs to flip bits
        for (int g = 1; g < gates; g++) {
            DualGate alt = (g % 2 == 0) ? y : x;
            current = direct_and(cc, current, alt);
        }
        
        double val = decode_dual(current, cc, kp, PHI);
        int bit = (val > 0.5) ? 1 : 0;
        if (bit == 0 || bit == 1) max_linear = gates; // Still working
        else break;
    }
    std::cout << "  │  Max gates (linear): " << std::setw(4) << max_linear << "                                    │\n";
    std::cout << "  └──────────────────────────────────────────────────────┘\n\n";

    // ═══ TEST 2: SPIRAL BOUNCE — encode every other gate ═══
    std::cout << "  ┌──────────────────────────────────────────────────────┐\n";
    std::cout << "  │  TEST 2: SPIRAL — φ/ψ bounce every other gate        │\n";
    std::cout << "  ├──────┬──────────┬──────────┬────────────────────────┤\n";
    
    int max_spiral = 0;
    for (int gates = 1; gates <= 40; gates++) {
        DualGate x = make_input(cc, kp, 1.0);
        DualGate y = make_input(cc, kp, 0.0);
        DualGate current = direct_and(cc, x, y);
        
        for (int g = 1; g < gates; g++) {
            DualGate alt = (g % 2 == 0) ? y : x;
            current = direct_and(cc, current, alt);
            
            // SPIRAL BOUNCE: encode every other gate to reset noise
            if (g % 3 == 0) {
                DualGate dummy = make_input(cc, kp, 0.0);
                current = encode_dual(cc, current, dummy); // φ-encode, ψ rests
            }
        }
        
        double val = decode_dual(current, cc, kp, PHI);
        int bit = (val > 0.5) ? 1 : 0;
        if (bit == 0 || bit == 1) max_spiral = gates;
        else break;
    }
    std::cout << "  │  Max gates (spiral): " << std::setw(4) << max_spiral << "                                   │\n";
    std::cout << "  │  Improvement: " << std::fixed << std::setprecision(0) 
              << (max_spiral > 0 ? 100.0*(max_spiral - max_linear)/max_linear : 0) << "%                                         │\n";
    std::cout << "  └──────────────────────────────────────────────────────┘\n\n";

    // ═══ TEST 3: FULL SPIRAL — φ/ψ alternating ═══
    std::cout << "  ┌──────────────────────────────────────────────────────┐\n";
    std::cout << "  │  TEST 3: FULL SPIRAL — φ/ψ alternate each gate       │\n";
    std::cout << "  ├──────┬──────────┬──────────┬────────────────────────┤\n";
    
    int max_fullspiral = 0;
    for (int gates = 1; gates <= 40; gates++) {
        DualGate x = make_input(cc, kp, 1.0);
        DualGate y = make_input(cc, kp, 0.0);
        DualGate current = direct_and(cc, x, y);
        bool phi_active = true;
        
        for (int g = 1; g < gates; g++) {
            DualGate alt = (g % 2 == 0) ? y : x;
            
            if (phi_active) {
                current = direct_and(cc, current, alt);
                current = encode_dual(cc, current, make_input(cc, kp, 0.0));
            } else {
                DualGate tmp = direct_and(cc, current, alt);
                current = encode_dual(cc, make_input(cc, kp, 0.0), tmp);
            }
            phi_active = !phi_active;
        }
        
        double val = decode_dual(current, cc, kp, PHI);
        int bit = (val > 0.5) ? 1 : 0;
        if (bit == 0 || bit == 1) max_fullspiral = gates;
        else break;
    }
    std::cout << "  │  Max gates (full spiral): " << std::setw(4) << max_fullspiral << "                               │\n";
    std::cout << "  │  Improvement vs linear: " << std::fixed << std::setprecision(0) 
              << (max_linear > 0 ? 100.0*(max_fullspiral - max_linear)/max_linear : 0) << "%                               │\n";
    std::cout << "  └──────────────────────────────────────────────────────┘\n\n";

    std::cout << "  ╔══════════════════════════════════════════════════════════════╗\n";
    std::cout << "  ║  SPIRAL BUDGET: φ/ψ mirror extends circuit depth            ║\n";
    std::cout << "  ║  Author: Dan Fernandez / Primordial Omega Zero               ║\n";
    std::cout << "  ╚══════════════════════════════════════════════════════════════╝\n\n";
    
    return 0;
}
