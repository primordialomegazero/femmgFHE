// Test FHE_OFFSET = |psi|(|psi| + vC) for arbitrary vC values
#include <iostream>
#include <iomanip>
#include <cmath>
#include <vector>
#include "openfhe.h"
using namespace lbcrypto;

struct PE { Ciphertext<DCRTPoly> a; Ciphertext<DCRTPoly> b; };

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

int main() {
    const double PSI = 0.6180339887498949;
    
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
    
    std::cout << "\n  ╔══════════════════════════════════════════════════════════╗\n";
    std::cout <<   "  ║   FHE_OFFSET Generalization Test                         ║\n";
    std::cout <<   "  ║   Verify: delta(vC) = |psi|(|psi| + vC)                  ║\n";
    std::cout <<   "  ╚══════════════════════════════════════════════════════════╝\n\n";
    
    // Test values
    std::vector<double> vC_values = {0.1, 0.2, 0.3, 0.5, 1.0, 2.0};
    std::vector<int> N_values = {1, 2, 3, 4};
    
    // A is fixed at 0.5
    PE A = make_state(cc, kp, 0.5);
    
    std::cout << "  vC     N   Raw FHE    Predicted delta   Actual delta    Match?\n";
    std::cout << "  -----------------------------------------------------------------\n";
    
    int matches = 0, total = 0;
    
    for (double vC : vC_values) {
        PE C_state = make_state(cc, kp, vC);
        double predicted_delta = PSI * (PSI + vC);
        
        for (int N : N_values) {
            total++;
            
            // Build sum of N copies of A
            PE sum_state = A;
            for (int i = 0; i < N; i++) {
                sum_state = ratio_add(cc, sum_state, A);
            }
            
            // Multiply by C
            PE result = ratio_mult(cc, sum_state, C_state);
            double raw_fhe = decode_value(get_ratio(cc, kp, result));
            
            // Ideal raw (without CKKS offset)
            double sum_v = 0.5 * (N + 1);
            double raw_ideal = (sum_v + N * PSI) * (vC + PSI) - PSI;
            double raw_ideal_decoded = raw_ideal - PSI; // Wait, decode is ratio - psi
            
            // Actual delta
            double actual_delta = raw_fhe - (sum_v * vC + sum_v * PSI + N * PSI * vC + N * PSI * PSI - PSI);
            // This is approximate — let's just compute the difference
            
            // Simpler: raw_fhe - raw_ideal (in ratio space)
            double ratio_fhe = get_ratio(cc, kp, result);
            double ratio_ideal = (sum_v + N * PSI) * (vC + PSI);
            double actual_delta_ratio = ratio_fhe - ratio_ideal;
            
            bool match = std::abs(actual_delta_ratio - predicted_delta) < 0.01;
            if (match) matches++;
            
            std::cout << "  " << std::fixed << std::setprecision(1) << vC
                 << "    " << N
                 << "   " << std::setprecision(6) << ratio_fhe
                 << "   " << std::setprecision(6) << predicted_delta
                 << "      " << std::setprecision(6) << actual_delta_ratio
                 << "      " << (match ? "✅" : "❌") << "\n";
        }
        std::cout << "\n";
    }
    
    std::cout << "  Result: " << matches << "/" << total << " match predicted delta\n";
    std::cout << "  Formula: delta(vC) = |psi|(|psi| + vC) = " << std::fixed << std::setprecision(10) << PSI << " * (" << PSI << " + vC)\n\n";
    
    return 0;
}
