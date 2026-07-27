// ATTACK 8: Oracle Seed Reconstruction
// Strategy: Brute-force srand(time(0)) seeds by comparing outputs
// We know the algorithm 100% — we just need the seed
#include <iostream>
#include <iomanip>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <sys/time.h>
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

// Full Reality 1 processing with a GIVEN seed
PE R1_process_with_seed(CryptoContext<DCRTPoly>& cc, const PE& input, unsigned int seed, double& balance_out) {
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
    
    balance_out = balance;
    return signal;
}

// ═══════════════════════════════════════════
// ORACLE ATTACK
// ═══════════════════════════════════════════

// Step 1: Get ONE oracle response (the target we're attacking)
PE get_oracle_output(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, 
                     const PE& input, unsigned int secret_seed, double& balance) {
    srand(secret_seed);
    return R1_process_with_seed(cc, input, secret_seed, balance);
}

// Step 2: Brute force seeds in a time window
struct SeedCandidate {
    unsigned int seed;
    double score;
};

std::vector<SeedCandidate> brute_force_seeds(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp,
                                              const PE& input, double target_ratio,
                                              time_t window_start, int window_seconds) {
    std::vector<SeedCandidate> candidates;
    
    for (int sec = 0; sec < window_seconds; sec++) {
        for (int ms = 0; ms < 1000; ms += 10) {  // Sample every 10ms
            unsigned int test_seed = (unsigned int)(window_start + sec);
            // Add millisecond variation
            test_seed = test_seed * 1000 + ms;
            
            double dummy_balance;
            PE output = R1_process_with_seed(cc, input, test_seed, dummy_balance);
            double output_ratio = F_ratio(cc, kp, output);
            
            // Score: how close is this ratio to the target?
            double score = std::abs(output_ratio - target_ratio);
            
            candidates.push_back({test_seed, score});
        }
    }
    
    // Sort by score (lower = better)
    std::sort(candidates.begin(), candidates.end(), 
              [](const SeedCandidate& a, const SeedCandidate& b) {
                  return a.score < b.score;
              });
    
    return candidates;
}

// ═══════════════════════════════════════════
// MAIN ATTACK
// ═══════════════════════════════════════════
int main() {
    std::cout << "\n";
    std::cout << "  ╔══════════════════════════════════════════════════════╗\n";
    std::cout << "  ║  ATTACK 8: Oracle Seed Reconstruction               ║\n";
    std::cout << "  ║  Strategy: Brute-force srand(time(0)) seeds         ║\n";
    std::cout << "  ╚══════════════════════════════════════════════════════╝\n\n";

    CCParams<CryptoContextCKKSRNS> p;
    p.SetMultiplicativeDepth(50);
    p.SetScalingModSize(50);
    p.SetBatchSize(1024);
    p.SetRingDim(16384);
    p.SetSecretKeyDist(UNIFORM_TERNARY);
    p.SetSecurityLevel(HEStd_NotSet);
    
    auto cc = GenCryptoContext(p);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);
    
    auto kp = cc->KeyGen();
    cc->EvalMultKeyGen(kp.secretKey);

    // Get current time as the "secret seed window"
    time_t now = time(0);
    unsigned int secret_seed = (unsigned int)now;
    
    std::cout << "  ┌──────────────────────────────────────────────────────┐\n";
    std::cout << "  │ PHASE 1: Oracle Query                               │\n";
    std::cout << "  ├──────────────────────────────────────────────────────┤\n";
    
    // Oracle: Give us ONE output for a known input
    PE input = F_enc(cc, kp, 0);  // We know this is bit=0
    double target_balance;
    PE oracle_output = get_oracle_output(cc, kp, input, secret_seed, target_balance);
    double target_ratio = F_ratio(cc, kp, oracle_output);
    
    std::cout << "  │ Oracle seed: " << secret_seed << " (time=now)                    │\n";
    std::cout << "  │ Known input: bit=0                                        │\n";
    std::cout << "  │ Oracle ratio: " << std::fixed << std::setprecision(6) 
              << target_ratio << "                              │\n";
    std::cout << "  │ Oracle balance: " << target_balance << "                              │\n";
    std::cout << "  └──────────────────────────────────────────────────────┘\n\n";
    
    // ═══════════════════════════════════
    // PHASE 2: Brute Force Seeds
    // ═══════════════════════════════════
    std::cout << "  ┌──────────────────────────────────────────────────────┐\n";
    std::cout << "  │ PHASE 2: Seed Brute Force                            │\n";
    std::cout << "  ├──────────────────────────────────────────────────────┤\n";
    std::cout << "  │ Searching ±5 second window...                        │\n";
    
    time_t window_start = now - 5;
    int window_seconds = 10;
    
    std::vector<SeedCandidate> candidates = brute_force_seeds(
        cc, kp, input, target_ratio, window_start, window_seconds
    );
    
    std::cout << "  │ Tested " << candidates.size() << " candidate seeds                       │\n";
    std::cout << "  └──────────────────────────────────────────────────────┘\n\n";
    
    // ═══════════════════════════════════
    // PHASE 3: Verify Top Candidates
    // ═══════════════════════════════════
    std::cout << "  ┌──────────────────────────────────────────────────────┐\n";
    std::cout << "  │ PHASE 3: Candidate Verification                      │\n";
    std::cout << "  ├──────────────────────────────────────────────────────┤\n";
    
    bool found = false;
    int top_n = 20;  // Check top 20 candidates
    
    for (int i = 0; i < std::min(top_n, (int)candidates.size()); i++) {
        unsigned int candidate_seed = candidates[i].seed;
        double score = candidates[i].score;
        
        // Verify: Generate output with this seed
        double verify_balance;
        PE verify_output = R1_process_with_seed(cc, input, candidate_seed, verify_balance);
        double verify_ratio = F_ratio(cc, kp, verify_output);
        
        // Check if output matches oracle EXACTLY
        bool ratio_match = (std::abs(verify_ratio - target_ratio) < 1e-6);
        bool balance_match = (std::abs(verify_balance - target_balance) < 1e-6);
        bool full_match = ratio_match && balance_match;
        
        std::cout << "  │ #" << std::setw(2) << i+1 << " Seed:" << std::setw(12) << candidate_seed 
                  << " Score:" << std::fixed << std::setprecision(6) << score;
        
        if (full_match) {
            std::cout << " ★★★ EXACT MATCH! ★★★";
            found = true;
        } else if (ratio_match) {
            std::cout << " (ratio match)";
        }
        std::cout << "     │\n";
        
        if (full_match) {
            std::cout << "  ├──────────────────────────────────────────────────────┤\n";
            std::cout << "  │ SEED FOUND! " << candidate_seed << "                              │\n";
            
            // Now exploit the found seed
            std::cout << "  ├──────────────────────────────────────────────────────┤\n";
            std::cout << "  │ PHASE 4: Exploitation                                │\n";
            std::cout << "  ├──────────────────────────────────────────────────────┤\n";
            
            // With the seed, we can predict ANY output
            int test_trials = 200;
            int correct = 0;
            
            for (int t = 0; t < test_trials; t++) {
                int secret_bit = rand()%2;
                PE test_input = F_enc(cc, kp, secret_bit);
                
                // We know the seed, so we can reproduce the defense
                srand(candidate_seed);
                double pred_balance;
                PE pred_output = R1_process_with_seed(cc, test_input, candidate_seed, pred_balance);
                double pred_ratio = F_ratio(cc, kp, pred_output);
                
                // We ALSO know the ORIGINAL ratio for this input
                // With the seed, we can determine the "direction" of obfuscation
                int guess;
                if (pred_balance > 0) guess = 1;
                else if (pred_balance < 0) guess = 0;
                else guess = (pred_ratio > 0.5) ? 1 : 0;
                
                if (guess == secret_bit) correct++;
            }
            
            double success_rate = 100.0 * correct / test_trials;
            
            std::cout << "  │ Exploitation accuracy: " << std::fixed << std::setprecision(1) 
                      << success_rate << "% (" << correct << "/" << test_trials << ")";
            
            if (success_rate > 55) {
                std::cout << " ★ BREAKTHROUGH!";
            } else if (success_rate > 50) {
                std::cout << " (slight advantage)";
            } else {
                std::cout << " (still random — seed not enough?)";
            }
            std::cout << "     │\n";
            
            break;
        }
    }
    
    if (!found) {
        std::cout << "  │ No exact seed match found in top " << top_n << " candidates              │\n";
        std::cout << "  │ (Window may need to be wider, or countermeasures exist)│\n";
    }
    
    std::cout << "  └──────────────────────────────────────────────────────┘\n\n";
    
    // ═══════════════════════════════════
    // ANALYSIS
    // ═══════════════════════════════════
    std::cout << "  ╔══════════════════════════════════════════════════════╗\n";
    std::cout << "  ║  ATTACK 8 ANALYSIS                                   ║\n";
    std::cout << "  ╠══════════════════════════════════════════════════════╣\n";
    std::cout << "  ║                                                      ║\n";
    std::cout << "  ║  Strategy: Attack the RANDOM SEED, not the math      ║\n";
    std::cout << "  ║  Weakness: srand(time(0)) = 1-second resolution      ║\n";
    std::cout << "  ║  Defense:  φ-chaos makes ratio matching insufficient ║\n";
    std::cout << "  ║                                                      ║\n";
    std::cout << "  ║  If seed found: Can predict all outputs              ║\n";
    std::cout << "  ║  If seed NOT found: φ-chaos successfully masks       ║\n";
    std::cout << "  ║                    the seed even from brute force     ║\n";
    std::cout << "  ║                                                      ║\n";
    std::cout << "  ╚══════════════════════════════════════════════════════╝\n\n";

    return 0;
}
