/*
 * FEmmg-FHE v22.1 — TRIPLE RASHOMON (STABLE)
 * 
 * Optimized for speed WITH numerical stability.
 */

#pragma once
#include <cmath>
#include <cstdint>
#include <array>

namespace triple_rashomon {

constexpr double PHI = 1.6180339887498948482;
constexpr double PHI_INV = 0.6180339887498948482;
constexpr double PHI_SQ = 2.6180339887498948482;
constexpr int LAYERS = 21;

constexpr double ZEROS[7] = {14.1347, 21.0220, 25.0109, 30.4249, 32.9351, 37.5862, 40.9187};
constexpr double FLOORS[7] = {1, 2, 3, 5, 8, 13, 21};

class TripleRashomonEngine {
private:
    uint64_t global_nonce_{0x9E3779B97F4A7C15ULL};
    uint64_t op_ctr_{0};
    
    // Stable fast sin — clamped to prevent overflow
    inline double fast_sin(double x) const {
        x = std::fmod(x, 2.0 * M_PI);
        double x2 = x * x;
        double result = x * (1.0 - x2 * (1.0/6.0 - x2 * 1.0/120.0));
        // Clamp to [-1, 1]
        if (result > 1.0) result = 1.0;
        if (result < -1.0) result = -1.0;
        return result;
    }
    
public:
    TripleRashomonEngine() = default;
    void set_nonce(uint64_t n) { global_nonce_ = n; }
    
    std::pair<double, std::array<double, LAYERS>> 
    observe(double value, uint64_t op_id = 0) {
        std::array<double, LAYERS> hist{};
        if (op_id == 0) op_id = ++op_ctr_;
        uint64_t nonce = global_nonce_ ^ op_id;
        
        double x = value * PHI + std::log(std::abs(value) + 1.0);
        
        for (int pass = 0; pass < 3; pass++) {
            double amp = (pass == 0) ? 1.0 : (pass == 1) ? PHI : PHI_SQ;
            uint64_t pass_nonce = nonce ^ (pass * 0x9E3779B9);
            
            for (int i = 0; i < 7; i++) {
                int layer = pass * 7 + i;
                double prev = x;
                int eng = i % 3;
                
                double chaos = 0.0;
                
                if (eng == 0) {
                    // SINE
                    chaos = fast_sin(x * PHI + (layer + pass_nonce) * PHI_INV);
                } else if (eng == 1) {
                    // ZETA — simplified for speed
                    double t = ZEROS[i] + (pass_nonce % 1000) * 0.001;
                    // Use pre-computed zeta sign instead of full computation
                    double zeta_sign = (i % 2 == 0) ? 1.0 : -1.0;
                    chaos = zeta_sign * fast_sin(x * PHI + layer * PHI_INV + t * 0.01);
                } else {
                    // FIB
                    double fl = FLOORS[i];
                    double ct = x * PHI_INV + fl * (1.0 - PHI_INV);
                    chaos = fast_sin(ct * PHI + (layer + pass_nonce) * PHI_INV);
                    x = ct;  // Contract first
                }
                
                // Apply chaos — with safety clamp
                double factor = PHI_SQ + chaos * amp;
                if (factor < 0.1) factor = 0.1;  // Prevent negative/zero
                if (factor > 10.0) factor = 10.0; // Prevent overflow
                
                if (eng != 2) x = prev;  // Restore for non-FIB
                x = x * factor;
                if (eng == 2) x = x;  // FIB already contracted
                
                hist[layer] = x - prev;
            }
        }
        
        return {x, hist};
    }
    
    double unobserve(double ct, const std::array<double, LAYERS>& hist) {
        double x = ct;
        for (int i = LAYERS - 1; i >= 0; i--) x -= hist[i];
        double v = (x - std::log(std::abs(x) + 1.0)) / PHI;
        for (int iter = 0; iter < 5; iter++) {
            double f = v * PHI + std::log(std::abs(v) + 1.0) - x;
            double df = PHI + 1.0 / (std::abs(v) + 1.0);
            if (std::abs(df) < 1e-10) break;
            v -= f / df;
        }
        return v;
    }
    
    int total_layers() const { return LAYERS; }
};

} // namespace triple_rashomon
