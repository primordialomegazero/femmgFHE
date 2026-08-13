#pragma once
#include <cmath>
#include <vector>
#include <iostream>

namespace PureGolden {

constexpr double PHI = 1.6180339887498948482;
constexpr double PSI = -0.6180339887498948482;

// Golden bit representation
inline double to_golden(bool b) { return b ? PHI : PSI; }
inline bool from_golden(double v) { return v > 0.5; } // φ > 0, ψ < 0

// Natural multiplication
inline double golden_mul(double a, double b) {
    return a * b;
}

// Natural addition
inline double golden_add(double a, double b) {
    return a + b;
}

// FGG collapse (golden recursion)
inline double fgg(double v, int depth = 3) {
    double c = v;
    for (int i = 0; i < depth; i++) {
        c = std::abs(c * (i % 2 == 0 ? PHI * PSI : PSI * PHI));
    }
    return c;
}

// Natural NAND attempt
inline double golden_nand(double a, double b) {
    double prod = a * b;
    double one = PHI + PSI; // = 1
    return one - prod;
}

} // namespace PureGolden

// Pendulum swing: natural bootstrapping
inline double swing(double x) {
    return -1.0 / x;
}

inline double pendulum_bootstrap(double noisy) {
    double first = swing(noisy);
    double second = swing(first);
    return second;
}
