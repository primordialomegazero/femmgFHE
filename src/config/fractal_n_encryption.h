#pragma once
#include "../core/constants.h"
#include "../utils/safe_math.h"
#include "../crypto/golden_fibonacci.h"
#include <vector>

// ═══════════════════════════════════════════════════════════════
// FRACTAL N-ENCRYPTION
// ═══════════════════════════════════════════════════════════════

struct FractalNEncryption {
    int N_layers;
    std::vector<GoldenFibonacci> gf_layers;
    std::vector<double> layer_seeds;
    
    void init(double master_seed, int n_layers = 23) {
        N_layers = n_layers;
        layer_seeds.resize(N_layers);
        gf_layers.resize(N_layers);
        
        for (int i = 0; i < N_layers; i++) {
            layer_seeds[i] = SafeMath::fmod_safe(master_seed * std::pow(PHI, i + 1));
            gf_layers[i].init(layer_seeds[i], 50);
        }
    }
    
    std::pair<double, double> encrypt(double plaintext) {
        double current = plaintext;
        double y2_accum = 0;
        
        for (int i = 0; i < N_layers; i++) {
            auto [y1, y2] = gf_layers[i].encrypt(current);
            current = y1;
            y2_accum = SafeMath::fmod_safe(y2_accum + y2);
        }
        
        return {current, y2_accum};
    }
    
    double decrypt(double y1, double y2) {
        double current = y1;
        
        for (int i = N_layers - 1; i >= 0; i--) {
            double y2_avg = SafeMath::fmod_safe(y2 / N_layers);
            current = gf_layers[i].decrypt(current, y2_avg);
        }
        
        return current;
    }
};
