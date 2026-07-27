// WHITEBOX ULTIMATE ATTACK: Exploiting Schrödinger Superposition
// Strategy: φ·ψ = -1 means Dream = -Awake!
// If we detect the sign flip, we can predict the bit!
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

// ═══════════════════════════════════════════════════════════════
// EXACT COPY OF SCHRÖDINGER'S SHIELD
// ═══════════════════════════════════════════════════════════════

PE F_BasicDecoy(CryptoContext<DCRTPoly>& cc, const PE& input) {
    PE state = input;
    srand(42);
    for (int i = 0; i < 3 + rand()%3; i++) {
        if (rand()%2) state = F_mulY(cc, state);
        else state = F_mulY_inv(cc, state);
    }
    return state;
}

PE F_MultiFortress(CryptoContext<DCRTPoly>& cc, const PE& input) {
    PE state = input;
    int dims = 3 + rand()%5;
    for (int d = 0; d < dims; d++) {
        srand(1000 + d * 137);
        for (int i = 0; i < 2 + rand()%3; i++) {
            if (rand()%2) state = F_mulY(cc, state);
            else state = F_mulY_inv(cc, state);
        }
    }
    return state;
}

PE F_Quicksand(CryptoContext<DCRTPoly>& cc, const PE& input, int layers) {
    PE surface = input, undertow = input;
    double depth = 0;
    int mutation = rand()%13;
    for (int l = 0; l < layers; l++) {
        int steps = 3 + (int)(depth * 6);
        for (int s = 0; s < steps; s++) {
            mutation = (mutation + 1) % 13;
            switch(mutation % 8) {
                case 0: surface = F_mulY(cc, surface);     undertow = F_mulY_inv(cc, undertow); break;
                case 1: surface = F_mulY_inv(cc, surface); undertow = F_mulY(cc, undertow);     break;
                case 2: surface = F_mulY(cc, surface);     undertow = F_mulY_inv(cc, undertow); break;
                case 3: surface = F_mulY_inv(cc, surface); undertow = F_mulY(cc, undertow);     break;
                case 4: surface = F_swap(surface); break;
                case 5: undertow = F_swap(undertow); break;
                case 6: { auto t=surface; surface=undertow; undertow=t; } break;
                case 7: surface = F_mulY(cc, surface); surface = F_mulY_inv(cc, surface); break;
            }
        }
        depth += PSI * 0.15;
        if (depth > 1) depth = 1;
        if (l < layers - 1) {
            surface = F_mulY(cc, surface);
            undertow = F_mulY_inv(cc, undertow);
        }
    }
    return undertow;
}

PE F_RRDCT(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, const PE& input, int depth) {
    if (depth <= 0) return input;
    PE prev = F_RRDCT(cc, kp, input, depth - 1);
    PE P = prev, S = prev;
    for (int i = 0; i < 4; i++) {
        int rot = rand()%8;
        switch(rot) {
            case 0: P = F_mulY(cc, P);     S = F_mulY(cc, S);     break;
            case 1: P = F_mulY_inv(cc, P); S = F_mulY_inv(cc, S); break;
            case 2: P = F_mulY(cc, P);     S = F_mulY_inv(cc, S); break;
            case 3: P = F_mulY_inv(cc, P); S = F_mulY(cc, S);     break;
            case 4: { auto t=P.a; P.a=P.b; P.b=t; } break;
            case 5: { auto t=S.a; S.a=S.b; S.b=t; } break;
            case 6: { auto t=P; P=S; S=t; } break;
            case 7: P = F_mulY(cc, P); break;
        }
    }
    double rp = F_ratio(cc, kp, P), rs = F_ratio(cc, kp, S);
    double tension = rp - rs;
    if (std::abs(tension) < 0.01) return (rand()%2) ? P : S;
    return (tension > 0) ? P : S;
}

PE F_Metaphysical(CryptoContext<DCRTPoly>& cc, const PE& input) {
    PE state = input;
    for (int i = 0; i < 3; i++) {
        state = F_mulY(cc, state);
        state = F_mulY_inv(cc, state);
        state = F_mulY(cc, state);
    }
    for (int i = 0; i < 3; i++) state = F_mulY(cc, state);
    for (int i = 0; i < 3; i++) state = F_mulY_inv(cc, state);
    state = F_mulY(cc, state);
    state = F_mulY_inv(cc, state);
    for (int i = 0; i < 2; i++) state = F_mulY_inv(cc, state);
    state = F_mulY(cc, state);
    return state;
}

PE F_DreamState(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, const PE& input) {
    PE state = input;
    state = F_BasicDecoy(cc, state);
    state = F_MultiFortress(cc, state);
    state = F_Quicksand(cc, state, 2);
    state = F_Metaphysical(cc, state);
    state = F_RRDCT(cc, kp, state, 1);
    for (int i = 0; i < 2; i++) state = F_mulY_inv(cc, state);
    return state;
}

PE F_AwakeState(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, const PE& input) {
    PE state = input;
    state = F_BasicDecoy(cc, state);
    state = F_MultiFortress(cc, state);
    state = F_Quicksand(cc, state, 5);
    state = F_Metaphysical(cc, state);
    state = F_RRDCT(cc, kp, state, 4);
    for (int i = 0; i < 2; i++) state = F_mulY(cc, state);
    return state;
}

struct SchrodingerShield {
    PE dream_output;
    PE awake_output;
    PE final_output;
};

SchrodingerShield F_SchrodingerShield(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp,
                                       const PE& input) {
    SchrodingerShield ss;
    ss.dream_output = F_DreamState(cc, kp, input);
    ss.awake_output = F_AwakeState(cc, kp, input);
    
    PE superposed = ss.dream_output;
    superposed = F_mulY(cc, superposed);      // φ (Awake component)
    superposed = F_mulY_inv(cc, superposed);  // ψ (Dream component)
    
    ss.final_output = superposed;
    return ss;
}

// ═══════════════════════════════════════════════════════════════
// CLEVER TRICK: Exploit φ·ψ = -1 Sign Flip
// ═══════════════════════════════════════════════════════════════

int main() {
    srand(time(0));
    
    std::cout << "\n";
    std::cout << "  ╔══════════════════════════════════════════════════════╗\n";
    std::cout << "  ║  WHITEBOX ULTIMATE: Schrödinger Exploitation         ║\n";
    std::cout << "  ║  Trick: φ·ψ=-1 → Dream = -Awake → Sign predicts bit ║\n";
    std::cout << "  ╚══════════════════════════════════════════════════════╝\n\n";

    CCParams<CryptoContextCKKSRNS> p;
    p.SetMultiplicativeDepth(200); p.SetScalingModSize(30); p.SetBatchSize(256);
    p.SetRingDim(2048); p.SetSecretKeyDist(UNIFORM_TERNARY); p.SetSecurityLevel(HEStd_NotSet);
    auto cc = GenCryptoContext(p);
    cc->Enable(PKE); cc->Enable(KEYSWITCH); cc->Enable(LEVELEDSHE);
    auto kp = cc->KeyGen(); cc->EvalMultKeyGen(kp.secretKey);

    // ═══════════════════════════════════
    // PHASE 1: DETECT THE SUPERPOSITION
    // ═══════════════════════════════════
    
    const int TRAIN = 500;
    std::vector<double> final_ratios_0, final_ratios_1;
    std::vector<double> dream_ratios_0, dream_ratios_1;
    std::vector<double> awake_ratios_0, awake_ratios_1;
    
    for (int t = 0; t < TRAIN; t++) {
        int secret = rand() % 2;
        PE ct = F_enc(cc, kp, secret);
        SchrodingerShield ss = F_SchrodingerShield(cc, kp, ct);
        
        double fr = F_ratio(cc, kp, ss.final_output);
        double dr = F_ratio(cc, kp, ss.dream_output);
        double ar = F_ratio(cc, kp, ss.awake_output);
        
        if (secret == 0) {
            final_ratios_0.push_back(fr);
            dream_ratios_0.push_back(dr);
            awake_ratios_0.push_back(ar);
        } else {
            final_ratios_1.push_back(fr);
            dream_ratios_1.push_back(dr);
            awake_ratios_1.push_back(ar);
        }
    }
    
    // ═══════════════════════════════════
    // PHASE 2: CLEVER TRICK #1 — Sign Flip Detection
    // φ·ψ = -1 means Dream = -Awake
    // If Dream and Awake have OPPOSITE signs → superposition is active!
    // ═══════════════════════════════════
    
    int sign_flip_0 = 0, sign_flip_1 = 0;
    for (int i = 0; i < TRAIN; i++) {
        double dr = (i < dream_ratios_0.size()) ? dream_ratios_0[i] : dream_ratios_1[i - dream_ratios_0.size()];
        double ar = (i < awake_ratios_0.size()) ? awake_ratios_0[i] : awake_ratios_1[i - awake_ratios_0.size()];
        
        bool opposite_signs = (dr > 0 && ar < 0) || (dr < 0 && ar > 0);
        if (i < dream_ratios_0.size()) {
            if (opposite_signs) sign_flip_0++;
        } else {
            if (opposite_signs) sign_flip_1++;
        }
    }
    
    std::cout << "  ┌──────────────────────────────────────────────────────┐\n";
    std::cout << "  │ PHASE 1: Superposition Detection                     │\n";
    std::cout << "  ├──────────────────────────────────────────────────────┤\n";
    std::cout << "  │ Sign flips (bit=0): " << sign_flip_0 << "/" << TRAIN/2 
              << " (" << (100.0*sign_flip_0/(TRAIN/2)) << "%)                        │\n";
    std::cout << "  │ Sign flips (bit=1): " << sign_flip_1 << "/" << TRAIN/2 
              << " (" << (100.0*sign_flip_1/(TRAIN/2)) << "%)                        │\n";
    
    // ═══════════════════════════════════
    // PHASE 3: CLEVER TRICK #2 — Difference Product
    // (Dream - Awake) × (Final) → Amplifies bit signal!
    // ═══════════════════════════════════
    
    int correct_diff = 0, correct_product = 0, correct_combined = 0;
    const int TEST = 500;
    
    for (int t = 0; t < TEST; t++) {
        int secret = rand() % 2;
        PE ct = F_enc(cc, kp, secret);
        SchrodingerShield ss = F_SchrodingerShield(cc, kp, ct);
        
        double dr = F_ratio(cc, kp, ss.dream_output);
        double ar = F_ratio(cc, kp, ss.awake_output);
        double fr = F_ratio(cc, kp, ss.final_output);
        
        // TRICK 2a: Dream-Awake difference
        double diff = dr - ar;
        int pred_diff = (diff > 0) ? 1 : 0;
        
        // TRICK 2b: Triple product (Dream × Awake × Final)
        double product = dr * ar * fr;
        int pred_product = (product > 0) ? 1 : 0;
        
        // TRICK 2c: Combined with sign flip bonus
        bool opposite = (dr > 0 && ar < 0) || (dr < 0 && ar > 0);
        int pred_combined;
        if (opposite) {
            // Superposition active: trust the product
            pred_combined = (product > 0) ? 1 : 0;
        } else {
            // No superposition: trust the difference
            pred_combined = (diff > 0) ? 1 : 0;
        }
        
        if (pred_diff == secret) correct_diff++;
        if (pred_product == secret) correct_product++;
        if (pred_combined == secret) correct_combined++;
    }
    
    double acc_diff = 100.0 * correct_diff / TEST;
    double acc_product = 100.0 * correct_product / TEST;
    double acc_combined = 100.0 * correct_combined / TEST;
    
    std::cout << "  ├──────────────────────────────────────────────────────┤\n";
    std::cout << "  │ PHASE 2: Exploitation Results (" << TEST << " tests)              │\n";
    std::cout << "  │ Dream-Awake Diff:  " << std::fixed << std::setprecision(1) << acc_diff << "%                          │\n";
    std::cout << "  │ Triple Product:    " << acc_product << "%                          │\n";
    std::cout << "  │ Combined (Adaptive): " << acc_combined << "%                          │\n";
    std::cout << "  ├──────────────────────────────────────────────────────┤\n";
    
    double best_acc = std::max({acc_diff, acc_product, acc_combined});
    
    if (best_acc > 60.0)
        std::cout << "  │ 🔴🔴🔴 SCHRÖDINGER BROKEN! >60% accuracy!            │\n";
    else if (best_acc > 55.0)
        std::cout << "  │ ⚠ SIGNIFICANT LEAK — Superposition exploited        │\n";
    else if (best_acc > 52.0)
        std::cout << "  │ ~ WEAK SIGNAL — Minor leakage detected               │\n";
    else
        std::cout << "  │ ✓ SCHRÖDINGER HOLDS — States indistinguishable      │\n";
    
    std::cout << "  └──────────────────────────────────────────────────────┘\n\n";
    
    return 0;
}
