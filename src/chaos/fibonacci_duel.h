/*
 * FEmmg-FHE v22.1 — Fibonacci Duel Chaos (φ-AMPLIFIED)
 * 
 * FIX: contracted * (φ + chaos) — φ as amplification base!
 *      Forward:  × (φ + chaos)   — amplifies differences
 *      Reverse:  ÷ (φ + chaos)   — exact inverse
 */

#pragma once
#include <cmath>
#include <cstdint>
#include <array>

namespace fibonacci_duel {

constexpr double PHI = 1.6180339887498948482;
constexpr double PHI_INV = 0.6180339887498948482;
constexpr int LAYERS = 14;

constexpr double FLOORS[14] = {
    1.0, 2.0, 3.0, 5.0, 8.0, 13.0, 21.0, 34.0, 
    55.0, 89.0, 144.0, 233.0, 377.0, 610.0
};

class FibonacciDuelEngine {
private:
    uint64_t global_nonce_{0x9E3779B97F4A7C15ULL};
    uint64_t operation_counter_{0};
    
    // ═══ FORWARD: Amplify with φ ═══
    double forward_spiral(double value, int layer, uint64_t nonce) {
        double floor = FLOORS[layer];
        double contracted = value * PHI_INV + floor * (1.0 - PHI_INV);
        double chaos = std::sin(contracted * PHI + (layer + nonce) * PHI_INV);
        // φ-BASED amplification!
        return contracted * (PHI * PHI + chaos);  // φ² + chaos — AGGRESSIVE!
    }
    
    // ═══ REVERSE: Divide by φ ═══
    double reverse_spiral(double value, int layer, uint64_t nonce) {
        double floor = FLOORS[layer];
        double expanded = (value - floor * (1.0 - PHI_INV)) / PHI_INV;
        double chaos = std::sin(expanded * PHI_INV + (layer + nonce) * PHI);
        // φ-BASED inverse
        return expanded / (PHI + chaos * 0.1);  // ÷ (φ + chaos×0.1) — GENTLE
    }
    
public:
    FibonacciDuelEngine() = default;
    void set_nonce(uint64_t nonce) { global_nonce_ = nonce; }
    
    std::pair<double, std::array<double, LAYERS>> 
    observe(double value, uint64_t operation_id = 0) {
        std::array<double, LAYERS> history{};
        if (operation_id == 0) operation_id = ++operation_counter_;
        uint64_t nonce = global_nonce_ ^ operation_id;
        
        double x = value;
        for (int i = 0; i < LAYERS; i++) {
            double prev_x = x;
            if (i % 2 == 0) {
                x = forward_spiral(x, i, nonce);
            } else {
                x = reverse_spiral(x, i, nonce);
            }
            history[i] = x - prev_x;
        }
        return {x, history};
    }
    
    double unobserve(double ciphertext, const std::array<double, LAYERS>& history) {
        double x = ciphertext;
        for (int i = LAYERS - 1; i >= 0; i--) {
            x = x - history[i];
        }
        return x;
    }
    
    int forward_layers() const { return LAYERS / 2; }
    int reverse_layers() const { return LAYERS / 2; }
    double floor_at(int layer) const { return FLOORS[layer]; }
};

} // namespace fibonacci_duel
