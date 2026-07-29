// Quick check: actual φ values per input at depth 8
#include <iostream>
#include <iomanip>
#include <cmath>
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
double reveal(DualGate& dg, CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, double root) {
    return decrypt_val(cc, kp, dg.a) + decrypt_val(cc, kp, dg.b) * root;
}
DualGate make_input(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, double val) {
    return {cc->Encrypt(kp.publicKey, cc->MakeCKKSPackedPlaintext(std::vector<double>{val})),
            cc->Encrypt(kp.publicKey, cc->MakeCKKSPackedPlaintext(std::vector<double>{0.0}))};
}

int main() {
    CCParams<CryptoContextCKKSRNS> p;
    p.SetMultiplicativeDepth(50); p.SetScalingModSize(50); p.SetBatchSize(512);
    p.SetRingDim(8192); p.SetSecretKeyDist(UNIFORM_TERNARY); p.SetSecurityLevel(HEStd_NotSet);
    auto cc = GenCryptoContext(p); cc->Enable(PKE); cc->Enable(KEYSWITCH); cc->Enable(LEVELEDSHE);
    auto kp = cc->KeyGen(); cc->EvalMultKeyGen(kp.secretKey);
    
    int inputs[8][3] = {{0,0,0},{0,0,1},{0,1,0},{0,1,1},{1,0,0},{1,0,1},{1,1,0},{1,1,1}};
    
    std::cout << "\n  φ-values per input at depths 0, 4, 8:\n";
    std::cout << "  ┌─────┬─────┬─────┬──────────┬──────────┬──────────┐\n";
    std::cout << "  │  x  │  y  │  z  │ depth 0  │ depth 4  │ depth 8  │\n";
    std::cout << "  ├─────┼─────┼─────┼──────────┼──────────┼──────────┤\n";
    
    for (int depth : {0, 4, 8}) {
        if (depth > 0) std::cout << "  │     │     │     │          │          │          │\n";
        for (int i = 0; i < 8; i++) {
            std::vector<double> dv = {(double)inputs[i][0], (double)inputs[i][1], (double)inputs[i][2]};
            DualGate x=make_input(cc,kp,dv[0]), y=make_input(cc,kp,dv[1]), z=make_input(cc,kp,dv[2]);
            DualGate cp = direct_and(cc, x, y);
            DualGate cs = direct_or(cc, x, z);
            for (int d = 1; d <= depth; d++) {
                cp = direct_and(cc, cp, cs);
                cs = direct_or(cc, cs, cp);
            }
            DualGate po = direct_or(cc, cp, z);
            double v = reveal(po, cc, kp, PHI);
            if (depth == 0) {
                std::cout << "  │  " << inputs[i][0] << "  │  " << inputs[i][1] << "  │  " << inputs[i][2] 
                          << "  │ " << std::fixed << std::setprecision(4) << std::setw(8) << v << "│          │          │\n";
            } else if (depth == 4) {
                std::cout << "  │     │     │     │          │ " << std::setw(8) << v << "│          │\n";
            } else {
                std::cout << "  │     │     │     │          │          │ " << std::setw(8) << v << "│\n";
            }
        }
    }
    std::cout << "  └─────┴─────┴─────┴──────────┴──────────┴──────────┘\n";
    std::cout << "  Perfect: 0.0000 or 1.0000 at all depths\n\n";
    
    return 0;
}
