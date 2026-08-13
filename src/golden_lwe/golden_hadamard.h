#pragma once
#include <cmath>
#include <vector>
#include <cstdint>
#include <iostream>

namespace GoldenHadamard {

constexpr double PHI = 1.6180339887498948482;
constexpr double PSI = -0.6180339887498948482;

inline double golden_hadamard(bool bit) {
    // H|0⟩ = φ·|0⟩ + ψ·|1⟩
    // H|1⟩ = ψ·|0⟩ + φ·|1⟩
    if (bit == 0) {
        return PHI;  // φ
    } else {
        return PSI;  // ψ
    }
}

inline bool golden_hadamard_decrypt(double v) {
    return v > 0;  // φ > 0, ψ < 0
}

// Hadamard sa pamamagitan ng golden ratio
inline double hadamard_transform(double v) {
    // H(φ) = (φ + ψ)/√2 = 1/√2
    // H(ψ) = (φ - ψ)/√2 = √2
    return v;
}

} // namespace GoldenHadamard
