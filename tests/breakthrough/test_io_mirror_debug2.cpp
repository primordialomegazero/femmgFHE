// MIRROR DEBUG 2 — Test the chain loop
#include <iostream>
#include <iomanip>
#include <cmath>
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

DualGate observe_or(CryptoContext<DCRTPoly>& cc, const DualGate& X, const DualGate& Y) {
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
    std::cout << "\n  MIRROR DEBUG 2 — Testing the chain loop\n\n";
    
    CCParams<CryptoContextCKKSRNS> p;
    p.SetMultiplicativeDepth(60); p.SetScalingModSize(50); p.SetBatchSize(512);
    p.SetRingDim(8192); p.SetSecretKeyDist(UNIFORM_TERNARY); p.SetSecurityLevel(HEStd_NotSet);
    auto cc = GenCryptoContext(p); cc->Enable(PKE); cc->Enable(KEYSWITCH); cc->Enable(LEVELEDSHE);
    auto kp = cc->KeyGen(); cc->EvalMultKeyGen(kp.secretKey);
    
    DualGate x = make_input(cc, kp, 1.0);
    DualGate y = make_input(cc, kp, 0.0);
    DualGate z = make_input(cc, kp, 1.0);
    
    std::cout << "  Testing observe_and + observe_or chain WITHOUT mirror:\n";
    DualGate cp = observe_and(cc, x, y);
    DualGate cs = observe_or(cc, x, z);
    
    for (int i = 1; i <= 10; i++) {
        try {
            cp = observe_and(cc, cp, cs);
            cs = observe_or(cc, cs, cp);
            std::cout << "  Gate " << i << ": OK\n" << std::flush;
        } catch (const std::exception& e) {
            std::cout << "  Gate " << i << ": CRASH — " << e.what() << "\n";
            break;
        }
    }
    
    std::cout << "\n  DONE\n";
    return 0;
}
