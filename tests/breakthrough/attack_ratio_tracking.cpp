// ATTACK 1: Ratio Distribution Analysis
// Can we predict the bit from ratio patterns?
#include <iostream>
#include <iomanip>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <vector>
#include <algorithm>
#include "openfhe.h"
using namespace lbcrypto;

struct PE { Ciphertext<DCRTPoly> a; Ciphertext<DCRTPoly> b; };

PE F_mulY(CryptoContext<DCRTPoly>& cc, const PE& x) {
    return {x.b, cc->EvalAdd(x.a, x.b)};
}
PE F_mulY_inv(CryptoContext<DCRTPoly>& cc, const PE& x) {
    return {cc->EvalSub(x.b, x.a), x.a};
}
double F_val(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, const Ciphertext<DCRTPoly>& c) {
    Plaintext pt; cc->Decrypt(kp.secretKey, c, &pt);
    return pt->GetCKKSPackedValue()[0].real();
}
double F_ratio(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, const PE& s) {
    double a = F_val(cc, kp, s.a), b = F_val(cc, kp, s.b);
    return (std::abs(b) > 1e-10) ? a / b : a;
}
PE F_enc(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, int b) {
    double v = b ? 1.0 : 0.0;
    return {cc->Encrypt(kp.publicKey, cc->MakeCKKSPackedPlaintext(std::vector<double>{v})),
            cc->Encrypt(kp.publicKey, cc->MakeCKKSPackedPlaintext(std::vector<double>{1.0}))};
}
PE F_swap(PE x) { auto t=x.a; x.a=x.b; x.b=t; return x; }

// Same Dual Fractal as Dan
struct FractalNode { PE expand; PE contract; int depth; };

void F_dual_rotate(CryptoContext<DCRTPoly>& cc, PE& E, PE& C) {
    int action = rand()%6;
    switch(action) {
        case 0: E = F_mulY(cc, E);     C = F_mulY(cc, C);     break;
        case 1: E = F_mulY_inv(cc, E); C = F_mulY_inv(cc, C); break;
        case 2: E = F_mulY(cc, E);     C = F_mulY_inv(cc, C); break;
        case 3: E = F_mulY_inv(cc, E); C = F_mulY(cc, C);     break;
        case 4: E = F_swap(E);         break;
        case 5: C = F_swap(C);         break;
    }
}

FractalNode F_recursive_dual(CryptoContext<DCRTPoly>& cc, const PE& input, int depth) {
    if (depth <= 0) return {input, input, 0};
    PE E = input, C = input;
    int rotations = 3 + rand()%4;
    for (int i = 0; i < rotations; i++) F_dual_rotate(cc, E, C);
    FractalNode subE = F_recursive_dual(cc, E, depth - 1);
    FractalNode subC = F_recursive_dual(cc, C, depth - 1);
    return {subE.expand, subC.contract, depth};
}

int main() {
    srand(time(0));
    
    std::cout << "\n";
    std::cout << "  ╔══════════════════════════════════════════════════════╗\n";
    std::cout << "  ║  ATTACK 1: Ratio Distribution Analysis              ║\n";
    std::cout << "  ║  Can we predict bit from ratio statistics?          ║\n";
    std::cout << "  ╚══════════════════════════════════════════════════════╝\n\n";

    CCParams<CryptoContextCKKSRNS> p;
    p.SetMultiplicativeDepth(20); p.SetScalingModSize(30); p.SetBatchSize(256);
    p.SetRingDim(2048); p.SetSecretKeyDist(UNIFORM_TERNARY); p.SetSecurityLevel(HEStd_NotSet);
    auto cc = GenCryptoContext(p);
    cc->Enable(PKE); cc->Enable(KEYSWITCH); cc->Enable(LEVELEDSHE);
    auto kp = cc->KeyGen(); cc->EvalMultKeyGen(kp.secretKey);

    const int SAMPLES = 200;
    const int DEPTH = 2;
    
    std::vector<double> ratios_0, ratios_1;
    std::vector<double> tensions_0, tensions_1;
    
    for (int t = 0; t < SAMPLES; t++) {
        int secret = rand() % 2;
        PE ct = F_enc(cc, kp, secret);
        FractalNode fractal = F_recursive_dual(cc, ct, DEPTH);
        
        double re = F_ratio(cc, kp, fractal.expand);
        double rc = F_ratio(cc, kp, fractal.contract);
        double tension = re - rc;
        double avg_ratio = (re + rc) / 2.0;
        
        if (secret == 0) {
            ratios_0.push_back(avg_ratio);
            tensions_0.push_back(tension);
        } else {
            ratios_1.push_back(avg_ratio);
            tensions_1.push_back(tension);
        }
    }
    
    // Statistical analysis
    auto stats = [](std::vector<double>& v, const char* label) {
        double sum = 0, min_v = 999, max_v = -999;
        for (double x : v) { sum += x; min_v = std::min(min_v, x); max_v = std::max(max_v, x); }
        double mean = sum / v.size();
        double var = 0;
        for (double x : v) var += (x - mean) * (x - mean);
        var /= v.size();
        std::cout << "  " << label << ": mean=" << std::fixed << std::setprecision(4) << mean
                  << " std=" << std::setprecision(4) << std::sqrt(var)
                  << " range=[" << min_v << "," << max_v << "]\n";
        return mean;
    };
    
    std::cout << "  ┌──────────────────────────────────────────────────────┐\n";
    std::cout << "  │ RATIO STATISTICS (Depth " << DEPTH << ", " << SAMPLES << " samples)            │\n";
    std::cout << "  ├──────────────────────────────────────────────────────┤\n";
    double mean0 = stats(ratios_0, "Bit=0 ratios ");
    double mean1 = stats(ratios_1, "Bit=1 ratios ");
    std::cout << "  │                                                      │\n";
    stats(tensions_0, "Bit=0 tensions");
    stats(tensions_1, "Bit=1 tensions");
    
    double separation = std::abs(mean0 - mean1);
    std::cout << "  ├──────────────────────────────────────────────────────┤\n";
    std::cout << "  │ Ratio separation: " << std::fixed << std::setprecision(6) << separation << "                              │\n";
    
    // Try to classify
    double threshold = (mean0 + mean1) / 2.0;
    int correct = 0;
    for (double r : ratios_0) if (r < threshold) correct++;
    for (double r : ratios_1) if (r > threshold) correct++;
    double accuracy = 100.0 * correct / (2 * SAMPLES);
    
    std::cout << "  │ Attack accuracy:  " << std::fixed << std::setprecision(1) << accuracy << "%                              │\n";
    
    if (accuracy < 55.0)
        std::cout << "  │ ✓ ATTACK FAILED — Ratio alone cannot predict bit     │\n";
    else if (accuracy < 70.0)
        std::cout << "  │ ⚠ WEAK LEAK — Slight correlation detected            │\n";
    else
        std::cout << "  │ ❌ VULNERABLE — Ratio reveals bit!                   │\n";
    
    std::cout << "  └──────────────────────────────────────────────────────┘\n\n";
    
    return 0;
}
