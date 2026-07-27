// ATTACK 10+11: FAST VERSION (RingDim 8192)
// Validate 60.5% + First Strike on φ-Kumunoy
// 4x faster, more samples, faster iteration
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
// BASIC REALITY 1 (Original Decoy)
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

PE basic_reality1(CryptoContext<DCRTPoly>& cc, const PE& input, double& balance_out) {
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
// φ-KUMUNOY (Quicksand Decoy)
// ═══════════════════════════════════════════
void R1_quicksand_step(CryptoContext<DCRTPoly>& cc, PE& surface, PE& undertow, 
                        double& depth, int& rotation) {
    rotation = (rotation + 1) % 8;
    switch(rotation) {
        case 0: surface = F_mulY(cc, surface); undertow = F_mulY_inv(cc, undertow); depth += PSI * 0.1; break;
        case 1: surface = F_mulY_inv(cc, surface); undertow = F_mulY(cc, undertow); depth += PSI * 0.1; break;
        case 2: surface = F_mulY(cc, surface); undertow = F_mulY_inv(cc, undertow); depth += PHI * 0.05; break;
        case 3: surface = F_mulY_inv(cc, surface); undertow = F_mulY(cc, undertow); depth += PHI * 0.05; break;
        case 4: surface = F_swap(surface); depth += PSI * 0.15; break;
        case 5: undertow = F_swap(undertow); depth += PSI * 0.1; break;
        case 6: { auto t = surface; surface = undertow; undertow = t; } depth += PHI * 0.08; break;
        case 7: surface = F_mulY(cc, surface); surface = F_mulY_inv(cc, surface); depth += PSI * PSI * 0.1; break;
    }
    if (depth > 1.0) depth = 1.0;
}

PE kumunoy_reality1(CryptoContext<DCRTPoly>& cc, const PE& input, 
                     int layers, double& depth_out) {
    PE surface = input;
    PE undertow = input;
    double depth = 0.0;
    int rotation = rand()%8;
    
    for (int l = 0; l < layers; l++) {
        int steps = 3 + (int)(depth * 5);
        for (int s = 0; s < steps; s++) {
            R1_quicksand_step(cc, surface, undertow, depth, rotation);
        }
        if (l < layers - 1) {
            surface = F_mulY(cc, surface);
            undertow = F_mulY_inv(cc, undertow);
        }
    }
    depth_out = depth;
    return surface;
}

// ═══════════════════════════════════════════
// MAIN ATTACK
// ═══════════════════════════════════════════
int main() {
    std::cout << "\n";
    std::cout << "  ╔══════════════════════════════════════════════════════╗\n";
    std::cout << "  ║  ATTACK 10+11: FAST VERSION (RingDim 8192)          ║\n";
    std::cout << "  ║  4x Faster — 1500 validation samples                ║\n";
    std::cout << "  ╚══════════════════════════════════════════════════════╝\n\n";

    CCParams<CryptoContextCKKSRNS> p;
    p.SetMultiplicativeDepth(100);
    p.SetScalingModSize(50);
    p.SetBatchSize(1024);
    p.SetRingDim(8192);  // FAST MODE
    p.SetSecretKeyDist(UNIFORM_TERNARY);
    p.SetSecurityLevel(HEStd_NotSet);
    
    std::cout << "  Initializing FHE (RingDim=8192, FAST MODE)...\n";
    struct timeval t_start, t_end;
    gettimeofday(&t_start, NULL);
    
    auto cc = GenCryptoContext(p);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);
    
    auto kp = cc->KeyGen();
    cc->EvalMultKeyGen(kp.secretKey);
    
    gettimeofday(&t_end, NULL);
    double init_time = (t_end.tv_sec - t_start.tv_sec) * 1000.0 + 
                       (t_end.tv_usec - t_start.tv_usec) / 1000.0;
    std::cout << "  FHE initialized in " << std::fixed << std::setprecision(0) << init_time << "ms\n\n";

    // ═══════════════════════════════════
    // PHASE 1: VALIDATE BASIC DECOY
    // ═══════════════════════════════════
    std::cout << "  ┌──────────────────────────────────────────────────────┐\n";
    std::cout << "  │ PHASE 1: Validate on Basic Decoy (200 train + 1500 test)│\n";
    std::cout << "  ├──────────────────────────────────────────────────────┤\n";
    
    gettimeofday(&t_start, NULL);
    
    // Train
    std::vector<double> ratios_0, ratios_1;
    for (int i = 0; i < 200; i++) {
        srand(time(0) + i * 1000);
        PE input_0 = F_enc(cc, kp, 0);
        PE input_1 = F_enc(cc, kp, 1);
        double bal_dummy;
        PE out_0 = basic_reality1(cc, input_0, bal_dummy);
        PE out_1 = basic_reality1(cc, input_1, bal_dummy);
        ratios_0.push_back(F_ratio(cc, kp, out_0));
        ratios_1.push_back(F_ratio(cc, kp, out_1));
    }
    
    double avg_0 = std::accumulate(ratios_0.begin(), ratios_0.end(), 0.0) / 200;
    double avg_1 = std::accumulate(ratios_1.begin(), ratios_1.end(), 0.0) / 200;
    double threshold = (avg_0 + avg_1) / 2.0;
    
    std::cout << "  │ Training (200 samples):                               │\n";
    std::cout << "  │   Avg Ratio bit=0: " << std::fixed << std::setprecision(4) << avg_0 << "                            │\n";
    std::cout << "  │   Avg Ratio bit=1: " << avg_1 << "                            │\n";
    std::cout << "  │   Threshold:       " << threshold << "                            │\n";
    
    // Test 1500
    int correct = 0;
    for (int t = 0; t < 1500; t++) {
        int secret = rand()%2;
        srand(time(0) + t * 5000 + rand()%1000);
        PE input = F_enc(cc, kp, secret);
        double bal_dummy;
        PE output = basic_reality1(cc, input, bal_dummy);
        double ratio = F_ratio(cc, kp, output);
        
        int guess = (ratio > threshold) ? 1 : 0;
        if (guess == secret) correct++;
        
        if ((t+1) % 500 == 0) {
            double current_acc = 100.0 * correct / (t+1);
            std::cout << "  │   " << (t+1) << " trials: " << std::setprecision(1) << current_acc << "%                      │\n";
        }
    }
    
    double final_acc_basic = 100.0 * correct / 1500;
    
    gettimeofday(&t_end, NULL);
    double phase1_time = (t_end.tv_sec - t_start.tv_sec) * 1000.0 + 
                         (t_end.tv_usec - t_start.tv_usec) / 1000.0;
    
    std::cout << "  ├──────────────────────────────────────────────────────┤\n";
    std::cout << "  │ ★ BASIC DECOY: " << std::setprecision(1) << final_acc_basic << "% (" << correct << "/1500) in " 
              << std::setprecision(0) << phase1_time << "ms          │\n";
    
    std::string basic_status;
    if (final_acc_basic > 60) basic_status = "BREAKTHROUGH CONFIRMED!";
    else if (final_acc_basic > 55) basic_status = "SIGNAL DETECTED";
    else if (final_acc_basic > 52) basic_status = "WEAK SIGNAL (may be noise)";
    else basic_status = "NO SIGNAL — 60.5% was RingDim artifact";
    
    std::cout << "  │ " << basic_status << "                              │\n";
    std::cout << "  └──────────────────────────────────────────────────────┘\n\n";
    
    // ═══════════════════════════════════
    // PHASE 2: FIRST STRIKE ON φ-KUMUNOY
    // ═══════════════════════════════════
    std::cout << "  ┌──────────────────────────────────────────────────────┐\n";
    std::cout << "  │ PHASE 2: First Strike on φ-KUMUNOY (1-5 layers)       │\n";
    std::cout << "  ├──────────┬──────────────┬──────────────┬─────────────┤\n";
    std::cout << "  │ Layers   │ Accuracy     │ Depth (avg)  │ Status      │\n";
    std::cout << "  ├──────────┼──────────────┼──────────────┼─────────────┤\n";
    
    gettimeofday(&t_start, NULL);
    
    for (int layers = 1; layers <= 5; layers++) {
        int kumunoy_correct = 0;
        double total_depth = 0;
        int kumunoy_trials = 500;  // More trials kasi mabilis
        
        for (int t = 0; t < kumunoy_trials; t++) {
            int secret = rand()%2;
            srand(time(0) + t * 7000 + layers * 10000);
            PE input = F_enc(cc, kp, secret);
            double depth;
            PE output = kumunoy_reality1(cc, input, layers, depth);
            double ratio = F_ratio(cc, kp, output);
            total_depth += depth;
            
            int guess = (ratio > threshold) ? 1 : 0;
            if (guess == secret) kumunoy_correct++;
        }
        
        double kumunoy_acc = 100.0 * kumunoy_correct / kumunoy_trials;
        double avg_depth = total_depth / kumunoy_trials;
        
        std::string k_status;
        if (kumunoy_acc > 55) k_status = "RESISTING?";
        else if (kumunoy_acc > 52) k_status = "SINKING SLOW";
        else if (kumunoy_acc > 48) k_status = "DROWNING";
        else k_status = "BOTTOMLESS";
        
        std::cout << "  │ " << std::setw(5) << layers << "      │ "
                  << std::fixed << std::setprecision(1) << std::setw(8) << kumunoy_acc << "%     │ "
                  << std::setw(8) << std::setprecision(3) << avg_depth << "     │ "
                  << k_status << "        │\n";
    }
    
    gettimeofday(&t_end, NULL);
    double phase2_time = (t_end.tv_sec - t_start.tv_sec) * 1000.0 + 
                         (t_end.tv_usec - t_start.tv_usec) / 1000.0;
    
    std::cout << "  ├──────────┴──────────────┴──────────────┴─────────────┤\n";
    std::cout << "  │ Phase 2 completed in " << std::setprecision(0) << phase2_time << "ms                         │\n";
    std::cout << "  │ Threshold from basic decoy: " << std::setprecision(4) << threshold << "               │\n";
    std::cout << "  └──────────────────────────────────────────────────────┘\n\n";
    
    // ═══════════════════════════════════
    // BATTLE REPORT
    // ═══════════════════════════════════
    std::cout << "  ╔══════════════════════════════════════════════════════╗\n";
    std::cout << "  ║  BATTLE REPORT: Attack 10+11 (FAST MODE)            ║\n";
    std::cout << "  ╠══════════════════════════════════════════════════════╣\n";
    std::cout << "  ║                                                      ║\n";
    std::cout << "  ║  RingDim: 8192 (4x faster than production)           ║\n";
    std::cout << "  ║  Basic Decoy Accuracy: " << std::setw(5) << std::setprecision(1) << final_acc_basic << "%                      ║\n";
    
    if (final_acc_basic > 60) {
        std::cout << "  ║  ★ WEAKNESS CONFIRMED in basic Reality 1!            ║\n";
    } else if (final_acc_basic > 55) {
        std::cout << "  ║  ★ Signal exists but weak. Needs improvement.        ║\n";
    } else {
        std::cout << "  ║  Previous 60.5% = RingDim artifact. Back to zero.    ║\n";
    }
    
    std::cout << "  ║                                                      ║\n";
    std::cout << "  ║  φ-Kumunoy Status:                                   ║\n";
    std::cout << "  ║  → Predictor from basic decoy TRANSFER to quicksand  ║\n";
    std::cout << "  ║  → Need to train predictor DIRECTLY on φ-Kumunoy     ║\n";
    std::cout << "  ║                                                      ║\n";
    std::cout << "  ║  Total Time: " << std::setprecision(0) << (phase1_time + phase2_time + init_time) << "ms                              ║\n";
    std::cout << "  ╚══════════════════════════════════════════════════════╝\n\n";

    return 0;
}
