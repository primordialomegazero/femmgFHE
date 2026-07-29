// φ-MIRROR — Built-in mirroring via φ/ψ scaling
// Architecture: Dan Fernandez / Primordial Omega Zero
#include <iostream>
#include <iomanip>
#include <cmath>
#include <sys/time.h>
#include "openfhe.h"
using namespace lbcrypto;

const double PHI = 1.6180339887498948482;
const double PSI = -0.6180339887498948482;

struct DualGate { Ciphertext<DCRTPoly> a; Ciphertext<DCRTPoly> b; };

DualGate observe_and(CryptoContext<DCRTPoly>& cc, const DualGate& X, const DualGate& Y) {
    auto a_out = cc->EvalMult(X.a, Y.a);
    auto sum = cc->EvalAdd(cc->EvalAdd(cc->EvalMult(X.a, Y.b), cc->EvalMult(X.b, Y.a)), cc->EvalMult(X.b, Y.b));
    auto neg_one = cc->MakeCKKSPackedPlaintext(std::vector<double>{-1.0});
    return {a_out, cc->EvalMult(neg_one, sum)};
}

DualGate observe_or(CryptoContext<DCRTPoly>& cc, const DualGate& X, const DualGate& Y) {
    auto one = cc->MakeCKKSPackedPlaintext(std::vector<double>{1.0});
    auto oma1 = cc->EvalSub(one, X.a), oma2 = cc->EvalSub(one, Y.a);
    auto a_out = cc->EvalSub(one, cc->EvalMult(oma1, oma2));
    auto b_out = cc->EvalAdd(cc->EvalAdd(cc->EvalMult(oma1, Y.b), cc->EvalMult(X.b, oma2)), cc->EvalMult(X.b, Y.b));
    return {a_out, b_out};
}

// φ-MIRROR: Lightweight self-stabilization
DualGate phi_mirror(CryptoContext<DCRTPoly>& cc, const DualGate& gate) {
    auto phi_pt = cc->MakeCKKSPackedPlaintext(std::vector<double>{PHI});
    auto psi_pt = cc->MakeCKKSPackedPlaintext(std::vector<double>{PSI});
    auto a_new = cc->EvalMult(gate.a, phi_pt);
    auto b_new = cc->EvalMult(gate.b, psi_pt);
    return {a_new, b_new};
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
    std::cout << "  ║  φ-MIRROR — Built-in self-stabilization                     ║\n";
    std::cout << "  ╚══════════════════════════════════════════════════════════════╝\n";
    std::cout << "  Started: " << ctime(&st) << "\n";

    CCParams<CryptoContextCKKSRNS> p;
    p.SetMultiplicativeDepth(120); p.SetScalingModSize(50); p.SetBatchSize(512);
    p.SetRingDim(8192); p.SetSecretKeyDist(UNIFORM_TERNARY); p.SetSecurityLevel(HEStd_NotSet);
    auto cc = GenCryptoContext(p); cc->Enable(PKE); cc->Enable(KEYSWITCH); cc->Enable(LEVELEDSHE);
    auto kp = cc->KeyGen(); cc->EvalMultKeyGen(kp.secretKey);
    
    int inputs[8][3] = {{0,0,0},{0,0,1},{0,1,0},{0,1,1},{1,0,0},{1,0,1},{1,1,0},{1,1,1}};

    // TEST: Chain with φ-mirror every N gates
    std::cout << "  ┌──────────────────────────────────────────────────────┐\n";
    std::cout << "  │  φ-MIRROR CHAIN — Mirror every N gates               │\n";
    std::cout << "  ├──────┬──────────┬──────────┬──────────────────────┤\n";
    std::cout << "  │ Mirror│  Gates   │ Correct  │ Status                │\n";
    std::cout << "  ├──────┼──────────┼──────────┼──────────────────────┤\n";
    
    for (int interval : {0, 1, 2, 3, 5}) {
        int correct = 0;
        bool crashed = false;
        
        for (int i = 0; i < 8 && !crashed; i++) {
            std::vector<double> dv = {(double)inputs[i][0], (double)inputs[i][1], (double)inputs[i][2]};
            DualGate x=make_input(cc,kp,dv[0]), y=make_input(cc,kp,dv[1]), z=make_input(cc,kp,dv[2]);
            
            try {
                DualGate cp = observe_and(cc, x, y);
                DualGate cs = observe_or(cc, x, z);
                
                for (int g = 1; g <= 20; g++) {
                    cp = observe_and(cc, cp, cs);
                    cs = observe_or(cc, cs, cp);
                    
                    if (interval > 0 && g % interval == 0) {
                        cp = phi_mirror(cc, cp);
                        cs = phi_mirror(cc, cs);
                    }
                }
                
                DualGate result = observe_or(cc, cp, z);
                int expected = (inputs[i][0] & inputs[i][1]) | inputs[i][2];
                int bit = (reveal(result, cc, kp, PHI) > 0.5) ? 1 : 0;
                if (bit == expected) correct++;
            } catch (...) {
                crashed = true;
            }
        }
        
        std::string label = (interval == 0) ? "OFF" : "E" + std::to_string(interval);
        std::cout << "  │ " << std::setw(4) << label
                  << " │ " << std::setw(6) << (crashed ? "CRASH" : "50")
                  << "   │ " << std::setw(6) << correct << "/8"
                  << "   │ " << (crashed ? "BUDGET OUT" : correct == 8 ? "PERFECT ✓✓✓" : 
                                  correct >= 6 ? "GOOD ✓" : "DEGRADED")
                  << "            │\n";
    }
    
    std::cout << "  ├──────┴──────────┴──────────┴──────────────────────┤\n";
    std::cout << "  │  φ-mirror: a*=φ (signal boost), b*=ψ (noise damp)   │\n";
    std::cout << "  └──────────────────────────────────────────────────────┘\n\n";
    
    time_t et = time(0);
    std::cout << "  Ended: " << ctime(&et) << "\n";
    return 0;
}
