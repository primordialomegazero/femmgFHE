// ATTACK 2: Tension Direction Analysis
// Does tension sign correlate with original bit?
#include <iostream>
#include <iomanip>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <vector>
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
    std::cout << "  ║  ATTACK 2: Tension Direction Bias                   ║\n";
    std::cout << "  ║  Does tension sign predict the bit?                 ║\n";
    std::cout << "  ╚══════════════════════════════════════════════════════╝\n\n";

    CCParams<CryptoContextCKKSRNS> p;
    p.SetMultiplicativeDepth(20); p.SetScalingModSize(30); p.SetBatchSize(256);
    p.SetRingDim(2048); p.SetSecretKeyDist(UNIFORM_TERNARY); p.SetSecurityLevel(HEStd_NotSet);
    auto cc = GenCryptoContext(p);
    cc->Enable(PKE); cc->Enable(KEYSWITCH); cc->Enable(LEVELEDSHE);
    auto kp = cc->KeyGen(); cc->EvalMultKeyGen(kp.secretKey);

    const int SAMPLES = 500;
    
    // Track: for bit=0, does tension tend positive or negative?
    int pos_tension_0 = 0, neg_tension_0 = 0;
    int pos_tension_1 = 0, neg_tension_1 = 0;
    
    // Track: swap count (does swapping reveal anything?)
    int swap_0 = 0, swap_1 = 0;
    
    // Track: which branch dominates?
    int expand_bigger_0 = 0, contract_bigger_0 = 0;
    int expand_bigger_1 = 0, contract_bigger_1 = 0;
    
    for (int t = 0; t < SAMPLES; t++) {
        int secret = rand() % 2;
        PE ct = F_enc(cc, kp, secret);
        FractalNode fractal = F_recursive_dual(cc, ct, 2);
        
        double re = F_ratio(cc, kp, fractal.expand);
        double rc = F_ratio(cc, kp, fractal.contract);
        double tension = re - rc;
        
        if (secret == 0) {
            if (tension > 0) pos_tension_0++;
            else neg_tension_0++;
            if (re > rc) expand_bigger_0++;
            else contract_bigger_0++;
        } else {
            if (tension > 0) pos_tension_1++;
            else neg_tension_1++;
            if (re > rc) expand_bigger_1++;
            else contract_bigger_1++;
        }
    }
    
    std::cout << "  ┌──────────────────────────────────────────────────────┐\n";
    std::cout << "  │ TENSION DIRECTION (" << SAMPLES << " samples, Depth 2)                  │\n";
    std::cout << "  ├──────────────────────────────────────────────────────┤\n";
    std::cout << "  │ Bit=0: Positive=" << std::setw(4) << pos_tension_0 
              << " (" << std::fixed << std::setprecision(1) << (100.0*pos_tension_0/SAMPLES) << "%)"
              << "  Negative=" << std::setw(4) << neg_tension_0 
              << " (" << (100.0*neg_tension_0/SAMPLES) << "%)    │\n";
    std::cout << "  │ Bit=1: Positive=" << std::setw(4) << pos_tension_1 
              << " (" << (100.0*pos_tension_1/SAMPLES) << "%)"
              << "  Negative=" << std::setw(4) << neg_tension_1 
              << " (" << (100.0*neg_tension_1/SAMPLES) << "%)    │\n";
    std::cout << "  ├──────────────────────────────────────────────────────┤\n";
    
    // Try predicting: if tension>0 → bit=1, else bit=0
    int correct_tension = 0;
    // For bit 0: predict 0 when tension<0
    correct_tension += neg_tension_0;
    // For bit 1: predict 1 when tension>0
    correct_tension += pos_tension_1;
    double acc_tension = 100.0 * correct_tension / (2 * SAMPLES);
    
    std::cout << "  │ Tension-based attack: " << std::fixed << std::setprecision(1) << acc_tension << "% accuracy                 │\n";
    
    // Try predicting: if expand>contract → bit=1, else bit=0
    int correct_expand = expand_bigger_1 + contract_bigger_0;
    double acc_expand = 100.0 * correct_expand / (2 * SAMPLES);
    std::cout << "  │ Expand-dominance attack: " << std::fixed << std::setprecision(1) << acc_expand << "% accuracy            │\n";
    
    std::cout << "  ├──────────────────────────────────────────────────────┤\n";
    if (acc_tension < 55.0 && acc_expand < 55.0)
        std::cout << "  │ ✓ ATTACK FAILED — No directional bias               │\n";
    else if (acc_tension < 70.0)
        std::cout << "  │ ⚠ WEAK BIAS — Slight directional correlation        │\n";
    else
        std::cout << "  │ ❌ VULNERABLE — Direction predicts bit!             │\n";
    std::cout << "  └──────────────────────────────────────────────────────┘\n\n";
    
    return 0;
}
