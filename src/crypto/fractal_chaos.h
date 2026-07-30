#pragma once
#include "../core/constants.h"
#include "../utils/safe_math.h"

inline double fibonacci_anchor(int n, double seed) {
    double f0 = SafeMath::fmod_safe(seed), f1 = SafeMath::fmod_safe(seed * PHI);
    for (int i = 0; i < n; i++) {
        double fn = f0 + f1; f0 = f1; f1 = SafeMath::fmod_safe(fn);
    }
    return SafeMath::fmod_safe(f1);
}

inline double lyapunov_estimate(double r, double x0, int n = 20) {
    double x = x0, sum = 0.0;
    for (int i = 0; i < n; i++) {
        sum += std::log(std::abs(r * (1.0 - 2.0 * x)) + 0.0001);
        x = r * x * (1.0 - x);
    }
    return sum / n;
}

inline double fractal_transform(double x, int layer, int depth) {
    double r = 3.7 + (layer * 0.05), result = x;
    for (int d = 0; d < depth; d++) {
        result = r * result * (1.0 - result);
        double angle = (layer + 1) * PHI * PI;
        result = result * SafeMath::cos_safe(angle) + (1.0 - result) * SafeMath::sin_safe(angle);
        result = SafeMath::fmod_safe(result);
    }
    return result;
}
