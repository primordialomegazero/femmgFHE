#pragma once
#include <cmath>
#include <vector>
#include <cstdint>
#include <iostream>
#include <array>
#include <complex>

namespace GoldenExactGates {

constexpr double PHI = 1.6180339887498948482;
constexpr double PSI = -0.6180339887498948482;

// ============================================================
// CLASSICAL GATES
// ============================================================
class GoldenExactSolver {
public:
    struct Result {
        bool satisfiable;
        std::vector<bool> assignment;
        double golden_score;
        long long steps;
    };

    static Result solve(const std::vector<std::vector<int>>& clauses, int num_vars) {
        Result result;
        result.satisfiable = false;
        result.golden_score = 0.0;
        result.steps = 0;

        std::vector<double> weights(num_vars, 0.0);

        for (const auto& clause : clauses) {
            for (int lit : clause) {
                int var = std::abs(lit) - 1;
                if (lit > 0) weights[var] += PHI;
                else weights[var] += PSI;
            }
            result.steps++;
        }

        result.assignment.resize(num_vars);
        for (int v = 0; v < num_vars; v++) {
            result.assignment[v] = weights[v] > 0;
        }

        int satisfied = 0;
        for (const auto& clause : clauses) {
            bool clause_satisfied = false;
            for (int lit : clause) {
                int var = std::abs(lit) - 1;
                bool val = result.assignment[var];
                if ((lit > 0 && val) || (lit < 0 && !val)) {
                    clause_satisfied = true;
                    break;
                }
            }
            if (clause_satisfied) satisfied++;
            result.steps++;
        }

        result.satisfiable = (satisfied == (int)clauses.size());
        result.golden_score = (double)satisfied / clauses.size() * PHI;
        return result;
    }
};

// ============================================================
// QUANTUM GATES
// ============================================================
struct QuantumState {
    double amplitude_0;
    double amplitude_1;
};

inline QuantumState hadamard_gate(const QuantumState& qs) {
    double a0 = (qs.amplitude_0 + qs.amplitude_1) / std::sqrt(2.0);
    double a1 = (qs.amplitude_0 - qs.amplitude_1) / std::sqrt(2.0);
    return {a0, a1};
}

inline QuantumState pauli_x(const QuantumState& qs) {
    return {qs.amplitude_1, qs.amplitude_0};
}

inline QuantumState pauli_y(const QuantumState& qs) {
    return {-qs.amplitude_1, qs.amplitude_0};
}

inline QuantumState pauli_z(const QuantumState& qs) {
    return {qs.amplitude_0, -qs.amplitude_1};
}

inline QuantumState cnot_gate(const QuantumState& control, const QuantumState& target) {
    if (control.amplitude_1 > control.amplitude_0) {
        return {target.amplitude_1, target.amplitude_0};
    }
    return target;
}

inline QuantumState phase_gate(const QuantumState& qs, double theta) {
    return {qs.amplitude_0, qs.amplitude_1 * std::exp(std::complex<double>(0, theta)).real()};
}

// Golden Quantum Gate Set
class GoldenQuantumGates {
public:
    static QuantumState apply_gate(const std::string& gate, const QuantumState& qs) {
        if (gate == "H") return hadamard_gate(qs);
        if (gate == "X") return pauli_x(qs);
        if (gate == "Y") return pauli_y(qs);
        if (gate == "Z") return pauli_z(qs);
        return qs;
    }

    static double measure(const QuantumState& qs) {
        return qs.amplitude_0 * qs.amplitude_0 + qs.amplitude_1 * qs.amplitude_1;
    }
};

} // namespace GoldenExactGates
