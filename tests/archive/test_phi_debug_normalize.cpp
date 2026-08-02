// Debug: Trace every step of the normalization pipeline
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

int main() {
    std::cout << "\n  ╔══════════════════════════════════════════════════════════╗\n";
    std::cout <<   "  ║   DEBUG: Trace Normalization Pipeline                    ║\n";
    std::cout <<   "  ║   Step-by-step value inspection                          ║\n";
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
    
    // === STEP 1: Test make_state/get_val roundtrip ===
    std::cout << "  === STEP 1: make_state / get_val roundtrip ===\n";
    double test_vals[] = {0.16, 0.5, 1.0, 2.5};
    for (double v : test_vals) {
        PE s = make_state(cc, kp, v);
        double a_val = get_val(cc, kp, s.a);
        double b_val = get_val(cc, kp, s.b);
        double decoded = a_val / b_val - PSI;
        std::cout << "  Input=" << v << " → a=" << a_val << " b=" << b_val 
                  << " decoded=" << decoded << " error=" << std::scientific << std::abs(decoded - v) << "\n";
    }
    
    // === STEP 2: Full computation trace ===
    std::cout << "\n  === STEP 2: Full (A+B)×C trace ===\n";
    
    PE A = make_state(cc, kp, 0.5);
    PE B = make_state(cc, kp, 0.3);
    PE C = make_state(cc, kp, 0.2);
    
    // Check inputs
    std::cout << "  A: a=" << get_val(cc, kp, A.a) << " b=" << get_val(cc, kp, A.b) 
              << " decoded=" << get_val(cc, kp, A.a)/get_val(cc, kp, A.b) - PSI << "\n";
    std::cout << "  B: a=" << get_val(cc, kp, B.a) << " b=" << get_val(cc, kp, B.b) 
              << " decoded=" << get_val(cc, kp, B.a)/get_val(cc, kp, B.b) - PSI << "\n";
    std::cout << "  C: a=" << get_val(cc, kp, C.a) << " b=" << get_val(cc, kp, C.b) 
              << " decoded=" << get_val(cc, kp, C.a)/get_val(cc, kp, C.b) - PSI << "\n\n";
    
    // ratio_add
    PE sum_ab = ratio_add(cc, A, B);
    double sum_a = get_val(cc, kp, sum_ab.a);
    double sum_b = get_val(cc, kp, sum_ab.b);
    double sum_ratio = sum_a / sum_b;
    double sum_decoded = sum_ratio - PSI;
    std::cout << "  After ratio_add(A,B):\n";
    std::cout << "    a=" << sum_a << " b=" << sum_b << " ratio=" << sum_ratio << " decoded=" << sum_decoded << "\n";
    std::cout << "    Expected decoded: 0.8\n\n";
    
    // ratio_mult
    PE result = ratio_mult(cc, sum_ab, C);
    double res_a = get_val(cc, kp, result.a);
    double res_b = get_val(cc, kp, result.b);
    double res_ratio = res_a / res_b;
    double res_decoded = res_ratio - PSI;
    std::cout << "  After ratio_mult(sum,C):\n";
    std::cout << "    a=" << res_a << " b=" << res_b << " ratio=" << res_ratio << " raw=" << res_decoded << "\n\n";
    
    // Correction
    double corrected = correct_add_then_mul(res_decoded, 0.8, 0.2, 1);
    std::cout << "  After correction:\n";
    std::cout << "    corrected=" << corrected << " (expected 0.16)\n\n";
    
    // === STEP 3: Re-encode and check ===
    std::cout << "  === STEP 3: Re-encode corrected value ===\n";
    PE re_encoded = make_state(cc, kp, corrected);
    double re_a = get_val(cc, kp, re_encoded.a);
    double re_b = get_val(cc, kp, re_encoded.b);
    double re_decoded = re_a / re_b - PSI;
    std::cout << "  make_state(" << corrected << "):\n";
    std::cout << "    a=" << re_a << " b=" << re_b << " decoded=" << re_decoded << "\n";
    std::cout << "    (decoded should = " << corrected << ")\n\n";
    
    // === STEP 4: Normalize (mulY steps) ===
    std::cout << "  === STEP 4: Fibonacci Normalization ===\n";
    const int STEPS = 8;
    PE norm_state = re_encoded;
    for (int i = 0; i < STEPS; i++) {
        norm_state = mulY(cc, norm_state);
    }
    double norm_a = get_val(cc, kp, norm_state.a);
    double norm_b = get_val(cc, kp, norm_state.b);
    double norm_ratio = norm_a / norm_b;
    double norm_decoded = norm_ratio - PSI;
    double recovered = norm_b / fib(STEPS + 2);
    
    std::cout << "  After " << STEPS << " mulY steps:\n";
    std::cout << "    a=" << norm_a << " (expected F(" << STEPS+1 << ")=" << fib(STEPS+1) << ")\n";
    std::cout << "    b=" << norm_b << " (expected F(" << STEPS+2 << ")=" << fib(STEPS+2) << ")\n";
    std::cout << "    ratio=" << norm_ratio << " (expected ψ≈0.618)\n";
    std::cout << "    decoded=" << norm_decoded << "\n";
    std::cout << "    recovered=b/F(" << STEPS+2 << ")=" << recovered << "\n";
    std::cout << "    (recovered should = " << corrected << ")\n\n";
    
    return 0;
}
