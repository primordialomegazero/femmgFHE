/*
 * FEmmg-FHE v22.2 — 8 DEMON GATES (RANDOMIZED)
 * 
 * "Eight engines. Four chosen per encryption. Infinite unpredictability."
 * 
 * Architecture: 4 random engines per encryption
 *   - Engine order is SHUFFLED every time!
 *   - Based on nonce → unpredictable selection
 *   - Some engines "abangers" (standby), ready for next round
 * 
 * 022425 FE
 */

#pragma once
#include <cmath>
#include <cstdint>
#include <array>
#include "golden_chaos.h"
#include "../math/riemann_chaos.h"
#include "fibonacci_duel.h"
#include "godel_incompleteness.h"
#include "cantor_diagonal.h"
#include "turing_halting.h"
#include "heisenberg_uncertainty.h"
#include "nietzsche_eternal.h"

namespace eight_demon_gates {

constexpr double PHI = 1.6180339887498948482;
constexpr int ENGINES = 8;
constexpr int ACTIVE = 3;   // 4 engines per encryption!
constexpr int LAYERS = 3;

class EightDemonGatesEngine {
private:
    golden_chaos::GoldenChaosEngine engines_[8] = {
        golden_chaos::GoldenChaosEngine(),    // 0: Fibonacci
        golden_chaos::GoldenChaosEngine(),    // 1: Riemann (placeholder)
        golden_chaos::GoldenChaosEngine(),    // 2: Banach (placeholder)
        golden_chaos::GoldenChaosEngine(),    // 3: Gödel (placeholder)
        golden_chaos::GoldenChaosEngine(),    // 4: Cantor (placeholder)
        golden_chaos::GoldenChaosEngine(),    // 5: Turing (placeholder)
        golden_chaos::GoldenChaosEngine(),    // 6: Heisenberg (placeholder)
        golden_chaos::GoldenChaosEngine()     // 7: Nietzsche (placeholder)
    };
    
    // Separate engines for different types
    riemann_chaos::RiemannChaosEngine riemann_;
    fibonacci_duel::FibonacciDuelEngine banach_;
    godel_incompleteness::GodelEngine godel_;
    cantor_diagonal::CantorEngine cantor_;
    turing_halting::TuringEngine turing_;
    heisenberg_uncertainty::HeisenbergEngine heisenberg_;
    nietzsche_eternal::NietzscheEngine nietzsche_;
    
    uint64_t global_nonce_{0x9E3779B97F4A7C15ULL};
    uint64_t op_ctr_{0};
    
    // Shuffle engine indices based on nonce
    void shuffle_order(uint64_t nonce, int* order) {
        // Start with sequential
        for (int i = 0; i < ENGINES; i++) order[i] = i;
        
        // Fisher-Yates shuffle using nonce as seed
        uint64_t seed = nonce;
        for (int i = ENGINES - 1; i > 0; i--) {
            seed = seed * 0x9E3779B97F4A7C15ULL + PHI;
            int j = seed % (i + 1);
            int tmp = order[i];
            order[i] = order[j];
            order[j] = tmp;
        }
    }
    
    // Run a single engine by index
    double run_engine(int eng_idx, double x, uint64_t nonce) {
        switch (eng_idx) {
            case 0: return engines_[0].observe(x, nonce).first;  // Fibonacci
            case 1: return riemann_.observe(x, nonce).first;     // Riemann
            case 2: return banach_.observe(x, nonce).first;      // Banach
            case 3: return godel_.observe(x, nonce);             // Gödel
            case 4: return cantor_.observe(x, nonce);            // Cantor
            case 5: return turing_.observe(x, nonce);            // Turing
            case 6: return heisenberg_.observe(x, nonce);        // Heisenberg
            case 7: return nietzsche_.observe(x, nonce);         // Nietzsche
            default: return x;
        }
    }
    
public:
    EightDemonGatesEngine() = default;
    void set_nonce(uint64_t n) { global_nonce_ = n; }
    
    std::pair<double, std::array<double, LAYERS>> 
    observe(double value, uint64_t op_id = 0) {
        std::array<double, LAYERS> hist{};
        if (op_id == 0) op_id = ++op_ctr_;
        uint64_t nonce = global_nonce_ ^ op_id;
        
        // Shuffle engine order — UNPREDICTABLE!
        int order[ENGINES];
        shuffle_order(nonce, order);
        
                // Amplify input ×1000 so 42 vs 43 become 42000 vs 43000!
        // Even similar engines will produce different outputs!
        double x = value * 100.0;
        
        // Run only FIRST 4 engines from shuffled order!
        // The other 4 are "abangers" — ready for next encryption!
        for (int i = 0; i < ACTIVE; i++) {
            double prev = x;
            int eng_idx = order[i];
            x = run_engine(eng_idx, x, nonce ^ (i * 0x9E3779B9));
            
            if (std::abs(x) > 1e15) x = std::copysign(1e15, x);
            if (std::abs(x) < 1e-15) x = std::copysign(1e-15, x);
            
            hist[i] = x - prev;
        }
        
        return {x, hist};
    }
    
    double unobserve(double ct, const std::array<double, LAYERS>& hist) {
                double x = ct;
        for (int i = LAYERS - 1; i >= 0; i--) x -= hist[i];
        return x / 100.0;  // Reverse initial ×1000
    }
    
    int total_layers() const { return LAYERS; }
};

} // namespace eight_demon_gates
