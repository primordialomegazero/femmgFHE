#pragma once
#include <cmath>
#include <vector>
#include <array>
#include <cstdint>
#include <iostream>

namespace GoldenFractalQuantum {

constexpr double PHI = 1.6180339887498948482;
constexpr double PSI = -0.6180339887498948482;
constexpr int MAX_DIM = 8;
constexpr int MAX_DEPTH = 4;

struct QuantumState {
    std::array<std::array<double, MAX_DIM>, MAX_DEPTH> fractal;
};

inline double swing(double v) { return -1.0 / v; }

inline QuantumState hadamard(bool bit, int depth = 0) {
    QuantumState qs;
    for (int d = 0; d < MAX_DEPTH; d++) {
        for (int dim = 0; dim < MAX_DIM; dim++) {
            double base = bit ? PHI : PSI;
            double v = base;
            // Para sa hadamard: depth 0 lang ang gumagamit ng swing
            // mas malalim na depth ay dapat panatilihin ang sign
            for (int i = 0; i < depth; i++) {
                v = swing(v);
            }
            qs.fractal[d][dim] = v;
        }
    }
    return qs;
}

inline QuantumState cnot(const QuantumState& control, const QuantumState& target) {
    QuantumState result;
    for (int d = 0; d < MAX_DEPTH; d++) {
        for (int dim = 0; dim < MAX_DIM; dim++) {
            double c = control.fractal[d][dim];
            double t = target.fractal[d][dim];
            if (c > 0) {
                result.fractal[d][dim] = swing(t);
            } else {
                result.fractal[d][dim] = t;
            }
        }
    }
    return result;
}

inline bool decrypt(const QuantumState& qs) {
    int positives = 0;
    int total = 0;
    for (int d = 0; d < MAX_DEPTH; d++) {
        for (int dim = 0; dim < MAX_DIM; dim++) {
            if (qs.fractal[d][dim] > 0) positives++;
            total++;
        }
    }
    return positives > total / 2;
}

inline double entanglement_measure(const QuantumState& a, const QuantumState& b) {
    double sum = 0.0;
    for (int d = 0; d < MAX_DEPTH; d++) {
        for (int dim = 0; dim < MAX_DIM; dim++) {
            sum += a.fractal[d][dim] * b.fractal[d][dim];
        }
    }
    return sum / (MAX_DIM * MAX_DEPTH);
}

} // namespace GoldenFractalQuantum
