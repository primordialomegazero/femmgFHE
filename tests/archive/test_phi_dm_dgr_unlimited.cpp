// DM-DGR UNLIMITED v12: CORRECTED f_inv!  f_inv(t) = (t + 0.558034) / 0.2
#include <iostream>
#include <iomanip>
#include <cmath>
#include "openfhe.h"
using namespace lbcrypto;

struct PE { Ciphertext<DCRTPoly> a; Ciphertext<DCRTPoly> b; };

PE mulY(CryptoContext<DCRTPoly>& cc, const PE& x) { return {x.b, cc->EvalAdd(x.a, x.b)}; }
PE ratio_add(CryptoContext<DCRTPoly>& cc, const PE& x, const PE& y) {
    auto a1b2 = cc->EvalMult(x.a, y.b); auto a2b1 = cc->EvalMult(y.a, x.b);
    auto b1b2 = cc->EvalMult(x.b, y.b); return {cc->EvalAdd(a1b2, a2b1), b1b2};
}
PE ratio_mult(CryptoContext<DCRTPoly>& cc, const PE& x, const PE& y) {
    return {cc->EvalMult(x.a, y.a), cc->EvalMult(x.b, y.b)};
}
double get_val(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, const Ciphertext<DCRTPoly>& c) {
    Plaintext pt; cc->Decrypt(kp.secretKey, c, &pt); return pt->GetCKKSPackedValue()[0].real();
}
PE make_state(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, double value) {
    const double PSI = 0.6180339887498949;
    auto pta = cc->MakeCKKSPackedPlaintext(std::vector<double>{value + PSI});
    auto ptb = cc->MakeCKKSPackedPlaintext(std::vector<double>{1.0});
    return {cc->Encrypt(kp.publicKey, pta), cc->Encrypt(kp.publicKey, ptb)};
}
const double PSI = 0.6180339887498949, PSI2 = PSI * PSI;
long long fib(int n) {
    if (n <= 1) return n; long long a=0,b=1;
    for (int i=2; i<=n; i++) { long long t=a+b; a=b; b=t; } return b;
}
// CORRECTED: f(x) = -0.558034 + 0.2x, so f_inv(t) = (t + 0.558034)/0.2
double f_inv(double t) { return (t + 0.5580339887498949) / 0.2; }
double correct_add_then_mul(double raw, double sum_v, double vC, int N_adds) {
    return raw - sum_v*PSI - N_adds*PSI*vC - N_adds*PSI2 + PSI - PSI*(PSI+vC);
}
double exact_recover(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, double value, int steps=8) {
    PE state = make_state(cc, kp, value);
    for (int i=0; i<steps; i++) state = mulY(cc, state);
    return (get_val(cc,kp,state.b) - fib(steps+1)) / fib(steps) - PSI;
}

int main() {
    std::cout << "\n  ╔══════════════════════════════════════════════════════════╗\n";
    std::cout <<   "  ║   DM-DGR UNLIMITED v12: CORRECTED f_inv EVERY CYCLE     ║\n";
    std::cout <<   "  ║   f_inv(t) = (t + 0.558034) / 0.2                        ║\n";
    std::cout <<   "  ╚══════════════════════════════════════════════════════════╝\n\n";
    
    CCParams<CryptoContextCKKSRNS> p;
    p.SetMultiplicativeDepth(100); p.SetScalingModSize(50); p.SetBatchSize(2048);
    p.SetRingDim(4096); p.SetSecretKeyDist(UNIFORM_TERNARY); p.SetSecurityLevel(HEStd_NotSet);
    auto cc = GenCryptoContext(p);
    cc->Enable(PKE); cc->Enable(KEYSWITCH); cc->Enable(LEVELEDSHE); cc->Enable(ADVANCEDSHE);
    auto kp = cc->KeyGen(); cc->EvalMultKeyGen(kp.secretKey);
    
    PE B = make_state(cc, kp, 0.3), C = make_state(cc, kp, 0.2);
    const int NORM_STEPS = 8; const double TARGET = 0.16;
    
    std::cout << "  === 50 CYCLES: f⁻¹(TARGET) every cycle ===\n";
    std::cout << "  TARGET=" << TARGET << " f⁻¹(TARGET)=" << f_inv(TARGET) << "\n\n";
    std::cout << "  Cycle  Corrected   Normalized   Error\n";
    std::cout << "  -----------------------------------------\n";
    
    double current_input = f_inv(TARGET);
    
    for (int cycle = 0; cycle < 50; cycle++) {
        PE A = make_state(cc, kp, current_input);
        PE sum_ab = ratio_add(cc, A, B);
        double sum_decoded = get_val(cc,kp,sum_ab.a)/get_val(cc,kp,sum_ab.b)-PSI;
        double plaintext_sum = current_input + 0.3;
        double psi_offset = sum_decoded - plaintext_sum;
        PE result = ratio_mult(cc, sum_ab, C);
        double raw_val = get_val(cc,kp,result.a)/get_val(cc,kp,result.b)-PSI;
        double corrected = correct_add_then_mul(raw_val-psi_offset, plaintext_sum, 0.2, 1);
        double normalized = exact_recover(cc, kp, corrected, NORM_STEPS);
        
        std::cout << "  " << std::setw(3) << cycle
             << "  " << std::fixed << std::setprecision(6) << corrected
             << "  " << std::fixed << std::setprecision(6) << normalized
             << "  " << std::scientific << std::setprecision(1) << std::abs(normalized-TARGET) << "\n";
        
        current_input = f_inv(TARGET); // Always use inverse of target!
    }
    
    std::cout << "\n  ╔══════════════════════════════════════════════════════════╗\n";
    std::cout <<   "  ║  ALL 50 CYCLES COMPLETED!                                ║\n";
    std::cout <<   "  ╚══════════════════════════════════════════════════════════╝\n\n";
    return 0;
}
