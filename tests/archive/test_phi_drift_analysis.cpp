// Drift Analysis: Trace what changes between cycle 0 and cycle 1
#include <iostream>
#include <iomanip>
#include <cmath>
#include "openfhe.h"
using namespace lbcrypto;

struct PE { Ciphertext<DCRTPoly> a; Ciphertext<DCRTPoly> b; };

PE mulY(CryptoContext<DCRTPoly>& cc, const PE& x) {
    return {x.b, cc->EvalAdd(x.a, x.b)};
}
PE ratio_add(CryptoContext<DCRTPoly>& cc, const PE& x, const PE& y) {
    auto a1b2 = cc->EvalMult(x.a, y.b);
    auto a2b1 = cc->EvalMult(y.a, x.b);
    auto b1b2 = cc->EvalMult(x.b, y.b);
    return {cc->EvalAdd(a1b2, a2b1), b1b2};
}
PE ratio_mult(CryptoContext<DCRTPoly>& cc, const PE& x, const PE& y) {
    return {cc->EvalMult(x.a, y.a), cc->EvalMult(x.b, y.b)};
}

double get_val(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, const Ciphertext<DCRTPoly>& c) {
    Plaintext pt; cc->Decrypt(kp.secretKey, c, &pt);
    return pt->GetCKKSPackedValue()[0].real();
}
PE make_state(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, double value) {
    const double PSI = 0.6180339887498949;
    auto pta = cc->MakeCKKSPackedPlaintext(std::vector<double>{value + PSI});
    auto ptb = cc->MakeCKKSPackedPlaintext(std::vector<double>{1.0});
    return {cc->Encrypt(kp.publicKey, pta), cc->Encrypt(kp.publicKey, ptb)};
}

const double PSI = 0.6180339887498949;
const double PSI2 = PSI * PSI;

long long fib(int n) {
    if (n <= 1) return n;
    long long a = 0, b = 1;
    for (int i = 2; i <= n; i++) { long long t = a+b; a = b; b = t; }
    return b;
}

double correct_add_then_mul(double raw, double sum_v, double vC, int N_adds) {
    return raw - sum_v * PSI - N_adds * PSI * vC - N_adds * PSI2 + PSI - PSI * (PSI + vC);
}

double exact_recover(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp,
                     double value, int steps = 8) {
    PE state = make_state(cc, kp, value);
    for (int i = 0; i < steps; i++) state = mulY(cc, state);
    double b_val = get_val(cc, kp, state.b);
    return (b_val - fib(steps + 1)) / fib(steps) - PSI;
}

int main() {
    std::cout << "\n  ╔══════════════════════════════════════════════════════════╗\n";
    std::cout <<   "  ║   Drift Analysis: Why does iteration drift?              ║\n";
    std::cout <<   "  ║   Compare: fresh input vs normalized input               ║\n";
    std::cout <<   "  ╚══════════════════════════════════════════════════════════╝\n\n";
    
    CCParams<CryptoContextCKKSRNS> p;
    p.SetMultiplicativeDepth(100);
    p.SetScalingModSize(50);
    p.SetBatchSize(2048);
    p.SetRingDim(4096);
    p.SetSecretKeyDist(UNIFORM_TERNARY);
    p.SetSecurityLevel(HEStd_NotSet);
    
    auto cc = GenCryptoContext(p);
    cc->Enable(PKE); cc->Enable(KEYSWITCH); cc->Enable(LEVELEDSHE); cc->Enable(ADVANCEDSHE);
    auto kp = cc->KeyGen();
    cc->EvalMultKeyGen(kp.secretKey);
    
    std::cout << std::fixed << std::setprecision(10);
    
    // === FRESH computation ===
    std::cout << "  === CYCLE 0: Fresh inputs ===\n";
    PE A0 = make_state(cc, kp, 0.5);
    PE B0 = make_state(cc, kp, 0.3);
    PE C0 = make_state(cc, kp, 0.2);
    
    std::cout << "  A: a=" << get_val(cc, kp, A0.a) << " b=" << get_val(cc, kp, A0.b) 
              << " decoded=" << get_val(cc, kp, A0.a)/get_val(cc, kp, A0.b) - PSI << "\n";
    std::cout << "  B: a=" << get_val(cc, kp, B0.a) << " b=" << get_val(cc, kp, B0.b) 
              << " decoded=" << get_val(cc, kp, B0.a)/get_val(cc, kp, B0.b) - PSI << "\n";
    
    PE sum0 = ratio_add(cc, A0, B0);
    PE res0 = ratio_mult(cc, sum0, C0);
    double raw0 = get_val(cc, kp, res0.a) / get_val(cc, kp, res0.b) - PSI;
    double corr0 = correct_add_then_mul(raw0, 0.8, 0.2, 1);
    double norm0 = exact_recover(cc, kp, corr0, 8);
    std::cout << "  raw=" << raw0 << " corrected=" << corr0 << " normalized=" << norm0 << "\n\n";
    
    // === CYCLE 1: Using normalized value as input ===
    std::cout << "  === CYCLE 1: Using normalized(" << norm0 << ") as input ===\n";
    PE A1 = make_state(cc, kp, norm0);  // <-- THIS IS THE PROBLEM!
    PE B1 = make_state(cc, kp, 0.3);
    PE C1 = make_state(cc, kp, 0.2);
    
    std::cout << "  A1: a=" << get_val(cc, kp, A1.a) << " b=" << get_val(cc, kp, A1.b) 
              << " decoded=" << get_val(cc, kp, A1.a)/get_val(cc, kp, A1.b) - PSI << "\n";
    std::cout << "  (A1 should have decoded=" << norm0 << ")\n\n";
    
    // Check: is A1 REALLY encoding norm0 correctly?
    double a1_decoded = get_val(cc, kp, A1.a) / get_val(cc, kp, A1.b) - PSI;
    std::cout << "  A1 decoded = " << a1_decoded << " (expected " << norm0 << ")\n";
    std::cout << "  Error in encoding: " << std::scientific << std::abs(a1_decoded - norm0) << "\n\n";
    
    PE sum1 = ratio_add(cc, A1, B1);
    PE res1 = ratio_mult(cc, sum1, C1);
    double raw1 = get_val(cc, kp, res1.a) / get_val(cc, kp, res1.b) - PSI;
    
    std::cout << "  After ratio_add(A1,B1): a=" << get_val(cc, kp, sum1.a) << " b=" << get_val(cc, kp, sum1.b) << "\n";
    std::cout << "  sum1 decoded = " << get_val(cc, kp, sum1.a)/get_val(cc, kp, sum1.b) - PSI << "\n";
    std::cout << "  (Expected sum1 decoded = " << norm0 + 0.3 << ")\n\n";
    
    std::cout << "  After ratio_mult(sum1,C1): a=" << get_val(cc, kp, res1.a) << " b=" << get_val(cc, kp, res1.b) << "\n";
    std::cout << "  raw1 = " << raw1 << "\n";
    
    double corr1 = correct_add_then_mul(raw1, norm0 + 0.3, 0.2, 1);
    std::cout << "  corrected = " << corr1 << " (expected " << norm0 * 0.2 + 0.3*0.2 << " = " << (norm0+0.3)*0.2 << ")\n";
    
    double norm1 = exact_recover(cc, kp, corr1, 8);
    std::cout << "  normalized = " << norm1 << "\n\n";
    
    // === COMPARE: What if we use the CORRECT expected sum? ===
    std::cout << "  === DIAGNOSIS ===\n";
    std::cout << "  correct_add_then_mul uses sum_v = " << norm0 + 0.3 << "\n";
    std::cout << "  But the ACTUAL sum after ratio_add is: " << get_val(cc, kp, sum1.a)/get_val(cc, kp, sum1.b) - PSI << "\n";
    std::cout << "  These should be equal but they're DIFFERENT!\n";
    std::cout << "  That's why the correction formula fails after cycle 0.\n\n";
    
    return 0;
}
