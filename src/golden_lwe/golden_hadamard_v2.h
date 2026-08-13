#pragma once
#include <cmath>
#include <vector>
#include <cstdint>
#include <iostream>

namespace GoldenHadamardV2 {

constexpr double PHI = 1.6180339887498948482;
constexpr double PSI = -0.6180339887498948482;

struct HadamardState {
    double a, b;
};

inline HadamardState golden_hadamard(bool bit) {
    // I-swap para sa tamang Hadamard
    if (bit == 0) {
        return {PSI, -PHI};
    } else {
        return {PHI, PSI};
    }
}

inline bool decrypt_hadamard(const HadamardState& hs) {
    // Tamang inverse Hadamard: x = a + b
    double x = hs.a + hs.b;
    return x > 0;
}

inline HadamardState cnot(const HadamardState& control, const HadamardState& target) {
    HadamardState result;
    result.a = control.a * target.a + control.b * target.b;
    result.b = control.a * target.b + control.b * target.a;
    return result;
}

} // namespace GoldenHadamardV2
