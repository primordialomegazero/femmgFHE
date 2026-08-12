#pragma once
#include <cmath>
#include <stdexcept>

class SafeMath {
public:
    static double fmod_safe(double x, double y = 1.0) {
        if (std::abs(y) < 1e-12) return 0.0;
        return std::fmod(x, y);
    }
    static double abs_safe(double x) { return std::abs(x); }
    static double clamp(double x, double lo = 0.0, double hi = 1.0) {
        if (x < lo) return lo;
        if (x > hi) return hi;
        return x;
    }
    static bool is_valid(double x) { return !std::isnan(x) && !std::isinf(x); }
};
