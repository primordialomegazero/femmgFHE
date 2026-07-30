// PURE CKKS CHAIN — No decrypt, no reset, no cleaning
// Just observer gates with built-in mirror. How deep can it go?
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
    return {a_out, cc->EvalMult(cc->MakeCKKSPackedPlaintext(std::vector<double>{-1.0}), sum)};
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
    std::cout << "\n  ╔══════════════════════════════════════════════╗\n";
    std::cout << "  ║  PURE CKKS CHAIN — No Decrypt, No Reset     ║\n";
    std::cout << "  ╚══════════════════════════════════════════════╝\n";
    std::cout << "  Started: " << ctime(&st) << "\n";

    for (int ringdim : {4096, 8192}) {
        for (int depth : {30, 60, 120}) {
            std::cout << "  ┌────────────────────────────────────────────┐\n";
            std::cout << "  │  RingDim=" << ringdim << ", Depth=" << depth << "                            │\n";
            std::cout << "  ├──────┬──────────┬────────────────────────┤\n";
            
            try {
                CCParams<CryptoContextCKKSRNS> p;
                p.SetMultiplicativeDepth(depth); p.SetScalingModSize(50); p.SetBatchSize(512);
                p.SetRingDim(ringdim); p.SetSecretKeyDist(UNIFORM_TERNARY); p.SetSecurityLevel(HEStd_NotSet);
                auto cc = GenCryptoContext(p); cc->Enable(PKE); cc->Enable(KEYSWITCH); cc->Enable(LEVELEDSHE);
                auto kp = cc->KeyGen(); cc->EvalMultKeyGen(kp.secretKey);
                
                DualGate cur = make_input(cc, kp, 1.0);
                int max_gates = 0;
                bool alive = true;
                
                for (int g = 1; g <= 500 && alive; g++) {
                    try {
                        DualGate one = make_input(cc, kp, 1.0);
                        cur = observe_and(cc, cur, one);
                        max_gates = g;
                    } catch (...) {
                        alive = false;
                    }
                }
                
                double val = reveal(cur, cc, kp, PHI);
                std::cout << "  │ " << std::setw(4) << max_gates 
                          << " │ " << std::fixed << std::setprecision(4) << std::setw(8) << val
                          << " │ " << (alive ? "ALIVE ✓" : "BUDGET OUT")
                          << "              │\n";
                
            } catch (...) {
                std::cout << "  │ FAIL │   -    │ CONTEXT FAILED          │\n";
            }
            
            std::cout << "  └──────┴──────────┴────────────────────────┘\n";
        }
    }
    
    time_t et = time(0);
    std::cout << "\n  Ended: " << ctime(&et) << "\n";
    return 0;
}
