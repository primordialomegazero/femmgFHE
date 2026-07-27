// ATTACK 7: WHITE-BOX — Full Knowledge Exploitation
// We know: φ·ψ = -1, harmonic cycles, balance evolution
// Strategy: Track the φ-state across multiple obfuscations
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

// Exact copy of Dan's Reality 1
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

int main() {
    srand(time(0));
    
    std::cout << "\n";
    std::cout << "  ╔══════════════════════════════════════════════════════╗\n";
    std::cout << "  ║  ATTACK 7: WHITE-BOX — Full Source Knowledge        ║\n";
    std::cout << "  ║  We know: φ·ψ=-1, balance evolution, chaos cycles    ║\n";
    std::cout << "  ╚══════════════════════════════════════════════════════╝\n\n";

    CCParams<CryptoContextCKKSRNS> p;
    p.SetMultiplicativeDepth(20); p.SetScalingModSize(30); p.SetBatchSize(256);
    p.SetRingDim(2048); p.SetSecretKeyDist(UNIFORM_TERNARY); p.SetSecurityLevel(HEStd_NotSet);
    auto cc = GenCryptoContext(p);
    cc->Enable(PKE); cc->Enable(KEYSWITCH); cc->Enable(LEVELEDSHE);
    auto kp = cc->KeyGen(); cc->EvalMultKeyGen(kp.secretKey);

    // === WHITE-BOX STRATEGY ===
    // We know: balance follows φ^n or ψ^n or -1 cycles
    // Key insight: Track balance EVOLUTION, not just final value!
    
    const int SAMPLES = 300;
    
    // Phase 1: Map the balance evolution curve
    std::vector<double> bal_evolution_0, bal_evolution_1;
    
    for (int t = 0; t < SAMPLES; t++) {
        int secret = rand() % 2;
        PE ct = F_enc(cc, kp, secret);
        Reality1 r1 = R1_process(cc, ct);
        
        // Track balance as product of φ/ψ operations
        double bal = r1.balance;
        double abs_bal = std::abs(bal);
        
        // WHITE-BOX INSIGHT:
        // balance = ±φ^n × ψ^m
        // If we can estimate n and m, we can invert!
        
        // Count how many φ vs ψ operations
        // φ-multiply: balance ×= PHI (expanding)
        // ψ-multiply: balance ×= PSI (contracting)
        // φ·ψ cross: balance ×= -1 (sign flip)
        
        if (secret == 0) bal_evolution_0.push_back(bal);
        else bal_evolution_1.push_back(bal);
    }
    
    // Phase 2: Find the φ/ψ fingerprint
    auto analyze = [](std::vector<double>& v, const char* label) {
        double sum = 0, abs_sum = 0;
        int pos = 0, neg = 0, near_phi = 0, near_psi = 0, near_one = 0;
        
        for (double x : v) {
            sum += x;
            abs_sum += std::abs(x);
            if (x > 0) pos++; else neg++;
            
            // Check if balance is near φ^n
            double log_abs = std::log(std::abs(x) + 1e-10);
            double log_phi = std::log(PHI);
            double n_approx = log_abs / log_phi;
            double frac = n_approx - std::floor(n_approx);
            
            if (frac < 0.1 || frac > 0.9) near_phi++;  // Near integer power of φ
            if (std::abs(x - PSI) < 0.1 || std::abs(x + PSI) < 0.1) near_psi++;
            if (std::abs(std::abs(x) - 1.0) < 0.1) near_one++;
        }
        
        double mean = sum / v.size();
        double mean_abs = abs_sum / v.size();
        
        std::cout << "  " << label << ": mean=" << std::fixed << std::setprecision(4) << mean
                  << " |mean|=" << mean_abs
                  << " pos=" << pos << "(" << (100.0*pos/v.size()) << "%)"
                  << " near_φ=" << near_phi << " near_ψ=" << near_psi << " near_±1=" << near_one << "\n";
        
        return mean;
    };
    
    std::cout << "  ┌──────────────────────────────────────────────────────┐\n";
    std::cout << "  │ φ/ψ FINGERPRINT ANALYSIS                              │\n";
    std::cout << "  ├──────────────────────────────────────────────────────┤\n";
    analyze(bal_evolution_0, "Balance (bit=0)");
    analyze(bal_evolution_1, "Balance (bit=1)");
    
    // Phase 3: EXPLOIT φ·ψ = -1 CYCLE
    // If we see balance = -1, it means last op was φ·ψ cross!
    // φ·ψ cross happens in harmony cases 2 and 3 (50% of harmony ops!)
    // After φ·ψ cross, balance flips sign!
    
    int correct_flip = 0;
    for (int t = 0; t < SAMPLES; t++) {
        int secret = rand() % 2;
        PE ct = F_enc(cc, kp, secret);
        Reality1 r1 = R1_process(cc, ct);
        
        double sr = F_ratio(cc, kp, r1.signal);
        double nr = F_ratio(cc, kp, r1.noise);
        
        // WHITE-BOX PREDICTION:
        // If balance < 0 → φ·ψ cross happened → flip prediction
        // If balance > 0 and near φ → expand happened → predict 1
        // If balance > 0 and near ψ → contract happened → predict 0
        
        int prediction;
        if (r1.balance < 0) {
            // Cross happened: everything is inverted
            prediction = (sr < nr) ? 1 : 0;  // Flipped!
        } else if (std::abs(r1.balance - PHI) < 1.0) {
            prediction = 1;  // φ-expand
        } else if (std::abs(r1.balance - PSI) < 1.0) {
            prediction = 0;  // ψ-contract
        } else {
            prediction = (sr > nr) ? 1 : 0;
        }
        
        if (prediction == secret) correct_flip++;
    }
    
    double acc_flip = 100.0 * correct_flip / SAMPLES;
    
    std::cout << "  ├──────────────────────────────────────────────────────┤\n";
    std::cout << "  │ WHITE-BOX ATTACK RESULTS                              │\n";
    std::cout << "  │ φ·ψ Flip Strategy: " << std::fixed << std::setprecision(1) << acc_flip << "% accuracy                     │\n";
    
    if (acc_flip > 55.0)
        std::cout << "  │ 🔴 REALITY 1 BROKEN! φ·ψ cycle exploited!            │\n";
    else if (acc_flip > 52.0)
        std::cout << "  │ ⚠ PARTIAL BREAK — Weak signal detected               │\n";
    else if (acc_flip >= 48.0)
        std::cout << "  │ ✓ RESISTANT — White-box knowledge not enough         │\n";
    else
        std::cout << "  │ ❌❌❌ ANTI-CORRELATED — Even white-box fails!       │\n";
    
    std::cout << "  └──────────────────────────────────────────────────────┘\n\n";
    
    return 0;
}
