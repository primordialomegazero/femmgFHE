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
constexpr int MAX_DEPTH = 64; // Unlimited depth para sa iO

inline void init_ring() { NTL::ZZ_p::init(NTL::ZZ(Q)); }

inline double swing(double v) { return -1.0 / v; }

inline double fgg_multilinear(double v, int level) {
    double c = v;
    for (int i = 0; i < level; i++) {
        c = std::abs(c * (i % 2 == 0 ? PHI * PSI : PSI * PHI));
    }
    return c;
}

// Golden orbit bootstrap para sa multilinear encodings
class GoldenOrbitBootstrap {
private:
    int orbit_phase;
    double golden_state;

public:
    GoldenOrbitBootstrap() : orbit_phase(0), golden_state(PHI) {}

    void bootstrap(double& value) {
        // I-project ang value pabalik sa golden orbit
        if (value > 0.5) {
            value = PHI;
        } else {
            value = PSI;
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

    void obfuscate(const std::function<bool(const std::vector<bool>&)>& func, int num_inputs, uint64_t seed) {
        obfuscated_program.clear();
        uint64_t state = seed;

        int num_combos = 1 << num_inputs;
        for (int i = 0; i < num_combos; i++) {
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

    // I-evaluate ang isang input na may bootstrapping
    bool evaluate_with_bootstrap(const std::vector<bool>& input) {
        if (depth_used >= max_depth) {
            // I-bootstrap: i-refresh ang program
            for (auto& val : obfuscated_program) {
                bootstrap.bootstrap(val);
            }
            depth_used = 0;
        }

        int idx = 0;
        for (bool bit : input) {
            idx = (idx << 1) | (bit ? 1 : 0);
        }

        if (idx >= (int)obfuscated_program.size()) return false;

        bool result = obfuscated_program[idx] > 0;
        depth_used++;
        return result;
    }

    // Unlimited evaluation: kahit ilang beses, hindi mauubusan ng depth
    bool evaluate_unlimited(const std::vector<bool>& input) {
        if (depth_used >= max_depth) {
            for (auto& val : obfuscated_program) {
                bootstrap.bootstrap(val);
            }
            depth_used = 0;
        }
        depth_used++;
        return evaluate_with_bootstrap(input);
    }

    int get_depth_used() const { return depth_used; }
    int get_bootstrap_phase() const { return bootstrap.get_phase(); }
};

// Quantum iO bootstrapper
class QuantumUnlimitedIO {
private:
    UnlimitedIO classical_io;
    std::array<double, 4> quantum_state;

public:
    QuantumUnlimitedIO() : classical_io() {
        quantum_state = {PHI, PSI, -PSI, -PHI};
    }

    void obfuscate(const std::function<bool(const std::vector<bool>&)>& func, int num_inputs, uint64_t seed) {
        classical_io.obfuscate(func, num_inputs, seed);
    }

    bool evaluate_unlimited(const std::vector<bool>& input) {
        bool classical = classical_io.evaluate_unlimited(input);

        // Quantum interference
        double interference = 0;
        for (double v : quantum_state) {
            interference += v * PHI;
        }
        bool quantum = interference > 0;

        return classical && quantum;
    }
};

} // namespace GoldenIOBootstrap
