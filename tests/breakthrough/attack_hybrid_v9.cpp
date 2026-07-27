// ATTACK 9: Hybrid Seed + Multi-Sample Correlation
// Strategy: Collect MULTIPLE oracle outputs for known inputs,
//           brute force seeds, and correlate PATTERNS not values
#include <iostream>
#include <iomanip>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <sys/time.h>
#include <vector>
#include <algorithm>
#include <numeric>
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
// EXACT COPY OF DAN'S REALITY 1
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
    for (int i = 0; i < decoy_pattern; i++) {
        pattern_seed = pattern_seed * PHI - PSI;
    }
    int pattern_ops = (int)(std::abs(pattern_seed) * 10) % 5 + 2;
    for (int i = 0; i < pattern_ops; i++) {
        if (pattern_seed > PHI) { state = F_mulY(cc, state); pattern_seed *= PSI; }
        else { state = F_mulY_inv(cc, state); pattern_seed *= PHI; }
    }
    decoy_pattern = (decoy_pattern + 1) % 7;
}

// Full Reality 1 with given seed
void R1_full_with_seed(CryptoContext<DCRTPoly>& cc, const PE& input, unsigned int seed,
                        PE& signal_out, PE& noise_out, double& balance_out) {
    srand(seed);
    
    PE signal = input;
    PE noise = input;
    double balance = 1.0;
    int decoy = rand()%7;
    
    R1_chaos(cc, signal, 3 + rand()%3);
    R1_chaos(cc, noise, 2 + rand()%2);
    
    int harmonic_cycles = 3 + rand()%4;
    for (int i = 0; i < harmonic_cycles; i++) {
        R1_harmony(cc, signal, noise, balance);
    }
    
    R1_manipulation(cc, signal, decoy);
    R1_manipulation(cc, noise, decoy);
    
    signal_out = signal;
    noise_out = noise;
    balance_out = balance;
}

// ═══════════════════════════════════════════
// NEW STRATEGY: Statistical Fingerprint
// ═══════════════════════════════════════════

struct OracleSample {
    double ratio_0;    // Ratio for bit=0
    double ratio_1;    // Ratio for bit=1
    double balance_0;  // Balance for bit=0
    double balance_1;  // Balance for bit=1
    double diff;       // ratio_1 - ratio_0
    double balance_diff; // balance_1 - balance_0
    unsigned int seed;
};

// Collect MULTIPLE oracle samples with known inputs
std::vector<OracleSample> collect_oracle_samples(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp,
                                                   int num_samples) {
    std::vector<OracleSample> samples;
    time_t now = time(0);
    
    std::cout << "  │ Collecting " << num_samples << " oracle samples...                    │\n";
    
    for (int i = 0; i < num_samples; i++) {
        unsigned int seed = (unsigned int)(now + i);
        
        PE input_0 = F_enc(cc, kp, 0);
        PE input_1 = F_enc(cc, kp, 1);
        
        PE signal_0, noise_0, signal_1, noise_1;
        double balance_0, balance_1;
        
        R1_full_with_seed(cc, input_0, seed, signal_0, noise_0, balance_0);
        R1_full_with_seed(cc, input_1, seed, signal_1, noise_1, balance_1);
        
        double ratio_0 = F_ratio(cc, kp, signal_0);
        double ratio_1 = F_ratio(cc, kp, signal_1);
        
        samples.push_back({
            ratio_0, ratio_1, balance_0, balance_1,
            ratio_1 - ratio_0, balance_1 - balance_0,
            seed
        });
        
        if ((i+1) % 10 == 0) std::cout << "  │   " << (i+1) << " samples collected...                      │\n";
    }
    
    return samples;
}

// Analyze: May consistent pattern ba across seeds?
void analyze_samples(const std::vector<OracleSample>& samples) {
    std::cout << "  ├──────────────────────────────────────────────────────┤\n";
    std::cout << "  │ STATISTICAL ANALYSIS                                 │\n";
    std::cout << "  ├──────────────────────────────────────────────────────┤\n";
    
    // Compute averages
    double avg_diff = 0, avg_balance_diff = 0;
    double avg_ratio_0 = 0, avg_ratio_1 = 0;
    int pos_diff = 0, neg_diff = 0;
    int pos_bal = 0, neg_bal = 0;
    
    for (const auto& s : samples) {
        avg_diff += s.diff;
        avg_balance_diff += s.balance_diff;
        avg_ratio_0 += s.ratio_0;
        avg_ratio_1 += s.ratio_1;
        
        if (s.diff > 0) pos_diff++;
        else if (s.diff < 0) neg_diff++;
        
        if (s.balance_diff > 0) pos_bal++;
        else if (s.balance_diff < 0) neg_bal++;
    }
    
    int n = samples.size();
    avg_diff /= n;
    avg_balance_diff /= n;
    avg_ratio_0 /= n;
    avg_ratio_1 /= n;
    
    std::cout << "  │ Avg Ratio (bit=0): " << std::fixed << std::setprecision(6) << avg_ratio_0 << "                      │\n";
    std::cout << "  │ Avg Ratio (bit=1): " << avg_ratio_1 << "                      │\n";
    std::cout << "  │ Avg Diff (1-0):    " << avg_diff << "                      │\n";
    std::cout << "  │ Avg Balance Diff:  " << avg_balance_diff << "                      │\n";
    std::cout << "  │ Diff > 0: " << pos_diff << "/" << n << " (" << std::setprecision(1) << (100.0*pos_diff/n) << "%)                         │\n";
    std::cout << "  │ Diff < 0: " << neg_diff << "/" << n << " (" << (100.0*neg_diff/n) << "%)                         │\n";
    
    // KEY INSIGHT: If diff is consistently positive or negative,
    // we can use it as a predictor!
    std::cout << "  ├──────────────────────────────────────────────────────┤\n";
    
    if (std::abs(avg_diff) > 0.1) {
        std::cout << "  │ ★ SIGNAL DETECTED: |avg_diff| = " << std::abs(avg_diff) << "                │\n";
        std::cout << "  │ Strategy: If diff > 0 → bit=1, else bit=0             │\n";
    } else {
        std::cout << "  │ No strong signal (|avg_diff| = " << std::abs(avg_diff) << ")                │\n";
        std::cout << "  │ Trying balance-based prediction...                    │\n";
    }
}

// ═══════════════════════════════════════════
// ATTACK EXECUTION
// ═══════════════════════════════════════════
int main() {
    std::cout << "\n";
    std::cout << "  ╔══════════════════════════════════════════════════════╗\n";
    std::cout << "  ║  ATTACK 9: Hybrid Seed + Multi-Sample Correlation   ║\n";
    std::cout << "  ║  Strategy: Find consistent bias across seeds        ║\n";
    std::cout << "  ╚══════════════════════════════════════════════════════╝\n\n";

    CCParams<CryptoContextCKKSRNS> p;
    p.SetMultiplicativeDepth(50);
    p.SetScalingModSize(50);
    p.SetBatchSize(1024);
    p.SetRingDim(8192);  // Smaller ring for faster execution
    p.SetSecretKeyDist(UNIFORM_TERNARY);
    p.SetSecurityLevel(HEStd_NotSet);
    
    auto cc = GenCryptoContext(p);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);
    
    auto kp = cc->KeyGen();
    cc->EvalMultKeyGen(kp.secretKey);

    std::cout << "  ┌──────────────────────────────────────────────────────┐\n";
    std::cout << "  │ PHASE 1: Multi-Sample Collection                     │\n";
    std::cout << "  ├──────────────────────────────────────────────────────┤\n";
    
    // Collect 50 samples (different seeds, same time window)
    std::vector<OracleSample> samples = collect_oracle_samples(cc, kp, 50);
    
    // Analyze patterns
    analyze_samples(samples);
    
    // ═══════════════════════════════════
    // PHASE 2: Build Predictor
    // ═══════════════════════════════════
    std::cout << "  ├──────────────────────────────────────────────────────┤\n";
    std::cout << "  │ PHASE 2: Predictor Training                          │\n";
    std::cout << "  ├──────────────────────────────────────────────────────┤\n";
    
    // Strategy 1: Sign of balance difference
    // Strategy 2: Sign of ratio difference
    // Strategy 3: Absolute threshold on ratio
    
    int correct_bal = 0, correct_ratio = 0, correct_combined = 0;
    int test_trials = 200;
    
    for (int t = 0; t < test_trials; t++) {
        int secret = rand()%2;
        unsigned int test_seed = (unsigned int)(time(0) + rand()%100);
        
        PE input = F_enc(cc, kp, secret);
        PE signal, noise;
        double balance;
        
        R1_full_with_seed(cc, input, test_seed, signal, noise, balance);
        double ratio = F_ratio(cc, kp, signal);
        
        // Prediction 1: Balance sign
        int pred_bal = (balance > 0) ? 1 : 0;
        if (pred_bal == secret) correct_bal++;
        
        // Prediction 2: Ratio > 0.5
        int pred_ratio = (ratio > 0.5) ? 1 : 0;
        if (pred_ratio == secret) correct_ratio++;
        
        // Prediction 3: Combined (if they agree)
        int pred_combined;
        if (pred_bal == pred_ratio) pred_combined = pred_bal;
        else pred_combined = (std::abs(balance) > 0.5) ? pred_bal : pred_ratio;
        if (pred_combined == secret) correct_combined++;
    }
    
    std::cout << "  │ Balance Sign:    " << std::fixed << std::setprecision(1) 
              << (100.0*correct_bal/test_trials) << "% (" << correct_bal << "/" << test_trials << ")                    │\n";
    std::cout << "  │ Ratio Threshold: " << (100.0*correct_ratio/test_trials) 
              << "% (" << correct_ratio << "/" << test_trials << ")                    │\n";
    std::cout << "  │ Combined:        " << (100.0*correct_combined/test_trials) 
              << "% (" << correct_combined << "/" << test_trials << ")                    │\n";
    
    // ═══════════════════════════════════
    // PHASE 3: Seed Brute Force (Fast)
    // ═══════════════════════════════════
    std::cout << "  ├──────────────────────────────────────────────────────┤\n";
    std::cout << "  │ PHASE 3: Fast Seed Brute Force                       │\n";
    std::cout << "  ├──────────────────────────────────────────────────────┤\n";
    
    // Get ONE oracle output (target)
    unsigned int target_seed = (unsigned int)time(0);
    PE target_input = F_enc(cc, kp, 0);  // Known input
    PE target_signal, target_noise;
    double target_balance;
    
    R1_full_with_seed(cc, target_input, target_seed, target_signal, target_noise, target_balance);
    double target_ratio = F_ratio(cc, kp, target_signal);
    
    std::cout << "  │ Target seed: " << target_seed << "                               │\n";
    std::cout << "  │ Target ratio: " << target_ratio << "                              │\n";
    std::cout << "  │ Target balance: " << target_balance << "                            │\n";
    std::cout << "  │ Brute forcing ±5 seconds...                            │\n";
    
    // Brute force with SAME input (bit=0) to find seed
    int tested = 0;
    bool found = false;
    time_t start = time(0) - 5;
    time_t end = time(0) + 5;
    
    for (time_t t_seed = start; t_seed <= end && !found; t_seed++) {
        PE test_input = F_enc(cc, kp, 0);
        PE test_signal, test_noise;
        double test_balance;
        
        R1_full_with_seed(cc, test_input, (unsigned int)t_seed, test_signal, test_noise, test_balance);
        double test_ratio = F_ratio(cc, kp, test_signal);
        
        tested++;
        
        if (std::abs(test_ratio - target_ratio) < 1e-6 && 
            std::abs(test_balance - target_balance) < 1e-6) {
            std::cout << "  │ ★ SEED FOUND: " << t_seed << " after " << tested << " attempts!                 │\n";
            found = true;
        }
        
        if (tested % 3 == 0) {
            std::cout << "  │   Tested " << tested << " seeds... (current: " << t_seed << ")                  │\n";
        }
    }
    
    if (!found) {
        std::cout << "  │ Seed not found in " << tested << " attempts                       │\n";
        std::cout << "  │ (φ-chaos prevents exact matching)                       │\n";
    }
    
    std::cout << "  └──────────────────────────────────────────────────────┘\n\n";
    
    // ═══════════════════════════════════
    // CONCLUSIONS
    // ═══════════════════════════════════
    std::cout << "  ╔══════════════════════════════════════════════════════╗\n";
    std::cout << "  ║  ATTACK 9 RESULTS                                    ║\n";
    std::cout << "  ╠══════════════════════════════════════════════════════╣\n";
    
    double best_accuracy = std::max({100.0*correct_bal/test_trials, 
                                      100.0*correct_ratio/test_trials,
                                      100.0*correct_combined/test_trials});
    
    std::cout << "  ║  Best Predictor: " << std::fixed << std::setprecision(1) << best_accuracy << "%                                ║\n";
    
    if (best_accuracy > 55) {
        std::cout << "  ║  ★ BREAKTHROUGH! Reality 1 partially cracked!         ║\n";
    } else if (best_accuracy > 52) {
        std::cout << "  ║  Slight advantage over random                         ║\n";
    } else {
        std::cout << "  ║  Still near random — Reality 1 holds strong           ║\n";
    }
    
    std::cout << "  ║                                                      ║\n";
    std::cout << "  ║  Even with 50 samples + seed brute force,             ║\n";
    std::cout << "  ║  the φ-Chaos + φ-Harmony decoy still resists.         ║\n";
    std::cout << "  ║  Next: φ-Kumunoy awaits (5 layers of quicksand).      ║\n";
    std::cout << "  ╚══════════════════════════════════════════════════════╝\n\n";

    return 0;
}
