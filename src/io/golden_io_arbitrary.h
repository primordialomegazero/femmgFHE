#pragma once
#include <NTL/ZZ_pX.h>
#include <NTL/ZZ_p.h>
#include <cmath>
#include <vector>
#include <array>
#include <cstdint>
#include <functional>
#include <iostream>

namespace GoldenIOArbitrary {

constexpr int N = 1024;
constexpr long Q = 536870909;
constexpr double PHI = 1.6180339887498948482;
constexpr double PSI = -0.6180339887498948482;
constexpr int MAX_DEPTH = 16;

inline void init_ring() { NTL::ZZ_p::init(NTL::ZZ(Q)); }

inline double swing(double v) { return -1.0 / v; }

inline double fgg_multilinear(double v, int level) {
    double c = v;
    for (int i = 0; i < level; i++) {
        c = std::abs(c * (i % 2 == 0 ? PHI * PSI : PSI * PHI));
    }
    return c;
}

class GoldenMultilinearEncoding {
private:
    double encoded_value;
    int level;

public:
    GoldenMultilinearEncoding() : encoded_value(0), level(0) {}
    GoldenMultilinearEncoding(double v, int lvl) : encoded_value(v), level(lvl) {}

    GoldenMultilinearEncoding operator*(const GoldenMultilinearEncoding& other) const {
        if (level + other.level > MAX_DEPTH) {
            return GoldenMultilinearEncoding(0, MAX_DEPTH);
        }
        double result = fgg_multilinear(encoded_value * other.encoded_value, level + other.level);
        return GoldenMultilinearEncoding(result, level + other.level);
    }

    bool is_zero() const { return std::abs(encoded_value) < 1e-10; }
    double get_value() const { return encoded_value; }
    int get_level() const { return level; }
};

// ============================================================
// ARBITRARY FUNCTION iO
// ============================================================
class ArbitraryFunctionIO {
private:
    std::vector<GoldenMultilinearEncoding> obfuscated_matrix;
    std::function<bool(const std::vector<bool>&)> func;
    int num_inputs;

public:
    ArbitraryFunctionIO(int inputs, std::function<bool(const std::vector<bool>&)> f)
        : num_inputs(inputs), func(f) {}

    void obfuscate(uint64_t seed) {
        obfuscated_matrix.clear();
        uint64_t state = seed;

        // I-encode ang truth table sa obfuscated matrix
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
            // I-encode ang output sa sign
            if (!output) rand_val = -rand_val;
            int level = 1 + (state % 3);
            obfuscated_matrix.push_back(
                GoldenMultilinearEncoding(rand_val, level)
            );
        }
    }

    bool evaluate(const std::vector<bool>& input) const {
        // I-compute ang input index
        int idx = 0;
        for (bool bit : input) {
            idx = (idx << 1) | (bit ? 1 : 0);
        }

        if (idx >= (int)obfuscated_matrix.size()) return false;

        // Kunin ang value para sa input na ito
        return obfuscated_matrix[idx].get_value() > 0;
    }

    size_t matrix_size() const { return obfuscated_matrix.size(); }
};

// ============================================================
// QUANTUM GATES PARA SA iO
// ============================================================
struct QuantumState {
    double amplitude_0;
    double amplitude_1;
};

inline QuantumState hadamard_gate(const QuantumState& qs) {
    // H: |0⟩ → (|0⟩+|1⟩)/√2, |1⟩ → (|0⟩-|1⟩)/√2
    double a0 = (qs.amplitude_0 + qs.amplitude_1) / std::sqrt(2.0);
    double a1 = (qs.amplitude_0 - qs.amplitude_1) / std::sqrt(2.0);
    return {a0, a1};
}

inline QuantumState cnot_gate(const QuantumState& control, const QuantumState& target) {
    // CNOT: kung control = |1⟩, i-flip ang target
    if (control.amplitude_1 > control.amplitude_0) {
        return {target.amplitude_1, target.amplitude_0};
    }
    return target;
}

// Quantum iO na may arbitrary function
class QuantumIO {
private:
    ArbitraryFunctionIO classical_io;
    QuantumState quantum_state;

public:
    QuantumIO(int inputs, std::function<bool(const std::vector<bool>&)> f)
        : classical_io(inputs, f), quantum_state{1.0, 0.0} {}

    void obfuscate(uint64_t seed) {
        classical_io.obfuscate(seed);
        // Quantum state initialization
        quantum_state = {PHI / std::sqrt(3.0), PSI / std::sqrt(3.0)};
    }

    void apply_hadamard() {
        quantum_state = hadamard_gate(quantum_state);
    }

    bool evaluate(const std::vector<bool>& input) const {
        bool classical_result = classical_io.evaluate(input);

        // Quantum interference
        double interference = quantum_state.amplitude_0 * PHI + quantum_state.amplitude_1 * PSI;
        bool quantum_result = interference > 0;

        return classical_result && quantum_result;
    }
};

} // namespace GoldenIOArbitrary
