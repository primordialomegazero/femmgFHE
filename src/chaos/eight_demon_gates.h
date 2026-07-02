/*
 * FEmmg-FHE v22.2 — 8 DEMON GATES (DUAL MODE)
 * 
 * "Eight engines. Dual mode. Infinite chaos."
 * 
 * Standard Mode: 1 random engine per encryption (FAST)
 * Maximum Mode:  3 random engines per encryption (MAX CHAOS)
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

class EightDemonGatesEngine {
private:
    static constexpr int ACTIVE_STANDARD = 1;
    static constexpr int ACTIVE_MAX = 3;
    int active_ = ACTIVE_STANDARD;
    
    // All 8 engines
    golden_chaos::GoldenChaosEngine fib_;
    riemann_chaos::RiemannChaosEngine rie_;
    fibonacci_duel::FibonacciDuelEngine ban_;
    godel_incompleteness::GodelEngine god_;
    cantor_diagonal::CantorEngine can_;
    turing_halting::TuringEngine tur_;
    heisenberg_uncertainty::HeisenbergEngine hei_;
    nietzsche_eternal::NietzscheEngine nie_;
    
    uint64_t global_nonce_{0x9E3779B97F4A7C15ULL};
    uint64_t op_ctr_{0};
    int cached_order_[ENGINES];
    uint64_t last_nonce_ = 0;
    
    void shuffle_order(uint64_t nonce, int* order) {
        if (nonce == last_nonce_) {
            for (int i = 0; i < ENGINES; i++) order[i] = cached_order_[i];
            return;
        }
        for (int i = 0; i < ENGINES; i++) order[i] = i;
        uint64_t seed = nonce;
        for (int i = ENGINES - 1; i > 0; i--) {
            seed = seed * 0x9E3779B97F4A7C15ULL + PHI;
            int j = seed % (i + 1);
            int tmp = order[i]; order[i] = order[j]; order[j] = tmp;
        }
        for (int i = 0; i < ENGINES; i++) cached_order_[i] = order[i];
        last_nonce_ = nonce;
    }
    
    double run_engine(int idx, double x, uint64_t nonce) {
        switch (idx) {
            case 0: return fib_.observe(x, nonce).first;
            case 1: return rie_.observe(x, nonce).first;
            case 2: return ban_.observe(x, nonce).first;
            case 3: return god_.observe(x, nonce);
            case 4: return can_.observe(x, nonce);
            case 5: return tur_.observe(x, nonce);
            case 6: return hei_.observe(x, nonce);
            case 7: return nie_.observe(x, nonce);
            default: return x;
        }
    }
    
public:
    EightDemonGatesEngine() = default;
    void set_nonce(uint64_t n) { global_nonce_ = n; }
    void set_standard_mode() { active_ = ACTIVE_STANDARD; }
    void set_maximum_mode()  { active_ = ACTIVE_MAX; }
    int total_layers() const { return active_; }
    
    std::pair<double, std::array<double, 3>> 
    observe(double value, uint64_t op_id = 0) {
        std::array<double, 3> hist{};
        if (op_id == 0) op_id = ++op_ctr_;
        uint64_t nonce = global_nonce_ ^ op_id;
        
        int order[ENGINES];
        shuffle_order(nonce, order);
        
                // Amplification depends on mode!
        double amp = (active_ == ACTIVE_MAX) ? 10000.0 : 100.0;
        double x = value * amp;
        
        for (int i = 0; i < active_; i++) {
            double prev = x;
            x = run_engine(order[i], x, nonce ^ (i * 0x9E3779B9));
            if (std::abs(x) > 1e15) x = std::copysign(1e15, x);
            if (std::abs(x) < 1e-15) x = std::copysign(1e-15, x);
            hist[i] = x - prev;
        }
        
        return {x, hist};
    }
    
    double unobserve(double ct, const std::array<double, 3>& hist) {
        double x = ct;
        for (int i = active_ - 1; i >= 0; i--) x -= hist[i];
                // Reverse the mode-dependent amplification
        double amp = (active_ == ACTIVE_MAX) ? 10000.0 : 100.0;
        return x / amp;
    }
};

} // namespace eight_demon_gates
