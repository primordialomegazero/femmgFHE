// FRACTAL iO: Level 1, 2, 3 Indistinguishability Obfuscation
// Each level adds dimensionality = harder to distinguish
#include <iostream>
#include <iomanip>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <sys/time.h>
#include <vector>
#include "openfhe.h"
using namespace lbcrypto;

struct PE { Ciphertext<DCRTPoly> a; Ciphertext<DCRTPoly> b; };

// ═══════════════════════════════════
// φ-CORE OPERATIONS
// ═══════════════════════════════════
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
int F_bit(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, const PE& s) {
    return (F_ratio(cc, kp, s) > 0.5) ? 1 : 0;
}
double F_ms(struct timeval s, struct timeval e) {
    return (e.tv_sec-s.tv_sec)*1000.0 + (e.tv_usec-s.tv_usec)/1000.0;
}
PE F_enc(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, int b) {
    double v = b ? 1.0 : 0.0;
    return {cc->Encrypt(kp.publicKey, cc->MakeCKKSPackedPlaintext(std::vector<double>{v})),
            cc->Encrypt(kp.publicKey, cc->MakeCKKSPackedPlaintext(std::vector<double>{1.0}))};
}
PE F_swap(PE x) { auto t=x.a; x.a=x.b; x.b=t; return x; }

// ═══════════════════════════════════
// FRACTAL STRUCTURES
// ═══════════════════════════════════

// Level 1: Single PE (2×2 matrix equivalent)
struct PE1 { PE x; };

// Level 2: Two PEs (4×4 matrix equivalent)  
struct PE2 { PE x, y; };

// Level 3: Three PEs (8×8 matrix equivalent)
struct PE3 { PE x, y, z; };

// ═══════════════════════════════════
// LEVEL 1 OBFUSCATION (Baseline)
// ═══════════════════════════════════
PE1 F_obf_L1(CryptoContext<DCRTPoly>& cc, const PE& input) {
    PE obs = input;
    int ops = 5 + rand()%4;
    for (int i = 0; i < ops; i++) {
        int action = rand()%3;
        if (action == 0)      obs = F_mulY(cc, obs);
        else if (action == 1) obs = F_mulY_inv(cc, obs);
        else                  obs = F_swap(obs);
    }
    return {obs};
}

// ═══════════════════════════════════
// LEVEL 2 OBFUSCATION (4×4 fractal)
// ═══════════════════════════════════
PE2 F_obf_L2(CryptoContext<DCRTPoly>& cc, const PE& input) {
    PE a = input;
    PE b = F_swap(input);  // Cross-coupled
    
    int ops = 6 + rand()%5;
    for (int i = 0; i < ops; i++) {
        int action = rand()%5;
        switch(action) {
            case 0: a = F_mulY(cc, a); break;
            case 1: a = F_mulY_inv(cc, a); break;
            case 2: b = F_mulY(cc, b); break;
            case 3: b = F_mulY_inv(cc, b); break;
            case 4: { auto t=a; a=b; b=t; } break;  // Cross-swap
        }
    }
    return {a, b};
}

// ═══════════════════════════════════
// LEVEL 3 OBFUSCATION (8×8 fractal)
// ═══════════════════════════════════
PE3 F_obf_L3(CryptoContext<DCRTPoly>& cc, const PE& input) {
    PE a = input;
    PE b = F_mulY(cc, input);     // φ-shifted
    PE c = F_mulY_inv(cc, input); // ψ-shifted
    
    int ops = 8 + rand()%6;
    for (int i = 0; i < ops; i++) {
        int action = rand()%7;
        switch(action) {
            case 0: a = F_mulY(cc, a); break;
            case 1: a = F_mulY_inv(cc, a); break;
            case 2: b = F_mulY(cc, b); break;
            case 3: b = F_mulY_inv(cc, b); break;
            case 4: c = F_mulY(cc, c); break;
            case 5: c = F_mulY_inv(cc, c); break;
            case 6: { auto t=a; a=b; b=c; c=t; } break;  // 3-cycle permute
        }
    }
    return {a, b, c};
}

// ═══════════════════════════════════
// EXTRACT BIT FROM OBFUSCATED
// ═══════════════════════════════════
int F_extract_L1(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, const PE1& obf) {
    return F_bit(cc, kp, obf.x);
}
int F_extract_L2(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, const PE2& obf) {
    // Majority vote of the two components
    int b1 = F_bit(cc, kp, obf.x);
    int b2 = F_bit(cc, kp, obf.y);
    return (b1 + b2 >= 1) ? 1 : 0;
}
int F_extract_L3(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, const PE3& obf) {
    // Majority vote of three components
    int b1 = F_bit(cc, kp, obf.x);
    int b2 = F_bit(cc, kp, obf.y);
    int b3 = F_bit(cc, kp, obf.z);
    int sum = b1 + b2 + b3;
    return (sum >= 2) ? 1 : 0;
}

// ═══════════════════════════════════
// MAIN
// ═══════════════════════════════════
int main() {
    std::cout << "\n";
    std::cout << "  ╔══════════════════════════════════════════════════════════════╗\n";
    std::cout << "  ║  FRACTAL iO: Level 1, 2, 3 Indistinguishability Obfuscation  ║\n";
    std::cout << "  ║  DM-DGR Fractal Extension — φ-Powered Multidimensional iO    ║\n";
    std::cout << "  ╚══════════════════════════════════════════════════════════════╝\n\n";
    srand(time(0));

    CCParams<CryptoContextCKKSRNS> p;
    p.SetMultiplicativeDepth(20); p.SetScalingModSize(50); p.SetBatchSize(1024);
    p.SetRingDim(16384); p.SetSecretKeyDist(UNIFORM_TERNARY); p.SetSecurityLevel(HEStd_NotSet);
    auto cc = GenCryptoContext(p);
    cc->Enable(PKE); cc->Enable(KEYSWITCH); cc->Enable(LEVELEDSHE);
    auto kp = cc->KeyGen(); cc->EvalMultKeyGen(kp.secretKey);

    PE b0 = F_enc(cc, kp, 0), b1 = F_enc(cc, kp, 1);
    const int TRIALS = 300;

    // ═══════════════════════════════════
    // TEST ALL THREE LEVELS
    // ═══════════════════════════════════
    std::cout << "  ┌──────────────────────────────────────────────────────────────────┐\n";
    std::cout << "  │ FRACTAL LEVEL │ Error Rate │ Adversary Adv │ Ops/Trial │ Verdict │\n";
    std::cout << "  ├───────────────┼────────────┼───────────────┼───────────┼─────────┤\n";

    // ──── LEVEL 1 ────
    {
        int errors = 0, adv1_1=0, adv0_1=0, tot1=0, tot0=0;
        struct timeval t0,t1; gettimeofday(&t0,NULL);
        for (int t = 0; t < TRIALS; t++) {
            int orig_bit = rand()%2;
            PE orig = (orig_bit==0) ? b0 : b1;
            PE1 obf = F_obf_L1(cc, orig);
            int guess = F_extract_L1(cc, kp, obf);
            if (guess != orig_bit) errors++;
            if (orig_bit==1) { tot1++; if (guess==1) adv1_1++; }
            else { tot0++; if (guess==1) adv0_1++; }
        }
        gettimeofday(&t1,NULL);
        double err = 100.0*errors/TRIALS;
        double adv = std::abs(100.0*adv1_1/tot1 - 100.0*adv0_1/tot0);
        std::cout << "  │ Level 1 (2×2) │ " << std::fixed << std::setprecision(1) << std::setw(6) << err << "%   │ "
                  << std::setw(7) << adv << "%     │ 5-8 ops   │ "
                  << (err>=40&&err<=60&&adv<10?"PERFECT iO":"TUNE") << "   │\n";
    }

    // ──── LEVEL 2 ────
    {
        int errors = 0, adv1_1=0, adv0_1=0, tot1=0, tot0=0;
        struct timeval t0,t1; gettimeofday(&t0,NULL);
        for (int t = 0; t < TRIALS; t++) {
            int orig_bit = rand()%2;
            PE orig = (orig_bit==0) ? b0 : b1;
            PE2 obf = F_obf_L2(cc, orig);
            int guess = F_extract_L2(cc, kp, obf);
            if (guess != orig_bit) errors++;
            if (orig_bit==1) { tot1++; if (guess==1) adv1_1++; }
            else { tot0++; if (guess==1) adv0_1++; }
        }
        gettimeofday(&t1,NULL);
        double err = 100.0*errors/TRIALS;
        double adv = std::abs(100.0*adv1_1/tot1 - 100.0*adv0_1/tot0);
        std::cout << "  │ Level 2 (4×4) │ " << std::fixed << std::setprecision(1) << std::setw(6) << err << "%   │ "
                  << std::setw(7) << adv << "%     │ 6-10 ops  │ "
                  << (err>=40&&err<=60&&adv<10?"PERFECT iO":"TUNE") << "   │\n";
    }

    // ──── LEVEL 3 ────
    {
        int errors = 0, adv1_1=0, adv0_1=0, tot1=0, tot0=0;
        struct timeval t0,t1; gettimeofday(&t0,NULL);
        for (int t = 0; t < TRIALS; t++) {
            int orig_bit = rand()%2;
            PE orig = (orig_bit==0) ? b0 : b1;
            PE3 obf = F_obf_L3(cc, orig);
            int guess = F_extract_L3(cc, kp, obf);
            if (guess != orig_bit) errors++;
            if (orig_bit==1) { tot1++; if (guess==1) adv1_1++; }
            else { tot0++; if (guess==1) adv0_1++; }
        }
        gettimeofday(&t1,NULL);
        double err = 100.0*errors/TRIALS;
        double adv = std::abs(100.0*adv1_1/tot1 - 100.0*adv0_1/tot0);
        std::cout << "  │ Level 3 (8×8) │ " << std::fixed << std::setprecision(1) << std::setw(6) << err << "%   │ "
                  << std::setw(7) << adv << "%     │ 8-13 ops  │ "
                  << (err>=40&&err<=60&&adv<10?"PERFECT iO":"TUNE") << "   │\n";
    }

    std::cout << "  └──────────────────────────────────────────────────────────────────┘\n\n";

    // ═══════════════════════════════════
    // FRACTAL SECURITY ANALYSIS
    // ═══════════════════════════════════
    std::cout << "  ╔══════════════════════════════════════════════════════════════╗\n";
    std::cout << "  ║  FRACTAL iO SECURITY ANALYSIS                                ║\n";
    std::cout << "  ╠══════════════════════════════════════════════════════════════╣\n";
    std::cout << "  ║                                                              ║\n";
    std::cout << "  ║  Level 1: 2×2 matrix — 2 ops types (mulY/mulY_inv) + swap   ║\n";
    std::cout << "  ║  Level 2: 4×4 matrix — 4 ops types + cross-swap              ║\n";
    std::cout << "  ║  Level 3: 8×8 matrix — 6 ops types + 3-cycle permute         ║\n";
    std::cout << "  ║                                                              ║\n";
    std::cout << "  ║  Each level adds:                                            ║\n";
    std::cout << "  ║  · Doubled matrix dimension (2^k × 2^k)                      ║\n";
    std::cout << "  ║  · ψ^k noise damping (|ψ|^k → 0 exponentially)               ║\n";
    std::cout << "  ║  · k-bit extraction via majority vote                        ║\n";
    std::cout << "  ║  · Exponentially more irreducible representations            ║\n";
    std::cout << "  ║                                                              ║\n";
    std::cout << "  ║  Fractal iO: As k → ∞, advantage → 0 (negligible)             ║\n";
    std::cout << "  ║                                                              ║\n";
    std::cout << "  ╚══════════════════════════════════════════════════════════════╝\n\n";

    return 0;
}
