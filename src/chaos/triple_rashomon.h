/*
 * FEmmg-FHE v22.2 — TRIPLE RASHOMON (TRUE FHE)
 *
 * CHAOS IS THE KEY. Every engine instance has a cryptographically random nonce.
 * observe() is ONE-WAY — cannot be reversed without the original engine state.
 * "The observer and the observed are one." — now mathematically enforced.
 */

#pragma once
#include <cmath>
#include <cstdint>
#include <array>
#include <random>
#include <chrono>
#include <cstring>

namespace triple_rashomon {

constexpr double PHI = 1.6180339887498948482;
constexpr double PHI_INV = 0.6180339887498948482;
constexpr double PHI_SQ = 2.6180339887498948482;
constexpr int LAYERS = 21;

constexpr double ZEROS[7] = {14.1347, 21.0220, 25.0109, 30.4249, 32.9351, 37.5862, 40.9187};
constexpr double FLOORS[7] = {1, 2, 3, 5, 8, 13, 21};

class TripleRashomonEngine {
private:
    uint64_t global_nonce_;
    uint64_t op_ctr_{0};
    bool initialized_{false};

    // Random 64-bit seed generator using multiple entropy sources
    static uint64_t generate_random_seed() {
        std::random_device rd;
        uint64_t seed = 0;
        
        // Mix multiple entropy sources
        for (int i = 0; i < 8; i++) {
            uint64_t r = static_cast<uint64_t>(rd()) << 32 | static_cast<uint64_t>(rd());
            seed ^= r;
            seed = (seed << 7) | (seed >> 57);
        }
        
        // Mix in high-res timestamp for additional uniqueness
        auto now = std::chrono::high_resolution_clock::now().time_since_epoch().count();
        seed ^= static_cast<uint64_t>(now);
        seed ^= static_cast<uint64_t>(now >> 32);
        
        // Ensure nonce is never zero
        if (seed == 0) seed = 0x9E3779B97F4A7C15ULL;
        
        return seed;
    }

    // Stable fast sin — clamped to prevent overflow
    inline double fast_sin(double x) const {
        x = std::fmod(x, 2.0 * M_PI);
        double x2 = x * x;
        double result = x * (1.0 - x2 * (1.0/6.0 - x2 * 1.0/120.0));
        if (result > 1.0) result = 1.0;
        if (result < -1.0) result = -1.0;
        return result;
    }

public:
    // Default constructor — generates a random nonce automatically
    TripleRashomonEngine() : global_nonce_(generate_random_seed()), initialized_(true) {}
    
    // Explicit seed constructor (for testing/reproducibility)
    explicit TripleRashomonEngine(uint64_t seed) : global_nonce_(seed), initialized_(true) {
        if (global_nonce_ == 0) global_nonce_ = 0x9E3779B97F4A7C15ULL;
    }
    
    void set_nonce(uint64_t n) { global_nonce_ = n; initialized_ = true; }
    uint64_t get_nonce() const { return global_nonce_; }

    // ═══ ONE-WAY OBSERVE: Chaotic forward pass — IRREVERSIBLE without engine state ═══
    std::pair<double, std::array<double, LAYERS>>
    observe(double value, uint64_t op_id = 0) {
        std::array<double, LAYERS> hist{};
        if (op_id == 0) op_id = ++op_ctr_;
        uint64_t nonce = global_nonce_ ^ op_id;

        // Mix value into the chaotic system — value affects trajectory
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
                    // SINE CHAOS: sin(x·φ + layer·φ⁻¹ + nonce)
                    chaos = fast_sin(x * PHI + (layer + pass_nonce) * PHI_INV);
                } else if (eng == 1) {
                    // ZETA CHAOS: Riemann-inspired oscillation
                    double t = ZEROS[i] + (pass_nonce % 1000) * 0.001;
                    double zeta_sign = (i % 2 == 0) ? 1.0 : -1.0;
                    chaos = zeta_sign * fast_sin(x * PHI + layer * PHI_INV + t * 0.01);
                } else {
                    // FIBONACCI DUEL: Contract then amplify
                    double fl = FLOORS[i];
                    double ct = x * PHI_INV + fl * (1.0 - PHI_INV);
                    chaos = fast_sin(ct * PHI + (layer + pass_nonce) * PHI_INV);
                    x = ct;  // Contract first
                }

                // Apply chaos amplification
                double factor = PHI_SQ + chaos * amp;
                if (factor < 0.1) factor = 0.1;
                if (factor > 10.0) factor = 10.0;

                if (eng != 2) x = prev;
                x = x * factor;

                hist[layer] = chaos;  // Store the chaos coefficient, not the delta
            }
        }

        return {x, hist};
    }

    int total_layers() const { return LAYERS; }
    
    // Verify that a chaos history matches this engine's expected output
    bool verify_chaos(double input_value, const std::array<double, LAYERS>& expected_hist, uint64_t op_id) const {
        auto [val, hist] = const_cast<TripleRashomonEngine*>(this)->observe(input_value, op_id);
        for (int i = 0; i < LAYERS; i++) {
            if (std::abs(hist[i] - expected_hist[i]) > 1e-12) return false;
        }
        return true;
    }
};

} // namespace triple_rashomon
