// DM-DGR iO TUNING: Find optimal obfuscation parameters
#include <iostream>
#include <iomanip>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <sys/time.h>
#include "openfhe.h"
using namespace lbcrypto;

struct PE { Ciphertext<DCRTPoly> a; Ciphertext<DCRTPoly> b; };

PE DM_mulY(CryptoContext<DCRTPoly>& cc, const PE& x) {
    return {x.b, cc->EvalAdd(x.a, x.b)};
}
PE DM_mulY_inv(CryptoContext<DCRTPoly>& cc, const PE& x) {
    return {cc->EvalSub(x.b, x.a), x.a};
}
double DM_val(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, const Ciphertext<DCRTPoly>& c) {
    Plaintext pt; cc->Decrypt(kp.secretKey, c, &pt);
    return pt->GetCKKSPackedValue()[0].real();
}
double DM_ratio(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, const PE& s) {
    double a = DM_val(cc, kp, s.a), b = DM_val(cc, kp, s.b);
    return (std::abs(b) > 1e-10) ? a / b : a;
}
int DM_bit(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, const PE& s) {
    return (DM_ratio(cc, kp, s) > 0.5) ? 1 : 0;
}
double DM_ms(struct timeval s, struct timeval e) {
    return (e.tv_sec-s.tv_sec)*1000.0 + (e.tv_usec-s.tv_usec)/1000.0;
}
PE DM_enc(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, int b) {
    double v = b ? 1.0 : 0.0;
    return {cc->Encrypt(kp.publicKey, cc->MakeCKKSPackedPlaintext(std::vector<double>{v})),
            cc->Encrypt(kp.publicKey, cc->MakeCKKSPackedPlaintext(std::vector<double>{1.0}))};
}
PE DM_swap(PE x) {
    auto tmp = x.a; x.a = x.b; x.b = tmp;
    return x;
}

int main() {
    std::cout << "\n";
    std::cout << "  ╔══════════════════════════════════════════════════╗\n";
    std::cout << "  ║  DM-DGR iO TUNING — Obfuscation Parameter Sweep   ║\n";
    std::cout << "  ╚══════════════════════════════════════════════════╝\n\n";
    srand(time(0));

    CCParams<CryptoContextCKKSRNS> p;
    p.SetMultiplicativeDepth(20); p.SetScalingModSize(50); p.SetBatchSize(1024);
    p.SetRingDim(16384); p.SetSecretKeyDist(UNIFORM_TERNARY); p.SetSecurityLevel(HEStd_NotSet);
    auto cc = GenCryptoContext(p);
    cc->Enable(PKE); cc->Enable(KEYSWITCH); cc->Enable(LEVELEDSHE);
    auto kp = cc->KeyGen(); cc->EvalMultKeyGen(kp.secretKey);

    PE b0 = DM_enc(cc, kp, 0), b1 = DM_enc(cc, kp, 1);
    const int TRIALS = 200;

    std::cout << "  ┌──────────────────────────────────────────────────────────────┐\n";
    std::cout << "  │ iO STRATEGY                  │ Error Rate │ Target  │ Verdict │\n";
    std::cout << "  ├──────────────────────────────┼────────────┼─────────┼─────────┤\n";

    // ──── STRATEGY 1: Original (2-3 random ops: mulY or swap) ────
    {
        int wrong = 0;
        for (int t = 0; t < TRIALS; t++) {
            PE orig = (rand()%2) ? b1 : b0;
            PE obs = orig;
            int ops = 2 + rand()%2;
            for (int i = 0; i < ops; i++) {
                if (rand()%2) obs = DM_mulY(cc, obs);
                else obs = DM_swap(obs);
            }
            if ((DM_ratio(cc,kp,obs) > 0.5) != (DM_ratio(cc,kp,orig) > 0.5)) wrong++;
        }
        double pct = 100.0 * wrong / TRIALS;
        std::cout << "  │ Original (2-3: mulY/swap)     │ " << std::fixed << std::setprecision(1) << std::setw(6) << pct << "%  │ ~50%    │ "
                  << (pct>=40 && pct<=60 ? "PERFECT" : (pct>=35 && pct<=65 ? "OK" : "TUNE")) << "  │\n";
    }

    // ──── STRATEGY 2: mulY only (no swap) ────
    {
        int wrong = 0;
        for (int t = 0; t < TRIALS; t++) {
            PE orig = (rand()%2) ? b1 : b0;
            PE obs = orig;
            int ops = 2 + rand()%3;
            for (int i = 0; i < ops; i++) obs = DM_mulY(cc, obs);
            if ((DM_ratio(cc,kp,obs) > 0.5) != (DM_ratio(cc,kp,orig) > 0.5)) wrong++;
        }
        double pct = 100.0 * wrong / TRIALS;
        std::cout << "  │ mulY only (2-4 ops)           │ " << std::fixed << std::setprecision(1) << std::setw(6) << pct << "%  │ ~50%    │ "
                  << (pct>=40 && pct<=60 ? "PERFECT" : (pct>=35 && pct<=65 ? "OK" : "TUNE")) << "  │\n";
    }

    // ──── STRATEGY 3: mulY + mulY_inv alternating ────
    {
        int wrong = 0;
        for (int t = 0; t < TRIALS; t++) {
            PE orig = (rand()%2) ? b1 : b0;
            PE obs = orig;
            int ops = 2 + rand()%2;
            for (int i = 0; i < ops; i++) {
                obs = DM_mulY(cc, obs);
                obs = DM_mulY_inv(cc, obs);
            }
            if ((DM_ratio(cc,kp,obs) > 0.5) != (DM_ratio(cc,kp,orig) > 0.5)) wrong++;
        }
        double pct = 100.0 * wrong / TRIALS;
        std::cout << "  │ mulY+mulY_inv pairs (2-3x)    │ " << std::fixed << std::setprecision(1) << std::setw(6) << pct << "%  │ ~50%    │ "
                  << (pct>=40 && pct<=60 ? "PERFECT" : (pct>=35 && pct<=65 ? "OK" : "TUNE")) << "  │\n";
    }

    // ──── STRATEGY 4: F4B4 full cycle per obfuscation ────
    {
        int wrong = 0;
        for (int t = 0; t < TRIALS; t++) {
            PE orig = (rand()%2) ? b1 : b0;
            PE obs = orig;
            int rounds = 1 + rand()%2;
            for (int r = 0; r < rounds; r++) {
                for (int i = 0; i < 4; i++) obs = DM_mulY(cc, obs);
                for (int i = 0; i < 4; i++) obs = DM_mulY_inv(cc, obs);
            }
            if ((DM_ratio(cc,kp,obs) > 0.5) != (DM_ratio(cc,kp,orig) > 0.5)) wrong++;
        }
        double pct = 100.0 * wrong / TRIALS;
        std::cout << "  │ F4B4 full cycle (1-2 rounds)  │ " << std::fixed << std::setprecision(1) << std::setw(6) << pct << "%  │ ~50%    │ "
                  << (pct>=40 && pct<=60 ? "PERFECT" : (pct>=35 && pct<=65 ? "OK" : "TUNE")) << "  │\n";
    }

    // ──── STRATEGY 5: Heavy obfuscation (5-8 random ops) ────
    {
        int wrong = 0;
        for (int t = 0; t < TRIALS; t++) {
            PE orig = (rand()%2) ? b1 : b0;
            PE obs = orig;
            int ops = 5 + rand()%4;
            for (int i = 0; i < ops; i++) {
                int action = rand()%3;
                if (action == 0) obs = DM_mulY(cc, obs);
                else if (action == 1) obs = DM_mulY_inv(cc, obs);
                else obs = DM_swap(obs);
            }
            if ((DM_ratio(cc,kp,obs) > 0.5) != (DM_ratio(cc,kp,orig) > 0.5)) wrong++;
        }
        double pct = 100.0 * wrong / TRIALS;
        std::cout << "  │ Heavy (5-8: mulY/mulY_inv/swap)│ " << std::fixed << std::setprecision(1) << std::setw(6) << pct << "%  │ ~50%    │ "
                  << (pct>=40 && pct<=60 ? "PERFECT" : (pct>=35 && pct<=65 ? "OK" : "TUNE")) << "  │\n";
    }

    // ──── STRATEGY 6: Light + noise injection ────
    {
        int wrong = 0;
        for (int t = 0; t < TRIALS; t++) {
            PE orig = (rand()%2) ? b1 : b0;
            PE obs = orig;
            // 1-2 mulY to shift ratio
            int warmup = 1 + rand()%2;
            for (int i = 0; i < warmup; i++) obs = DM_mulY(cc, obs);
            // then 1-2 mulY_inv to bring back but with accumulated CKKS noise
            int cooldown = 1 + rand()%2;
            for (int i = 0; i < cooldown; i++) obs = DM_mulY_inv(cc, obs);
            if ((DM_ratio(cc,kp,obs) > 0.5) != (DM_ratio(cc,kp,orig) > 0.5)) wrong++;
        }
        double pct = 100.0 * wrong / TRIALS;
        std::cout << "  │ Asymmetric noise (1-2 fwd+inv) │ " << std::fixed << std::setprecision(1) << std::setw(6) << pct << "%  │ ~50%    │ "
                  << (pct>=40 && pct<=60 ? "PERFECT" : (pct>=35 && pct<=65 ? "OK" : "TUNE")) << "  │\n";
    }

    // ──── STRATEGY 7: F4B4 full + random swap ────
    {
        int wrong = 0;
        for (int t = 0; t < TRIALS; t++) {
            PE orig = (rand()%2) ? b1 : b0;
            PE obs = orig;
            for (int i = 0; i < 4; i++) obs = DM_mulY(cc, obs);
            if (rand()%2) obs = DM_swap(obs);
            for (int i = 0; i < 4; i++) obs = DM_mulY_inv(cc, obs);
            if (rand()%2) obs = DM_swap(obs);
            if ((DM_ratio(cc,kp,obs) > 0.5) != (DM_ratio(cc,kp,orig) > 0.5)) wrong++;
        }
        double pct = 100.0 * wrong / TRIALS;
        std::cout << "  │ F4B4 + swap midpoint & end     │ " << std::fixed << std::setprecision(1) << std::setw(6) << pct << "%  │ ~50%    │ "
                  << (pct>=40 && pct<=60 ? "PERFECT" : (pct>=35 && pct<=65 ? "OK" : "TUNE")) << "  │\n";
    }

    // ──── STRATEGY 8: Randomized F-ratio (asymmetric F/B) ────
    {
        int wrong = 0;
        for (int t = 0; t < TRIALS; t++) {
            PE orig = (rand()%2) ? b1 : b0;
            PE obs = orig;
            int fwd = 2 + rand()%4;  // 2-5 forward
            int bwd = 2 + rand()%4;  // 2-5 backward (asymmetric)
            for (int i = 0; i < fwd; i++) obs = DM_mulY(cc, obs);
            for (int i = 0; i < bwd; i++) obs = DM_mulY_inv(cc, obs);
            if ((DM_ratio(cc,kp,obs) > 0.5) != (DM_ratio(cc,kp,orig) > 0.5)) wrong++;
        }
        double pct = 100.0 * wrong / TRIALS;
        std::cout << "  │ Asymmetric F/B (2-5 each)      │ " << std::fixed << std::setprecision(1) << std::setw(6) << pct << "%  │ ~50%    │ "
                  << (pct>=40 && pct<=60 ? "PERFECT" : (pct>=35 && pct<=65 ? "OK" : "TUNE")) << "  │\n";
    }

    std::cout << "  └──────────────────────────────────────────────────────────────┘\n\n";

    return 0;
}
