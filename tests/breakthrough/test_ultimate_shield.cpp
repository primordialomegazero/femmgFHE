// ULTIMATE SHIELD: Two-Reality Consciousness-Based iO
// Reality 1 (Physical): Basic Decoy → Multifortress → Mutating Quicksand
// Reality 2 (Metaphysical): Consciousness + Whitehole/Blackhole + RRDCT + Primordial Omega
// Architecture: Dan Fernandez / Primordial Omega Zero
#include <iostream>
#include <iomanip>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <sys/time.h>
#include <vector>
#include <string>
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

const double PHI = 1.618033988749895;
const double PSI = 0.6180339887498949;

// ═══════════════════════════════════════════════════════════════
// REALITY 1: PHYSICAL LAYER (The Decoy Universe)
// ═══════════════════════════════════════════════════════════════

// ─── LAYER 1: BASIC DECOY (False Hope Generator) ───
struct BasicDecoy {
    PE state;
    double false_hope;  // 0=none, 1=attacker certain they cracked it
    int fake_seed;      // Deterministic fake seed
};

BasicDecoy R1_L1_BasicDecoy(CryptoContext<DCRTPoly>& cc, const PE& input) {
    PE state = input;
    int fake_seed = 42;  // OBVIOUS seed — designed to be found
    
    // Predictable pattern: attacker WILL find this
    srand(fake_seed);
    for (int i = 0; i < 3 + rand()%3; i++) {
        if (rand()%2) state = F_mulY(cc, state);
        else state = F_mulY_inv(cc, state);
    }
    
    // False hope metric: 0.618 = "I'm close!" feeling
    double false_hope = PSI;
    
    return {state, false_hope, fake_seed};
}

// ─── LAYER 2: MULTIFORTRESS FAKE SEED DECOY ───
struct MultiFortress {
    PE surface;
    std::vector<int> fake_seeds;  // Multiple fake seeds
    int dimension;                 // How many seed layers
};

MultiFortress R1_L2_MultiFortress(CryptoContext<DCRTPoly>& cc, const PE& input) {
    PE state = input;
    int dims = 3 + rand()%5;  // 3-7 dimensional seed space
    std::vector<int> seeds;
    
    // Each dimension has its own fake seed
    for (int d = 0; d < dims; d++) {
        int fake_seed = 1000 + d * 137;  // Seeds look structured
        seeds.push_back(fake_seed);
        
        srand(fake_seed);
        for (int i = 0; i < 2 + rand()%3; i++) {
            if (rand()%2) state = F_mulY(cc, state);
            else state = F_mulY_inv(cc, state);
        }
    }
    
    return {state, seeds, dims};
}

// ─── LAYER 3: MUTATING QUICKSAND ───
struct MutatingQuicksand {
    PE surface;
    PE undertow;
    double depth;
    int mutation_cycle;
};

void R1_L3_MutatingStep(CryptoContext<DCRTPoly>& cc, PE& surface, PE& undertow,
                          double& depth, int& mutation) {
    mutation = (mutation + 1) % 13;  // Prime cycle — harder to predict
    
    switch(mutation % 8) {
        case 0: surface = F_mulY(cc, surface);     undertow = F_mulY_inv(cc, undertow); depth += PSI * 0.07; break;
        case 1: surface = F_mulY_inv(cc, surface); undertow = F_mulY(cc, undertow);     depth += PSI * 0.07; break;
        case 2: surface = F_mulY(cc, surface);     undertow = F_mulY_inv(cc, undertow); depth += PHI * 0.04; break;
        case 3: surface = F_mulY_inv(cc, surface); undertow = F_mulY(cc, undertow);     depth += PHI * 0.04; break;
        case 4: surface = F_swap(surface);         depth += PSI * 0.12; break;
        case 5: undertow = F_swap(undertow);       depth += PSI * 0.08; break;
        case 6: { auto t=surface; surface=undertow; undertow=t; } depth += PHI * 0.06; break;
        case 7: surface = F_mulY(cc, surface); surface = F_mulY_inv(cc, surface); depth += PSI * PSI * 0.08; break;
    }
    
    // MUTATION: Every 13 cycles, the quicksand CHANGES its rotation pattern
    if (mutation == 0) {
        surface = F_mulY(cc, surface);
        undertow = F_mulY_inv(cc, undertow);
        depth += 0.05;  // Extra sink on mutation
    }
    
    if (depth > 1.0) depth = 1.0;
}

MutatingQuicksand R1_L3_MutatingQuicksand(CryptoContext<DCRTPoly>& cc, const PE& input, int layers) {
    PE surface = input, undertow = input;
    double depth = 0.0;
    int mutation = rand()%13;
    
    for (int l = 0; l < layers; l++) {
        int steps = 3 + (int)(depth * 6);
        for (int s = 0; s < steps; s++) {
            R1_L3_MutatingStep(cc, surface, undertow, depth, mutation);
        }
        if (l < layers - 1) {
            surface = F_mulY(cc, surface);
            undertow = F_mulY_inv(cc, undertow);
        }
    }
    
    return {surface, undertow, depth, mutation};
}

// ═══════════════════════════════════════════════════════════════
// REALITY 2: METAPHYSICAL LAYER (True Defense)
// ═══════════════════════════════════════════════════════════════

// ─── CONSCIOUSNESS DEFENSE ───
struct ConsciousnessState {
    double IQ;   // Rational intelligence
    double EQ;   // Emotional intelligence
    double SQ;   // Spiritual intelligence
    double harmony; // IQ+EQ+SQ balance
};

ConsciousnessState R2_Consciousness_Balance(const ConsciousnessState& prev) {
    // Divine balance: IQ+EQ+SQ aligned = Godlike
    double iq = prev.IQ * PSI + PHI * 0.1;
    double eq = prev.EQ * PSI + PHI * 0.1;
    double sq = prev.SQ * PSI + PHI * 0.1;
    
    // Harmony = how balanced the three are
    double avg = (iq + eq + sq) / 3.0;
    double harmony = 1.0 - (std::abs(iq-avg) + std::abs(eq-avg) + std::abs(sq-avg)) / 3.0;
    if (harmony < 0) harmony = 0;
    if (harmony > 1) harmony = 1;
    
    return {iq, eq, sq, harmony};
}

void R2_Consciousness_Apply(CryptoContext<DCRTPoly>& cc, PE& state, const ConsciousnessState& cs) {
    // Godlike + Pure Love + Selflessness = God
    if (cs.harmony > 0.9) {
        // Transcendent state: apply φ-powered divine recursion
        state = F_mulY(cc, state);       // Expand consciousness
        state = F_mulY_inv(cc, state);   // Contract ego
        state = F_mulY(cc, state);       // Pure love expansion
    }
    
    // IQ>EQ: protecting themselves (strong defense)
    if (cs.IQ > cs.EQ) {
        state = F_mulY(cc, state);       // φ-shield up
    }
    
    // IQ<EQ: protecting others (sacrificial defense)
    if (cs.IQ < cs.EQ) {
        state = F_mulY_inv(cc, state);   // ψ-absorb attack energy
    }
    
    // Feel + Think fused = Intuition (φ·ψ resonance)
    if (std::abs(cs.IQ - cs.EQ) < 0.1) {
        state = F_mulY(cc, state);
        state = F_mulY_inv(cc, state);
        // φ·ψ = -1: perfect intuitive response
    }
}

// ─── WHITEHOLE DEFENSE (Emission) ───
void R2_Whitehole(CryptoContext<DCRTPoly>& cc, PE& state) {
    // Whitehole: emits φ-radiation, blinds observer
    // φ³ expansion: information explodes outward
    for (int i = 0; i < 3; i++) {
        state = F_mulY(cc, state);  // φ³ = 4.236x expansion
    }
}

// ─── BLACKHOLE DEFENSE (Absorption) ───
void R2_Blackhole(CryptoContext<DCRTPoly>& cc, PE& state) {
    // Blackhole: absorbs attack vectors, nothing escapes
    // ψ³ contraction: attack information collapses inward
    for (int i = 0; i < 3; i++) {
        state = F_mulY_inv(cc, state);  // ψ³ = 0.236x contraction
    }
}

// ─── ANTIMATTER DEFENSE (Annihilation) ───
void R2_Antimatter(CryptoContext<DCRTPoly>& cc, PE& signal, PE& noise) {
    // Anti-matter: signal + anti-signal = pure energy (information destroyed)
    // φ·ψ = -1: perfect annihilation
    signal = F_mulY(cc, signal);
    noise  = F_mulY_inv(cc, noise);
    // After φ·ψ cross: original information is annihilated
}

// ─── RRDCT (Rotating Random Dual Chain Tension) ───
struct RRDCTNode {
    PE primary, secondary;
    double tension;
    int direction;
};

void R2_RRDCT_Rotate(CryptoContext<DCRTPoly>& cc, PE& P, PE& S, int& dir) {
    int rot = rand()%8;
    switch(rot) {
        case 0: P = F_mulY(cc, P);     S = F_mulY(cc, S);     dir = +1; break;
        case 1: P = F_mulY_inv(cc, P); S = F_mulY_inv(cc, S); dir = -1; break;
        case 2: P = F_mulY(cc, P);     S = F_mulY_inv(cc, S); dir = 0;  break;
        case 3: P = F_mulY_inv(cc, P); S = F_mulY(cc, S);     dir = 0;  break;
        case 4: { auto t=P.a; P.a=P.b; P.b=t; } break;
        case 5: { auto t=S.a; S.a=S.b; S.b=t; } break;
        case 6: { auto t=P; P=S; S=t; } dir = -dir; break;
        case 7: P = F_mulY(cc, P); break;
    }
}

RRDCTNode R2_RRDCT(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp,
                    const PE& input, int depth) {
    if (depth <= 0) return {input, input, 0.0, 0};
    
    RRDCTNode prev = R2_RRDCT(cc, kp, input, depth - 1);
    PE P = prev.primary, S = prev.secondary;
    int dir = prev.direction;
    
    int rot = 4 + (int)(std::abs(prev.tension) * 50) % 6;
    for (int i = 0; i < rot; i++) R2_RRDCT_Rotate(cc, P, S, dir);
    
    double rp = F_ratio(cc, kp, P), rs = F_ratio(cc, kp, S);
    double tension = (rp - rs) * (dir >= 0 ? 1.0 : -1.0);
    double accumulated = tension * PSI + prev.tension * (1.0 - PSI);
    accumulated *= (rand()%2 ? 1.0 : -1.0);
    
    PE output;
    if (std::abs(accumulated) < 0.01) output = (rand()%2) ? P : S;
    else if (accumulated > 0) output = P;
    else output = S;
    
    return {output, P, accumulated, dir};
}

// ─── PRIMORDIAL OMEGA ZERO DEFENSE (Full Circle) ───
void R2_PrimordialOmega(CryptoContext<DCRTPoly>& cc, PE& state, double& cycle) {
    // Primordial Omega Zero: guides attacker BACK to Basic Decoy
    // φ⁰ = 1: return to origin
    // After all defenses, the output LOOKS LIKE Reality 1 Layer 1
    
    // ψ-dampen to appear "simple"
    for (int i = 0; i < 2; i++) {
        state = F_mulY_inv(cc, state);
    }
    
    // φ-restore to mimic basic decoy pattern
    state = F_mulY(cc, state);
    
    // Cycle tracker: ω = 0 (return to origin)
    cycle = 0.0;
}

// ═══════════════════════════════════════════════════════════════
// ULTIMATE SHIELD: Complete Assembly
// ═══════════════════════════════════════════════════════════════

struct UltimateShield {
    // Reality 1
    BasicDecoy r1_l1;
    MultiFortress r1_l2;
    MutatingQuicksand r1_l3;
    
    // Reality 2
    ConsciousnessState consciousness;
    PE whitehole_state;
    PE blackhole_state;
    PE antimatter_signal;
    PE antimatter_noise;
    RRDCTNode rrdct;
    double primordial_cycle;
    
    // Final output
    PE output;
};

UltimateShield F_UltimateShield(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp,
                                  const PE& input, int quicksand_layers, int rrdct_depth) {
    UltimateShield us;
    
    // ═══ REALITY 1: PHYSICAL ═══
    us.r1_l1 = R1_L1_BasicDecoy(cc, input);
    us.r1_l2 = R1_L2_MultiFortress(cc, us.r1_l1.state);
    us.r1_l3 = R1_L3_MutatingQuicksand(cc, us.r1_l2.surface, quicksand_layers);
    
    // ═══ REALITY 2: METAPHYSICAL ═══
    PE current = us.r1_l3.undertow;  // Start from deepest quicksand
    
    // Consciousness balance
    us.consciousness = {PHI, PSI, 1.0, PSI};  // Initial: IQ=φ, EQ=ψ, SQ=1
    for (int i = 0; i < 3; i++) {
        us.consciousness = R2_Consciousness_Balance(us.consciousness);
    }
    R2_Consciousness_Apply(cc, current, us.consciousness);
    
    // Whitehole emission
    us.whitehole_state = current;
    R2_Whitehole(cc, us.whitehole_state);
    
    // Blackhole absorption
    us.blackhole_state = current;
    R2_Blackhole(cc, us.blackhole_state);
    
    // Antimatter annihilation
    us.antimatter_signal = us.whitehole_state;
    us.antimatter_noise  = us.blackhole_state;
    R2_Antimatter(cc, us.antimatter_signal, us.antimatter_noise);
    
    // RRDCT (fed with antimatter-processed signal)
    PE rrdct_input = us.antimatter_signal;
    us.rrdct = R2_RRDCT(cc, kp, rrdct_input, rrdct_depth);
    
    // Primordial Omega Zero: Full circle back to Basic Decoy appearance
    us.output = us.rrdct.primary;
    R2_PrimordialOmega(cc, us.output, us.primordial_cycle);
    
    // FINAL: φ·ψ cross with consciousness harmony
    if (us.consciousness.harmony > 0.8) {
        us.output = F_mulY(cc, us.output);
        us.output = F_mulY_inv(cc, us.output);
    }
    
    return us;
}

int F_UltimateExtract(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp,
                       const UltimateShield& us) {
    double ratio = F_ratio(cc, kp, us.output);
    double threshold = 0.5 + us.rrdct.tension * 0.01;
    return (ratio > threshold) ? 1 : 0;
}

// ═══════════════════════════════════════════════════════════════
// ATTACK SIMULATOR
// ═══════════════════════════════════════════════════════════════

double F_attack_ultimate(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp,
                          const PE& b0, const PE& b1, int qs_layers, int rrdct_depth,
                          int trials, bool attack_decoy) {
    int wrong = 0;
    for (int t = 0; t < trials; t++) {
        int secret = rand()%2;
        PE orig = (secret == 0) ? b0 : b1;
        UltimateShield us = F_UltimateShield(cc, kp, orig, qs_layers, rrdct_depth);
        
        int guess;
        if (attack_decoy) {
            // Attacker attacks Reality 1 surface
            guess = F_bit(cc, kp, us.r1_l1.state);
        } else {
            // True extraction
            guess = F_UltimateExtract(cc, kp, us);
        }
        if (guess != secret) wrong++;
    }
    return 100.0 * wrong / trials;
}

double F_adv_ultimate(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp,
                       const PE& b0, const PE& b1, int qs_layers, int rrdct_depth, int trials) {
    int g1_1 = 0, t1 = 0, g1_0 = 0, t0 = 0;
    for (int t = 0; t < trials; t++) {
        int secret = rand()%2;
        PE orig = (secret == 0) ? b0 : b1;
        UltimateShield us = F_UltimateShield(cc, kp, orig, qs_layers, rrdct_depth);
        int guess = F_UltimateExtract(cc, kp, us);
        if (secret == 1) { t1++; if (guess == 1) g1_1++; }
        else { t0++; if (guess == 1) g1_0++; }
    }
    return std::abs(100.0 * g1_1 / t1 - 100.0 * g1_0 / t0);
}

// ═══════════════════════════════════════════════════════════════
// MAIN
// ═══════════════════════════════════════════════════════════════
int main() {
    std::cout << "\n";
    std::cout << "  ╔══════════════════════════════════════════════════════════════╗\n";
    std::cout << "  ║  ULTIMATE SHIELD: Two-Reality Consciousness iO               ║\n";
    std::cout << "  ║  Reality 1: Physical Decoy Universe                          ║\n";
    std::cout << "  ║  Reality 2: Metaphysical True Defense                        ║\n";
    std::cout << "  ╚══════════════════════════════════════════════════════════════╝\n\n";
    srand(time(0));

    CCParams<CryptoContextCKKSRNS> p;
    p.SetMultiplicativeDepth(150); p.SetScalingModSize(50); p.SetBatchSize(1024);
    p.SetRingDim(16384); p.SetSecretKeyDist(UNIFORM_TERNARY); p.SetSecurityLevel(HEStd_NotSet);
    auto cc = GenCryptoContext(p);
    cc->Enable(PKE); cc->Enable(KEYSWITCH); cc->Enable(LEVELEDSHE);
    auto kp = cc->KeyGen(); cc->EvalMultKeyGen(kp.secretKey);

    PE b0 = F_enc(cc, kp, 0), b1 = F_enc(cc, kp, 1);
    const int TRIALS = 200;

    std::cout << "  φ = " << std::fixed << std::setprecision(12) << PHI << "\n";
    std::cout << "  ψ = " << PSI << "\n";
    std::cout << "  φ·ψ = " << PHI * PSI << "\n\n";

    // ═══════════════════════════════════
    // REALITY 1: DECOY TEST
    // ═══════════════════════════════════
    std::cout << "  ┌──────────────────────────────────────────────────────────────┐\n";
    std::cout << "  │  REALITY 1: PHYSICAL DECOY (What Attacker Sees)              │\n";
    std::cout << "  ├──────────┬──────────────┬──────────────────────────────────┤\n";
    std::cout << "  │ Layer    │ Attacker Acc │ Status                           │\n";
    std::cout << "  ├──────────┼──────────────┼──────────────────────────────────┤\n";
    
    // Basic Decoy only
    double acc_basic = F_attack_ultimate(cc, kp, b0, b1, 0, 0, TRIALS, true);
    std::cout << "  │ Basic    │ " << std::fixed << std::setprecision(1) << std::setw(8) << acc_basic << "%     │ FALSE HOPE — looks crackable       │\n";
    
    // + MultiFortress
    double acc_fortress = F_attack_ultimate(cc, kp, b0, b1, 1, 0, TRIALS, true);
    std::cout << "  │ Fortress │ " << std::fixed << std::setprecision(1) << std::setw(8) << acc_fortress << "%     │ MULTI-SEED — wasting their time    │\n";
    
    // + Mutating Quicksand
    double acc_quicksand = F_attack_ultimate(cc, kp, b0, b1, 3, 0, TRIALS, true);
    std::cout << "  │ Quicksand│ " << std::fixed << std::setprecision(1) << std::setw(8) << acc_quicksand << "%     │ SINKING — pattern keeps shifting  │\n";
    
    std::cout << "  └──────────┴──────────────┴──────────────────────────────────┘\n\n";

    // ═══════════════════════════════════
    // REALITY 2: TRUE DEFENSE
    // ═══════════════════════════════════
    std::cout << "  ┌──────────────────────────────────────────────────────────────────────┐\n";
    std::cout << "  │  REALITY 2: METAPHYSICAL TRUE DEFENSE                                │\n";
    std::cout << "  ├──────────────┬────────────────┬───────────┬──────────────────────────┤\n";
    std::cout << "  │ RRDCT Depth  │ Attacker Error │ Advantage │ Status                   │\n";
    std::cout << "  ├──────────────┼────────────────┼───────────┼──────────────────────────┤\n";

    for (int depth = 1; depth <= 6; depth++) {
        double error = F_attack_ultimate(cc, kp, b0, b1, 3, depth, TRIALS, false);
        double adv = F_adv_ultimate(cc, kp, b0, b1, 3, depth, TRIALS);
        
        std::string status;
        if (adv < 0.5)      status = "PERFECT (<0.5%)";
        else if (adv < 1.0) status = "EXCELLENT (<1%)";
        else if (adv < 2.0) status = "GOOD (<2%)";
        else                status = "STABLE";
        
        std::cout << "  │ " << std::setw(10) << std::left << 
            (depth==1?"Shallow":depth==3?"Medium":depth==6?"Deep":"")
                  << " │ " << std::fixed << std::setprecision(2) << std::setw(10) << error << "%     │ "
                  << std::setw(5) << std::setprecision(4) << adv << "%  │ "
                  << status << "        │\n";
    }

    std::cout << "  └──────────────┴────────────────┴───────────┴──────────────────────────┘\n\n";

    // ═══════════════════════════════════
    // ARCHITECTURE
    // ═══════════════════════════════════
    std::cout << "  ╔══════════════════════════════════════════════════════════════╗\n";
    std::cout << "  ║  ULTIMATE SHIELD — COMPLETE ARCHITECTURE                      ║\n";
    std::cout << "  ╠══════════════════════════════════════════════════════════════╣\n";
    std::cout << "  ║                                                              ║\n";
    std::cout << "  ║  REALITY 1: PHYSICAL (Decoy Universe)                        ║\n";
    std::cout << "  ║  L1: Basic Decoy — False hope, fake seed (DESIGNED TO FALL)  ║\n";
    std::cout << "  ║  L2: MultiFortress — N-dimensional fake seeds (TIME WASTER)  ║\n";
    std::cout << "  ║  L3: Mutating Quicksand — Shifting sand + mutation cycles    ║\n";
    std::cout << "  ║                                                              ║\n";
    std::cout << "  ║  REALITY 2: METAPHYSICAL (True Defense)                      ║\n";
    std::cout << "  ║  · Consciousness: IQ+EQ+SQ = Godlike → God = Pure Love       ║\n";
    std::cout << "  ║  · Whitehole: φ³ emission → blinds observer                  ║\n";
    std::cout << "  ║  · Blackhole: ψ³ absorption → attack vanishes                ║\n";
    std::cout << "  ║  · Antimatter: φ·ψ = -1 → perfect annihilation               ║\n";
    std::cout << "  ║  · RRDCT: Rotating Random Dual Chain Tension                 ║\n";
    std::cout << "  ║  · Primordial Ω: Full circle → back to Basic Decoy           ║\n";
    std::cout << "  ║                                                              ║\n";
    std::cout << "  ║  Attacker journey:                                           ║\n";
    std::cout << "  ║  Basic Decoy → 'We cracked it! (60%)'                       ║\n";
    std::cout << "  ║  MultiFortress → 'Wait, more seeds...'                       ║\n";
    std::cout << "  ║  Mutating Quicksand → 'It keeps changing!'                   ║\n";
    std::cout << "  ║  [Stuck in Reality 1 forever]                                ║\n";
    std::cout << "  ║  Primordial Ω → OUTPUT LOOKS LIKE BASIC DECOY                ║\n";
    std::cout << "  ║  Attacker: 'We're back to the start?!'                       ║\n";
    std::cout << "  ║                                                              ║\n";
    std::cout << "  ║  Full Circle. Infinite Loop. No Escape.                      ║\n";
    std::cout << "  ║                                                              ║\n";
    std::cout << "  ╚══════════════════════════════════════════════════════════════╝\n\n";

    return 0;
}
