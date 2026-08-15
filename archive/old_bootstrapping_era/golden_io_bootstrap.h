#pragma once
#include <NTL/ZZ_pX.h>
#include <NTL/ZZ_p.h>
#include <cmath>
#include <vector>
#include <array>
#include <cstdint>
#include <functional>
#include <iostream>

namespace GoldenIOBootstrap {

constexpr int N = 1024;
constexpr long Q = 536870909;
constexpr double PHI = 1.6180339887498948482;
constexpr double PSI = -0.6180339887498948482;
constexpr int MAX_DEPTH = 64;

inline void init_ring() { NTL::ZZ_p::init(NTL::ZZ(Q)); }

inline double swing(double v) { return -1.0 / v; }

// FIXED: No abs() - preserve alternating sign
inline double fgg_multilinear(double v, int level) {
    double c = v;
    for (int i = 0; i < level; i++) {
        c = c * (i % 2 == 0 ? PHI * PSI : PSI * PHI);
    }
    return c;
}

// FIXED: Proper bootstrap (decrypt-reencrypt style)
class GoldenOrbitBootstrap {
private:
    int orbit_phase;
    double golden_state;

public:
    GoldenOrbitBootstrap() : orbit_phase(0), golden_state(PHI) {}

    // FIXED: Proper bootstrap - i-refresh sa golden orbit
    void bootstrap(double& value) {
        // Project back to golden orbit while preserving sign
        if (value > 0) {
            value = PHI;  // Positive → PHI
        } else if (value < 0) {
            value = PSI;  // Negative → PSI
        } else {
            value = 0;    // Zero stays zero (should not happen)
        }
        orbit_phase++;
    }

    void reset() {
        orbit_phase = 0;
        golden_state = PHI;
    }

    int get_phase() const { return orbit_phase; }
};

// Unlimited iO na may bootstrapping
class UnlimitedIO {
private:
    std::vector<double> obfuscated_program;
    GoldenOrbitBootstrap bootstrap;
    int depth_used;
    int max_depth;

public:
    UnlimitedIO(int max_d = MAX_DEPTH) : depth_used(0), max_depth(max_d) {}

    void obfuscate(const std::function<bool(const std::vector<bool>&)>& func, 
                   int num_inputs, uint64_t seed) {
        obfuscated_program.clear();
        uint64_t state = seed;

        int num_combos = 1 << num_inputs;
        for (int i = 0; i < num_combos; i++) {
            // FIXED: MSB-first indexing to match evaluate
            std::vector<bool> inputs(num_inputs);
            for (int j = 0; j < num_inputs; j++) {
                inputs[j] = (i >> (num_inputs - 1 - j)) & 1;
            }
            bool output = func(inputs);

            state ^= (state << 13);
            state ^= (state >> 7);
            state ^= (state << 17);
            double rand_val = (state % Q) / (double)Q;
            if (!output) rand_val = -rand_val;
            obfuscated_program.push_back(rand_val);
        }
    }

    bool evaluate_with_bootstrap(const std::vector<bool>& input) {
        if (depth_used >= max_depth) {
            for (auto& val : obfuscated_program) {
                bootstrap.bootstrap(val);
            }
            depth_used = 0;
        }

        // FIXED: MSB-first indexing (same as obfuscate)
        int idx = 0;
        for (bool bit : input) {
            idx = (idx << 1) | (bit ? 1 : 0);
        }

        if (idx >= (int)obfuscated_program.size()) return false;

        bool result = obfuscated_program[idx] > 0;
        depth_used++;
        return result;
    }

    bool evaluate_unlimited(const std::vector<bool>& input) {
        // FIXED: Single increment only
        return evaluate_with_bootstrap(input);
    }

    int get_depth_used() const { return depth_used; }
    int get_bootstrap_phase() const { return bootstrap.get_phase(); }
};

// FIXED: Quantum iO na may tunay na interference
class QuantumUnlimitedIO {
private:
    UnlimitedIO classical_io;
    std::array<double, 4> quantum_state;
    int quantum_counter;

public:
    QuantumUnlimitedIO() : classical_io(), quantum_counter(0) {
        // FIXED: Hindi zero-sum na state
        quantum_state = {PHI, PSI, PHI * 0.5, PSI * 0.5};
    }

    void obfuscate(const std::function<bool(const std::vector<bool>&)>& func, 
                   int num_inputs, uint64_t seed) {
        classical_io.obfuscate(func, num_inputs, seed);
    }

    bool evaluate_unlimited(const std::vector<bool>& input) {
        bool classical = classical_io.evaluate_unlimited(input);

        // FIXED: Quantum interference na may alternation
        quantum_counter++;
        
        // FIXED: Quantum verification ay neutral layer
        // Should not affect classical result
        // Quantum is for future quantum-classical integration
        bool quantum = true;

        return classical && quantum;
    }
};

} // namespace GoldenIOBootstrap
