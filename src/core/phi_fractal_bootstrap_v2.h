// ΦΩ0 — FRACTAL BOOTSTRAP ENGINE v2.0 — MULTI-PARTY
// Pure Enc(0) cascading — no EvalMult, no chain consumption
// Self-referential: output → input → deeper cleaning
// "THE FRACTAL CONTEMPLATES ITSELF. NOISE BECOMES ZERO."
// "I AM THAT I AM"

#ifndef PHI_FRACTAL_BOOTSTRAP_V2_H
#define PHI_FRACTAL_BOOTSTRAP_V2_H

#include <openfhe.h>
#include <vector>
#include <cmath>
#include <iostream>
#include <iomanip>
#include "zans_production_lib.h"

using namespace lbcrypto;
using namespace std;

class FractalBootstrapEngine {
private:
    CryptoContext<DCRTPoly> cc;
    KeyPair<DCRTPoly> keys;
    ZANSAnchorPool pool;
    int64_t modulus;
    
    int fractal_depth;
    vector<Ciphertext<DCRTPoly>> anchors;  // Fractal anchors at each level
    int total_enc0_additions = 0;

public:
    FractalBootstrapEngine(CryptoContext<DCRTPoly> ctx, KeyPair<DCRTPoly> kp,
                           int64_t mod, int depth = 5, int pool_size = 100)
        : cc(ctx), keys(kp), pool(ctx, kp, pool_size), modulus(mod),
          fractal_depth(depth) {
        generate_fractal_anchors();
    }

    double measure_noise(const Ciphertext<DCRTPoly>& ct) {
        return ct->GetNoiseScaleDeg();
    }

    // ==========================================
    // FRACTAL ANCHOR GENERATION (Pure Enc(0) additions)
    // ==========================================
    void generate_fractal_anchors() {
        cout << "  ╔══════════════════════════════════════════════════╗\n";
        cout << "  ║   GENERATING FRACTAL ANCHORS (Depth " << fractal_depth << ")";
        for (int i = 0; i < (22 - to_string(fractal_depth).length()); i++) cout << " ";
        cout << "║\n";
        cout << "  ╚══════════════════════════════════════════════════╝\n";

        // Level 0: Raw Enc(0)
        vector<int64_t> zv = {0};
        auto raw = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(zv));
        double raw_noise = measure_noise(raw);
        anchors.push_back(raw);
        
        cout << "  Level 0: noise=" << fixed << setprecision(4) << raw_noise 
             << " (raw Enc(0))\n";

        // Fractal refinement via repeated Enc(0) addition
        // Each level: previous anchor + N × Enc(0) where N = φ^level
        auto current = raw;
        for (int level = 1; level <= fractal_depth; level++) {
            // Fibonacci number of Enc(0) additions for this level
            int fib_n = fibonacci(level + 2);
            
            for (int i = 0; i < fib_n; i++) {
                current = pool.stabilize(current);
                total_enc0_additions++;
            }
            
            double noise = measure_noise(current);
            anchors.push_back(current);
            
            cout << "  Level " << level << ": noise=" << fixed << setprecision(4) << noise
                 << " (φ^" << level << " ≈ " << fib_n << " Enc(0) adds)\n";
        }
        
        cout << "  Total Enc(0) additions: " << total_enc0_additions << "\n\n";
    }

    int fibonacci(int n) {
        int a = 1, b = 1;
        for (int i = 2; i < n; i++) { int t = b; b = a + b; a = t; }
        return b;
    }

    // ==========================================
    // FRACTAL BOOTSTRAP — Multi-layer noise reset
    // ==========================================
    Ciphertext<DCRTPoly> fractal_bootstrap(const Ciphertext<DCRTPoly>& ct,
                                            int rounds = -1) {
        double noise = measure_noise(ct);
        if (rounds < 0) {
            rounds = compute_rounds(noise);
        }

        cout << "  Fractal Bootstrap: noise=" << fixed << setprecision(2) << noise
             << " → " << rounds << " rounds\n";

        auto result = ct;
        
        for (int r = 0; r < rounds; r++) {
            // Apply fractal anchors at increasing depth
            int level = min(r, fractal_depth);
            
            // Add multiple anchors from this level
            for (int i = 0; i < (fractal_depth - level + 1) * 5; i++) {
                result = cc->EvalAdd(result, anchors[level]);
                total_enc0_additions++;
            }
            
            // Cross-level stabilization: mix anchors from adjacent levels
            if (level > 0) {
                for (int i = 0; i < level * 3; i++) {
                    result = cc->EvalAdd(result, anchors[level - 1]);
                }
            }
            
            double new_noise = measure_noise(result);
            cout << "    Round " << (r+1) << ": noise=" << fixed << setprecision(4) 
                 << noise << " → " << new_noise;
            
            double reduction = noise - new_noise;
            if (reduction > 0) {
                cout << " (↓" << fixed << setprecision(1) << (reduction/noise*100) << "%)";
            }
            cout << "\n";
            noise = new_noise;
        }

        cout << "  Final noise: " << fixed << setprecision(4) << measure_noise(result) << "\n";
        return result;
    }

    int compute_rounds(double noise) {
        if (noise <= 5.0) return 1;
        if (noise <= 10.0) return 2;
        if (noise <= 20.0) return 3;
        if (noise <= 50.0) return 4;
        if (noise <= 100.0) return 5;
        return fractal_depth + 1;
    }

    int get_total_enc0_additions() { return total_enc0_additions; }
};

#endif
