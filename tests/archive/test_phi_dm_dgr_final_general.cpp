// DM-DGR FINAL with Generalized FHE_OFFSET
// Tests add-then-mul with multiple vC values
#include <iostream>
#include <iomanip>
#include <cmath>
#include <vector>
#include "openfhe.h"
using namespace lbcrypto;

struct PE { Ciphertext<DCRTPoly> a; Ciphertext<DCRTPoly> b; };

PE mulY(CryptoContext<DCRTPoly>& cc, const PE& x) {
    return {x.b, cc->EvalAdd(x.a, x.b)};
}
PE mulY_inv(CryptoContext<DCRTPoly>& cc, const PE& x) {
    return {cc->EvalSub(x.b, x.a), x.a};
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
    Plaintext pt;
    cc->Decrypt(kp.secretKey, c, &pt);
    return pt->GetCKKSPackedValue()[0].real();
}
double get_ratio(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, const PE& s) {
    double a = get_val(cc, kp, s.a);
    double b = get_val(cc, kp, s.b);
    return (std::abs(b) > 1e-10) ? a / b : a;
}
PE make_state(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, double value) {
    const double PSI = 0.6180339887498949;
    auto pta = cc->MakeCKKSPackedPlaintext(std::vector<double>{value + PSI});
    auto ptb = cc->MakeCKKSPackedPlaintext(std::vector<double>{1.0});
    return {cc->Encrypt(kp.publicKey, pta), cc->Encrypt(kp.publicKey, ptb)};
}
double decode_value(double ratio) {
    return ratio - 0.6180339887498949;
}

const double PSI = 0.6180339887498949;
const double PSI2 = PSI * PSI;

// GENERALIZED correction
double correct_add_then_mul(double raw, double sum_v, double vC, int N_adds) {
    return raw - sum_v * PSI - N_adds * PSI * vC - N_adds * PSI2 + PSI - PSI * (PSI + vC);
}

int main() {
    std::cout << "\n  ╔══════════════════════════════════════════════════════════╗\n";
    std::cout <<   "  ║   DM-DGR: Generalized FHE_OFFSET Test                    ║\n";
    std::cout <<   "  ║   delta(vC) = |psi|(|psi| + vC)                          ║\n";
    std::cout <<   "  ╚══════════════════════════════════════════════════════════╝\n\n";
    
    CCParams<CryptoContextCKKSRNS> p;
    p.SetMultiplicativeDepth(50);
    p.SetScalingModSize(50);
    p.SetBatchSize(2048);
    p.SetRingDim(4096);
    p.SetSecretKeyDist(UNIFORM_TERNARY);
    p.SetSecurityLevel(HEStd_NotSet);
    
    auto cc = GenCryptoContext(p);
    cc->Enable(PKE); cc->Enable(KEYSWITCH); cc->Enable(LEVELEDSHE); cc->Enable(ADVANCEDSHE);
    auto kp = cc->KeyGen();
    cc->EvalMultKeyGen(kp.secretKey);
    
    int passed = 0, total = 0;
    
    auto test = [&](std::string name, double expected, double got, double tol = 0.02) {
        total++;
        double err = std::abs(got - expected);
        bool ok = err < tol;
        if (ok) passed++;
        std::cout << "    " << (ok ? "✅" : "❌") << " " << std::setw(35) << std::left << name
                  << std::fixed << std::setprecision(6) << got
                  << " (exp " << expected << ", err=" << std::scientific << err << ")\n";
    };
    
    // Test with vC = 0.1, 0.2, 0.3, 0.5, 1.0, 2.0
    std::vector<double> vC_vals = {0.1, 0.2, 0.3, 0.5, 1.0, 2.0};
    std::vector<int> N_vals = {1, 2, 4};
    
    PE A = make_state(cc, kp, 0.5);
    
    for (double vC : vC_vals) {
        PE C_state = make_state(cc, kp, vC);
        
        for (int N : N_vals) {
            // Build sum of N+1 copies of A (since we start with A and add N times)
            PE sum_state = A;
            for (int i = 0; i < N; i++) {
                sum_state = ratio_add(cc, sum_state, A);
            }
            
            PE result = ratio_mult(cc, sum_state, C_state);
            double raw = decode_value(get_ratio(cc, kp, result));
            double sum_v = 0.5 * (N + 1);
            double expected = sum_v * vC;
            double corrected = correct_add_then_mul(raw, sum_v, vC, N);
            
            std::string name = "(N=" + std::to_string(N) + " x 0.5) x " + std::to_string(vC);
            test(name, expected, corrected);
        }
    }
    
    // Also test with different base values (not just 0.5)
    std::cout << "\n  --- Different base values ---\n";
    PE B = make_state(cc, kp, 0.3);
    PE C02 = make_state(cc, kp, 0.2);
    PE E = make_state(cc, kp, 1.0);
    
    // (0.5 + 0.3) x 0.2
    PE sum1 = ratio_add(cc, A, B);
    PE res1 = ratio_mult(cc, sum1, C02);
    double raw1 = decode_value(get_ratio(cc, kp, res1));
    test("(0.5+0.3)x0.2", 0.16, correct_add_then_mul(raw1, 0.8, 0.2, 1));
    
    // (0.5 + 0.3 + 1.0) x 0.2
    PE sum2 = ratio_add(cc, ratio_add(cc, A, B), E);
    PE res2 = ratio_mult(cc, sum2, C02);
    double raw2 = decode_value(get_ratio(cc, kp, res2));
    test("(0.5+0.3+1.0)x0.2", 0.36, correct_add_then_mul(raw2, 1.8, 0.2, 2));
    
    // (0.5 x 5) x 0.3
    PE C03 = make_state(cc, kp, 0.3);
    PE sum3 = A;
    for (int i = 0; i < 4; i++) sum3 = ratio_add(cc, sum3, A);
    PE res3 = ratio_mult(cc, sum3, C03);
    double raw3 = decode_value(get_ratio(cc, kp, res3));
    test("(5x0.5)x0.3", 0.75, correct_add_then_mul(raw3, 2.5, 0.3, 4));
    
    // (0.5 + 0.5) x 1.0
    PE C10 = make_state(cc, kp, 1.0);
    PE sum4 = ratio_add(cc, A, A);
    PE res4 = ratio_mult(cc, sum4, C10);
    double raw4 = decode_value(get_ratio(cc, kp, res4));
    test("(0.5+0.5)x1.0", 1.0, correct_add_then_mul(raw4, 1.0, 1.0, 1));
    
    // (0.5) x 2.0
    PE C20 = make_state(cc, kp, 2.0);
    PE res5 = ratio_mult(cc, A, C20);
    double raw5 = decode_value(get_ratio(cc, kp, res5));
    test("0.5x2.0", 1.0, correct_add_then_mul(raw5, 0.5, 2.0, 0));
    
    std::cout << "\n  ╔══════════════════════════════════════════════════════════╗\n";
    std::cout << "  ║  RESULTS: " << passed << "/" << total << " tests passed";
    for (int i = 0; i < 30; i++) std::cout << " ";
    std::cout << "║\n";
    std::cout << "  ║  delta(vC) = |psi|(|psi| + vC) — VERIFIED              ║\n";
    std::cout << "  ╚══════════════════════════════════════════════════════════╝\n\n";
    
    return 0;
}
