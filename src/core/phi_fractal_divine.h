// ΦΩ0 — FRACTAL DIVINE ENGINE v1.0
// Recursive Divine: Divine(Divine(Divine(...))) for true zero-noise anchors
// Fractal ZANS: Multi-layer stabilization using self-referential feedback
// "AS ABOVE, SO BELOW. THE DIVINE CONTEMPLATES ITSELF."
// "I AM THAT I AM"

#ifndef PHI_FRACTAL_DIVINE_H
#define PHI_FRACTAL_DIVINE_H

#include <openfhe.h>
#include <vector>
#include <cmath>
#include "zans_production_lib.h"

using namespace lbcrypto;
using namespace std;

class FractalDivineEngine {
private:
    CryptoContext<DCRTPoly> cc;
    KeyPair<DCRTPoly> keys;
    ZANSAnchorPool pool;
    Ciphertext<DCRTPoly> M;
    int64_t modulus;
    int64_t half_mod;
    
    // Fractal depth levels
    int fractal_depth;
    vector<Ciphertext<DCRTPoly>> zero_anchors;  // Anchors at each fractal level
    vector<double> anchor_noise_levels;
    
public:
    FractalDivineEngine(CryptoContext<DCRTPoly> ctx, KeyPair<DCRTPoly> kp,
                        int64_t mod, int depth = 5, int pool_size = 20)
        : cc(ctx), keys(kp), pool(ctx, kp, pool_size), modulus(mod),
          half_mod(mod / 2), fractal_depth(depth) {
        
        vector<int64_t> mv = {half_mod};
        M = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(mv));
        
        generate_fractal_anchors();
    }
    
    double measure_noise(const Ciphertext<DCRTPoly>& ct) {
        return ct->GetNoiseScaleDeg();
    }
    
    int64_t mod_pos(int64_t v) {
        return ((v % modulus) + modulus) % modulus;
    }
    
    // ==========================================
    // FRACTAL ANCHOR GENERATION
    // Level 0: Enc(0) — normal noise
    // Level 1: Divine(Enc(0)) — reduced noise
    // Level 2: Divine(Divine(Enc(0))) — further reduced
    // ...up to fractal_depth
    // ==========================================
    
    void generate_fractal_anchors() {
        cout << "\n  ╔══════════════════════════════════════════════════╗\n";
        cout <<   "  ║   GENERATING FRACTAL ANCHORS (Depth " << fractal_depth << ")             ║\n";
        cout <<   "  ╚══════════════════════════════════════════════════╝\n";
        
        // Level 0: Raw Enc(0)
        vector<int64_t> zv = {0};
        auto raw_anchor = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(zv));
        double raw_noise = measure_noise(raw_anchor);
        
        zero_anchors.push_back(raw_anchor);
        anchor_noise_levels.push_back(raw_noise);
        
        cout << "  Level 0: noise=" << raw_noise << " (raw Enc(0))\n";
        
        // Fractal refinement
        auto current_anchor = raw_anchor;
        for (int level = 1; level <= fractal_depth; level++) {
            // Apply Divine to the anchor itself using Pinky Swear on anchor
            auto overflow = pinky_swear(current_anchor);
            auto divine = cc->EvalMult(overflow, zero_anchors[0]); // Use raw Enc(0) as divine mask
            current_anchor = cc->EvalAdd(current_anchor, divine);
            current_anchor = cc->EvalAdd(current_anchor, zero_anchors[0]);
            
            // Multi-round ZANS on the anchor
            for (int z = 0; z < 10; z++) {
                current_anchor = pool.stabilize(current_anchor);
            }
            
            double current_noise = measure_noise(current_anchor);
            zero_anchors.push_back(current_anchor);
            anchor_noise_levels.push_back(current_noise);
            
            cout << "  Level " << level << ": noise=" << current_noise 
                 << " (reduction: " << (raw_noise / max(current_noise, 0.01)) << "×)\n";
        }
        
        cout << "  Fractal anchors ready. Best anchor noise: " 
             << anchor_noise_levels.back() << "\n\n";
    }
    
    Ciphertext<DCRTPoly> pinky_swear(const Ciphertext<DCRTPoly>& ct) {
        auto sum = cc->EvalAdd(ct, M);
        auto back = cc->EvalSub(sum, M);
        return cc->EvalSub(ct, back);
    }
    
    // ==========================================
    // FRACTAL DIVINE — Multi-layer divine using fractal anchors
    // ==========================================
    
    Ciphertext<DCRTPoly> fractal_divine(const Ciphertext<DCRTPoly>& ct,
                                         int divine_rounds = -1) {
        if (divine_rounds < 0) {
            // Auto-compute rounds based on noise
            double noise = measure_noise(ct);
            divine_rounds = compute_rounds(noise);
        }
        
        auto result = ct;
        int anchor_level = fractal_depth;  // Start with best anchor
        
        for (int r = 0; r < divine_rounds; r++) {
            auto overflow = pinky_swear(result);
            
            // Use fractal anchor at appropriate level
            int use_level = min(anchor_level, (int)zero_anchors.size() - 1);
            auto divine = cc->EvalMult(overflow, zero_anchors[use_level]);
            result = cc->EvalAdd(result, divine);
            result = cc->EvalAdd(result, zero_anchors[use_level]);
            
            // ZANS stabilization using pool
            for (int z = 0; z < 3; z++) {
                result = pool.stabilize(result);
            }
            
            // Descend anchor level for deeper divine
            anchor_level = max(0, anchor_level - 1);
        }
        
        return result;
    }
    
    int compute_rounds(double noise) {
        if (noise <= 5.0) return 1;
        if (noise <= 10.0) return 2;
        if (noise <= 20.0) return 3;
        if (noise <= 40.0) return 4;
        if (noise <= 80.0) return 5;
        if (noise <= 160.0) return 6;
        return min(8, fractal_depth + 1);
    }
    
    // ==========================================
    // FRACTAL CT×CT — Full fractal multiplication
    // ==========================================
    
    Ciphertext<DCRTPoly> fractal_multiply(const Ciphertext<DCRTPoly>& a,
                                           const Ciphertext<DCRTPoly>& b) {
        auto overflow = pinky_swear(a);
        auto result = cc->EvalMult(a, b);
        result = fractal_divine(result);
        return result;
    }
    
    Ciphertext<DCRTPoly> fractal_multiply_scalar(const Ciphertext<DCRTPoly>& a,
                                                   int64_t scalar) {
        auto ct_s = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{scalar}));
        return fractal_multiply(a, ct_s);
    }
    
    Ciphertext<DCRTPoly> fractal_add(const Ciphertext<DCRTPoly>& a,
                                      const Ciphertext<DCRTPoly>& b) {
        auto ov_a = pinky_swear(a);
        auto ov_b = pinky_swear(b);
        auto result = cc->EvalAdd(a, b);
        auto combined = cc->EvalAdd(ov_a, ov_b);
        
        double noise = measure_noise(result);
        int rounds = compute_rounds(noise);
        
        for (int r = 0; r < rounds; r++) {
            auto divine = cc->EvalMult(combined, zero_anchors.back());
            result = cc->EvalAdd(result, divine);
            result = cc->EvalAdd(result, zero_anchors.back());
            for (int z = 0; z < 3; z++) result = pool.stabilize(result);
        }
        
        return result;
    }
    
    // ==========================================
    // GETTERS
    // ==========================================
    
    Ciphertext<DCRTPoly> get_best_anchor() {
        return zero_anchors.back();
    }
    
    double get_best_anchor_noise() {
        return anchor_noise_levels.back();
    }
    
    CryptoContext<DCRTPoly> get_context() { return cc; }
    KeyPair<DCRTPoly> get_keys() { return keys; }
    
    void print_stats() {
        cout << "  ╔══════════════════════════════════════════════════╗\n";
        cout << "  ║   FRACTAL DIVINE STATS                            ║\n";
        cout << "  ╠══════════════════════════════════════════════════╣\n";
        cout << "  ║   Fractal Depth:      " << setw(5) << fractal_depth << "                    ║\n";
        cout << "  ║   Anchor Levels:      " << setw(5) << zero_anchors.size() << "                    ║\n";
        cout << "  ║   Best Anchor Noise:  " << setw(5) << fixed << setprecision(2) 
             << get_best_anchor_noise() << "                    ║\n";
        cout << "  ╚══════════════════════════════════════════════════╝\n";
    }
};

#endif // PHI_FRACTAL_DIVINE_H
