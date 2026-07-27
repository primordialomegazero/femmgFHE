// ATTACK 4: FUSION — Combined all attack vectors
// Ratio + Tension + Direction + Counts → Machine Learning style
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

// Feature vector for ML-style attack
struct Features {
    double re, rc, tension, avg_ratio;
    bool tension_positive, expand_dominates;
};

int main() {
    srand(time(0));
    
    std::cout << "\n";
    std::cout << "  ╔══════════════════════════════════════════════════════╗\n";
    std::cout << "  ║  ATTACK 4: FUSION — All Vectors Combined            ║\n";
    std::cout << "  ║  Ratio + Tension + Direction → Voting Classifier    ║\n";
    std::cout << "  ╚══════════════════════════════════════════════════════╝\n\n";

    CCParams<CryptoContextCKKSRNS> p;
    p.SetMultiplicativeDepth(20); p.SetScalingModSize(30); p.SetBatchSize(256);
    p.SetRingDim(2048); p.SetSecretKeyDist(UNIFORM_TERNARY); p.SetSecurityLevel(HEStd_NotSet);
    auto cc = GenCryptoContext(p);
    cc->Enable(PKE); cc->Enable(KEYSWITCH); cc->Enable(LEVELEDSHE);
    auto kp = cc->KeyGen(); cc->EvalMultKeyGen(kp.secretKey);

    const int TRAIN_SAMPLES = 300;
    const int TEST_SAMPLES = 200;
    
    // === TRAINING PHASE ===
    std::vector<Features> train_0, train_1;
    
    for (int t = 0; t < TRAIN_SAMPLES; t++) {
        int secret = rand() % 2;
        PE ct = F_enc(cc, kp, secret);
        FractalNode fractal = F_recursive_dual(cc, ct, 2);
        
        Features f;
        f.re = F_ratio(cc, kp, fractal.expand);
        f.rc = F_ratio(cc, kp, fractal.contract);
        f.tension = f.re - f.rc;
        f.avg_ratio = (f.re + f.rc) / 2.0;
        f.tension_positive = (f.tension > 0);
        f.expand_dominates = (f.re > f.rc);
        
        if (secret == 0) train_0.push_back(f);
        else train_1.push_back(f);
    }
    
    // Train: find distinguishing features
    double avg_re0=0, avg_re1=0, avg_rc0=0, avg_rc1=0, avg_t0=0, avg_t1=0;
    int pos_t0=0, pos_t1=0, exp_d0=0, exp_d1=0;
    
    for (auto& f : train_0) {
        avg_re0 += f.re; avg_rc0 += f.rc; avg_t0 += f.tension;
        if (f.tension_positive) pos_t0++;
        if (f.expand_dominates) exp_d0++;
    }
    for (auto& f : train_1) {
        avg_re1 += f.re; avg_rc1 += f.rc; avg_t1 += f.tension;
        if (f.tension_positive) pos_t1++;
        if (f.expand_dominates) exp_d1++;
    }
    
    double n = TRAIN_SAMPLES / 2.0;
    avg_re0 /= n; avg_rc0 /= n; avg_t0 /= n;
    avg_re1 /= n; avg_rc1 /= n; avg_t1 /= n;
    
    std::cout << "  ┌──────────────────────────────────────────────────────┐\n";
    std::cout << "  │ TRAINING RESULTS (" << TRAIN_SAMPLES << " samples)                         │\n";
    std::cout << "  ├──────────────────────────────────────────────────────┤\n";
    std::cout << "  │ Avg Re (bit=0):  " << std::fixed << std::setprecision(6) << avg_re0 << "                         │\n";
    std::cout << "  │ Avg Re (bit=1):  " << avg_re1 << "                         │\n";
    std::cout << "  │ Avg Tension (0): " << avg_t0 << "                         │\n";
    std::cout << "  │ Avg Tension (1): " << avg_t1 << "                         │\n";
    std::cout << "  │ Pos Tension (0): " << (100.0*pos_t0/n) << "%                            │\n";
    std::cout << "  │ Pos Tension (1): " << (100.0*pos_t1/n) << "%                            │\n";
    std::cout << "  ├──────────────────────────────────────────────────────┤\n";
    
    // === ATTACK PHASE: Voting Classifier ===
    int correct = 0;
    int vote_correct = 0;
    
    for (int t = 0; t < TEST_SAMPLES; t++) {
        int secret = rand() % 2;
        PE ct = F_enc(cc, kp, secret);
        FractalNode fractal = F_recursive_dual(cc, ct, 2);
        
        Features f;
        f.re = F_ratio(cc, kp, fractal.expand);
        f.rc = F_ratio(cc, kp, fractal.contract);
        f.tension = f.re - f.rc;
        f.avg_ratio = (f.re + f.rc) / 2.0;
        f.tension_positive = (f.tension > 0);
        f.expand_dominates = (f.re > f.rc);
        
        // Voter 1: Ratio proximity
        double dist0 = std::abs(f.avg_ratio - (avg_re0+avg_rc0)/2.0);
        double dist1 = std::abs(f.avg_ratio - (avg_re1+avg_rc1)/2.0);
        int vote1 = (dist0 < dist1) ? 0 : 1;
        
        // Voter 2: Tension proximity
        double tdist0 = std::abs(f.tension - avg_t0);
        double tdist1 = std::abs(f.tension - avg_t1);
        int vote2 = (tdist0 < tdist1) ? 0 : 1;
        
        // Voter 3: Tension sign majority
        int vote3 = f.tension_positive ? 
                    (pos_t1 > pos_t0 ? 1 : 0) : 
                    (pos_t0 > pos_t1 ? 0 : 1);
        
        // Voter 4: Expand dominance
        int vote4 = f.expand_dominates ?
                    (exp_d1 > exp_d0 ? 1 : 0) :
                    (exp_d0 > exp_d1 ? 0 : 1);
        
        // Majority vote
        int votes_for_1 = vote1 + vote2 + vote3 + vote4;
        int prediction = (votes_for_1 >= 2) ? 1 : 0;
        
        if (prediction == secret) correct++;
    }
    
    double accuracy = 100.0 * correct / TEST_SAMPLES;
    
    std::cout << "  │ FUSION ATTACK RESULTS                                │\n";
    std::cout << "  │ Test samples: " << TEST_SAMPLES << "                                      │\n";
    std::cout << "  │ Correct predictions: " << correct << "/" << TEST_SAMPLES << "                            │\n";
    std::cout << "  │ Accuracy: " << std::fixed << std::setprecision(1) << accuracy << "%                                        │\n";
    std::cout << "  │                                                      │\n";
    
    if (accuracy < 45.0)
        std::cout << "  │ ❌❌❌ ANTI-CORRELATED — Worse than random!         │\n";
    else if (accuracy < 55.0)
        std::cout << "  │ ✓ ATTACK FAILED — Statistically insignificant       │\n";
    else if (accuracy < 70.0)
        std::cout << "  │ ⚠ WEAK CORRELATION — Slight leakage detected        │\n";
    else
        std::cout << "  │ 🔴 VULNERABLE — Fusion attack breaks iO!            │\n";
    
    std::cout << "  └──────────────────────────────────────────────────────┘\n\n";
    
    return 0;
}
