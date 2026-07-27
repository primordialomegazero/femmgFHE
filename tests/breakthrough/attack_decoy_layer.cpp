// ATTACK 5: Decoy Layer Exploitation
// Target: Reality 1's φ-Chaos + φ-Harmony + φ-Manipulation
// Strategy: Find the underlying φ-pattern in the decoy
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

const double PHI = 1.618033988749895;
const double PSI = 0.6180339887498949;

// ═══════════════════════════════════════════
// REALITY 1: THE DECOY (Exact copy from Dan)
// ═══════════════════════════════════════════
void R1_chaos(CryptoContext<DCRTPoly>& cc, PE& state, int cycles) {
    for (int i = 0; i < cycles; i++) {
        if (i % 3 == 0) state = F_mulY(cc, state);
        else if (i % 3 == 1) state = F_mulY_inv(cc, state);
        else state = F_swap(state);
    }
}

void R1_harmony(CryptoContext<DCRTPoly>& cc, PE& signal, PE& noise, double& balance) {
    int action = rand()%4;
    switch(action) {
        case 0: signal = F_mulY(cc, signal); noise = F_mulY(cc, noise); balance *= PHI; break;
        case 1: signal = F_mulY_inv(cc, signal); noise = F_mulY_inv(cc, noise); balance *= PSI; break;
        case 2: signal = F_mulY(cc, signal); noise = F_mulY_inv(cc, noise); balance *= -1.0; break;
        case 3: signal = F_mulY_inv(cc, signal); noise = F_mulY(cc, noise); balance *= -1.0; break;
    }
}

void R1_manipulation(CryptoContext<DCRTPoly>& cc, PE& state, int& decoy_pattern) {
    double pattern_seed = PHI;
    for (int i = 0; i < decoy_pattern; i++) pattern_seed = pattern_seed * PHI - PSI;
    int pattern_ops = (int)(std::abs(pattern_seed) * 10) % 5 + 2;
    for (int i = 0; i < pattern_ops; i++) {
        if (pattern_seed > PHI) { state = F_mulY(cc, state); pattern_seed *= PSI; }
        else { state = F_mulY_inv(cc, state); pattern_seed *= PHI; }
    }
    decoy_pattern = (decoy_pattern + 1) % 7;
}

struct Reality1 { PE signal; PE noise; double balance; int decoy_state; };

Reality1 R1_process(CryptoContext<DCRTPoly>& cc, const PE& input) {
    PE signal = input, noise = input;
    double balance = 1.0;
    int decoy = rand()%7;
    R1_chaos(cc, signal, 3 + rand()%3);
    R1_chaos(cc, noise, 2 + rand()%2);
    int harmonic_cycles = 3 + rand()%4;
    for (int i = 0; i < harmonic_cycles; i++) R1_harmony(cc, signal, noise, balance);
    R1_manipulation(cc, signal, decoy);
    R1_manipulation(cc, noise, decoy);
    return {signal, noise, balance, decoy};
}

// ═══════════════════════════════════════════
// ATTACK: Exploit φ-patterns in Reality 1
// ═══════════════════════════════════════════
int main() {
    srand(time(0));
    
    std::cout << "\n";
    std::cout << "  ╔══════════════════════════════════════════════════════╗\n";
    std::cout << "  ║  ATTACK 5: Decoy Layer Exploitation                 ║\n";
    std::cout << "  ║  Target: Reality 1 — φ-Chaos + φ-Harmony            ║\n";
    std::cout << "  ╚══════════════════════════════════════════════════════╝\n\n";

    CCParams<CryptoContextCKKSRNS> p;
    p.SetMultiplicativeDepth(20); p.SetScalingModSize(30); p.SetBatchSize(256);
    p.SetRingDim(2048); p.SetSecretKeyDist(UNIFORM_TERNARY); p.SetSecurityLevel(HEStd_NotSet);
    auto cc = GenCryptoContext(p);
    cc->Enable(PKE); cc->Enable(KEYSWITCH); cc->Enable(LEVELEDSHE);
    auto kp = cc->KeyGen(); cc->EvalMultKeyGen(kp.secretKey);

    const int SAMPLES = 500;
    
    // === PHASE 1: MAP THE DECOY PATTERNS ===
    std::vector<double> balances_0, balances_1;
    std::vector<double> signal_ratios_0, signal_ratios_1;
    std::vector<double> noise_ratios_0, noise_ratios_1;
    std::vector<double> balance_products;
    
    for (int t = 0; t < SAMPLES; t++) {
        int secret = rand() % 2;
        PE ct = F_enc(cc, kp, secret);
        Reality1 r1 = R1_process(cc, ct);
        
        double sr = F_ratio(cc, kp, r1.signal);
        double nr = F_ratio(cc, kp, r1.noise);
        
        if (secret == 0) {
            balances_0.push_back(r1.balance);
            signal_ratios_0.push_back(sr);
            noise_ratios_0.push_back(nr);
        } else {
            balances_1.push_back(r1.balance);
            signal_ratios_1.push_back(sr);
            noise_ratios_1.push_back(nr);
        }
        balance_products.push_back(sr * nr);  // φ·ψ = -1 detection
    }
    
    // === PHASE 2: FIND EXPLOITABLE φ-PATTERNS ===
    auto stats = [](std::vector<double>& v, const char* label) {
        double sum = 0;
        int positive = 0, negative = 0;
        for (double x : v) { 
            sum += x; 
            if (x > 0) positive++; else negative++;
        }
        double mean = sum / v.size();
        std::cout << "  " << label << ": mean=" << std::fixed << std::setprecision(4) << mean
                  << " pos=" << positive << "(" << (100.0*positive/v.size()) << "%)"
                  << " neg=" << negative << "(" << (100.0*negative/v.size()) << "%)\n";
        return mean;
    };
    
    std::cout << "  ┌──────────────────────────────────────────────────────┐\n";
    std::cout << "  │ DECOY PATTERN ANALYSIS (" << SAMPLES << " samples)                  │\n";
    std::cout << "  ├──────────────────────────────────────────────────────┤\n";
    stats(balances_0, "Balance (bit=0)");
    stats(balances_1, "Balance (bit=1)");
    
    // KEY INSIGHT: φ·ψ = -1 means balance sign flips!
    // Can we detect sign flip patterns?
    double avg_product_0 = 0, avg_product_1 = 0;
    int cross_detected_0 = 0, cross_detected_1 = 0;
    for (int i = 0; i < SAMPLES/2; i++) {
        avg_product_0 += balance_products[i];
        if (std::abs(balance_products[i] + 1.0) < 0.5) cross_detected_0++;  // Near -1
    }
    for (int i = SAMPLES/2; i < SAMPLES; i++) {
        avg_product_1 += balance_products[i];
        if (std::abs(balance_products[i] + 1.0) < 0.5) cross_detected_1++;
    }
    avg_product_0 /= (SAMPLES/2);
    avg_product_1 /= (SAMPLES/2);
    
    std::cout << "  ├──────────────────────────────────────────────────────┤\n";
    std::cout << "  │ φ·ψ Cross Detection (near -1):                       │\n";
    std::cout << "  │ Bit=0: " << cross_detected_0 << "/" << (SAMPLES/2) << " (" << (100.0*cross_detected_0/(SAMPLES/2)) << "%)                            │\n";
    std::cout << "  │ Bit=1: " << cross_detected_1 << "/" << (SAMPLES/2) << " (" << (100.0*cross_detected_1/(SAMPLES/2)) << "%)                            │\n";
    
    // === PHASE 3: EXPLOIT φ·ψ = -1 CANCELLATION ===
    // If balance is negative (φ·ψ cross happened), the attacker can flip prediction
    int correct = 0;
    for (int t = 0; t < SAMPLES; t++) {
        int secret = rand() % 2;
        PE ct = F_enc(cc, kp, secret);
        Reality1 r1 = R1_process(cc, ct);
        
        double sr = F_ratio(cc, kp, r1.signal);
        double nr = F_ratio(cc, kp, r1.noise);
        
        // ATTACK STRATEGY:
        // If signal > noise and balance positive → predict 1
        // If signal > noise and balance negative → predict 0 (φ·ψ flipped!)
        int prediction;
        if (r1.balance > 0) {
            prediction = (sr > nr) ? 1 : 0;
        } else {
            prediction = (sr > nr) ? 0 : 1;  // Flipped due to φ·ψ!
        }
        
        if (prediction == secret) correct++;
    }
    
    double accuracy = 100.0 * correct / SAMPLES;
    
    std::cout << "  ├──────────────────────────────────────────────────────┤\n";
    std::cout << "  │ EXPLOIT ATTACK RESULTS                               │\n";
    std::cout << "  │ Correct: " << correct << "/" << SAMPLES << " (" << std::fixed << std::setprecision(1) << accuracy << "%)                          │\n";
    
    if (accuracy > 55.0)
        std::cout << "  │ 🔴 DECOY EXPLOITED! φ·ψ flip detected!               │\n";
    else if (accuracy > 52.0)
        std::cout << "  │ ⚠ WEAK SIGNAL — Slight exploitation possible         │\n";
    else if (accuracy >= 48.0)
        std::cout << "  │ ✓ DECOY RESISTANT — Near random                      │\n";
    else
        std::cout << "  │ ❌❌❌ ANTI-CORRELATED — Decoy is MISLEADING!        │\n";
    
    std::cout << "  └──────────────────────────────────────────────────────┘\n\n";
    
    return 0;
}
