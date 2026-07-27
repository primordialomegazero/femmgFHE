// ATTACK 6: Balance Asymmetry Exploitation
// Exploit: Bit=0 → negative balance (56%), Bit=1 → positive balance (57%)
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

const double PHI = 1.618033988749895;
const double PSI = 0.6180339887498949;

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
    std::cout << "  ║  ATTACK 6: Balance Asymmetry Exploitation           ║\n";
    std::cout << "  ║  Using: Bit=0→Negative bias, Bit=1→Positive bias    ║\n";
    std::cout << "  ╚══════════════════════════════════════════════════════╝\n\n";

    CCParams<CryptoContextCKKSRNS> p;
    p.SetMultiplicativeDepth(20); p.SetScalingModSize(30); p.SetBatchSize(256);
    p.SetRingDim(2048); p.SetSecretKeyDist(UNIFORM_TERNARY); p.SetSecurityLevel(HEStd_NotSet);
    auto cc = GenCryptoContext(p);
    cc->Enable(PKE); cc->Enable(KEYSWITCH); cc->Enable(LEVELEDSHE);
    auto kp = cc->KeyGen(); cc->EvalMultKeyGen(kp.secretKey);

    // === PHASE 1: TRAINING — Learn balance thresholds ===
    const int TRAIN = 1000;
    std::vector<double> bal0, bal1;
    std::vector<double> snr0, snr1;  // Signal-to-noise ratio
    
    for (int t = 0; t < TRAIN; t++) {
        int secret = rand() % 2;
        PE ct = F_enc(cc, kp, secret);
        Reality1 r1 = R1_process(cc, ct);
        
        double sr = F_ratio(cc, kp, r1.signal);
        double nr = F_ratio(cc, kp, r1.noise);
        
        if (secret == 0) {
            bal0.push_back(r1.balance);
            snr0.push_back(sr - nr);
        } else {
            bal1.push_back(r1.balance);
            snr1.push_back(sr - nr);
        }
    }
    
    // Find optimal thresholds
    double avg_bal0 = 0, avg_bal1 = 0, avg_snr0 = 0, avg_snr1 = 0;
    for (double x : bal0) avg_bal0 += x;
    for (double x : bal1) avg_bal1 += x;
    for (double x : snr0) avg_snr0 += x;
    for (double x : snr1) avg_snr1 += x;
    avg_bal0 /= TRAIN; avg_bal1 /= TRAIN;
    avg_snr0 /= TRAIN; avg_snr1 /= TRAIN;
    
    double bal_threshold = (avg_bal0 + avg_bal1) / 2.0;
    double snr_threshold = (avg_snr0 + avg_snr1) / 2.0;
    
    std::cout << "  ┌──────────────────────────────────────────────────────┐\n";
    std::cout << "  │ TRAINING (" << TRAIN << " samples)                                  │\n";
    std::cout << "  ├──────────────────────────────────────────────────────┤\n";
    std::cout << "  │ Avg Balance (bit=0): " << std::fixed << std::setprecision(6) << avg_bal0 << "                    │\n";
    std::cout << "  │ Avg Balance (bit=1): " << avg_bal1 << "                    │\n";
    std::cout << "  │ Balance threshold:   " << bal_threshold << "                    │\n";
    std::cout << "  │ Avg SNR (bit=0):     " << avg_snr0 << "                    │\n";
    std::cout << "  │ Avg SNR (bit=1):     " << avg_snr1 << "                    │\n";
    std::cout << "  ├──────────────────────────────────────────────────────┤\n";
    
    // === PHASE 2: ATTACK WITH LEARNED THRESHOLDS ===
    const int TEST = 500;
    int correct_bal = 0, correct_snr = 0, correct_combined = 0;
    
    for (int t = 0; t < TEST; t++) {
        int secret = rand() % 2;
        PE ct = F_enc(cc, kp, secret);
        Reality1 r1 = R1_process(cc, ct);
        
        double sr = F_ratio(cc, kp, r1.signal);
        double nr = F_ratio(cc, kp, r1.noise);
        double snr = sr - nr;
        
        // Voter 1: Balance-based
        int pred_bal = (r1.balance > bal_threshold) ? 1 : 0;
        
        // Voter 2: SNR-based
        int pred_snr = (snr > snr_threshold) ? 1 : 0;
        
        // Voter 3: Combined (weighted vote)
        double score = (r1.balance - bal_threshold) * 1.0 + (snr - snr_threshold) * 0.5;
        int pred_combined = (score > 0) ? 1 : 0;
        
        if (pred_bal == secret) correct_bal++;
        if (pred_snr == secret) correct_snr++;
        if (pred_combined == secret) correct_combined++;
    }
    
    double acc_bal = 100.0 * correct_bal / TEST;
    double acc_snr = 100.0 * correct_snr / TEST;
    double acc_combined = 100.0 * correct_combined / TEST;
    
    std::cout << "  │ ATTACK RESULTS (" << TEST << " test samples)                      │\n";
    std::cout << "  │ Balance-based:    " << std::fixed << std::setprecision(1) << acc_bal << "%                              │\n";
    std::cout << "  │ SNR-based:        " << acc_snr << "%                              │\n";
    std::cout << "  │ Combined:         " << acc_combined << "%                              │\n";
    std::cout << "  ├──────────────────────────────────────────────────────┤\n";
    
    if (acc_combined > 55.0)
        std::cout << "  │ 🔴 REALITY 1 EXPLOITED! Asymmetry = vulnerability!   │\n";
    else if (acc_combined > 52.0)
        std::cout << "  │ ⚠ MINOR LEAK — Small asymmetry detected              │\n";
    else
        std::cout << "  │ ✓ REALITY 1 RESISTANT — Asymmetry not exploitable    │\n";
    
    std::cout << "  └──────────────────────────────────────────────────────┘\n\n";
    
    return 0;
}
