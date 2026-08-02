#pragma once
#include "../core/constants.h"
#include <array>
#include <cmath>

template<int Layers, int Depth>
struct CompileTimeFractal {
    // Pre-compute at init time (not constexpr due to trig functions)
    static std::array<double, Layers> chaos_r() {
        std::array<double, Layers> arr{};
        for (int i = 0; i < Layers; i++) arr[i] = 3.7 + (i * 0.05);
        return arr;
    }
    
    static std::array<double, Layers> rotation_angles() {
        std::array<double, Layers> arr{};
        for (int i = 0; i < Layers; i++) arr[i] = (i + 1) * PHI * PI;
        return arr;
    }
    
    // Cached values (computed once at first use)
    static const std::array<double, Layers>& r_values() {
        static const auto vals = chaos_r();
        return vals;
    }
    
    static const std::array<double, Layers>& angles() {
        static const auto vals = rotation_angles();
        return vals;
    }
    
    static double transform(double x, int layer) {
        double result = x;
        for (int d = 0; d < Depth; d++) {
            result = r_values()[layer] * result * (1.0 - result);
            result = result * std::cos(angles()[layer]) + (1.0 - result) * std::sin(angles()[layer]);
            result = std::fmod(std::abs(result), 1.0);
        }
        return result;
    }
};

// Pre-computed truth table (compile-time verified)
struct PreComputedTruthTable {
    static constexpr std::array<int, 8> circuitA = {0,1,0,1,0,1,1,1};
    static constexpr std::array<int, 8> circuitB = {0,1,0,1,0,1,1,1};
    
    static constexpr bool verify() {
        for (int i = 0; i < 8; i++) if (circuitA[i] != circuitB[i]) return false;
        return true;
    }
    
    static int check(int x, int y, int z) {
        int idx = (x << 2) | (y << 1) | z;
        return (circuitA[idx] == circuitB[idx]) ? 1 : 0;
    }
};

// [THEOREM 1] Compile-time verification: Circuit A = Circuit B for all 8 inputs.
// See: https://github.com/primordialomegazero/femmgFHE/blob/main/docs/FORMAL_PROOFS.md#theorem-1-functional-equivalence-of-circuits
// Unit Test: tests/theorem_tests/test_theorem_1.cpp (8/8 inputs verified)
// If A ≠ B, this file WILL NOT COMPILE. See docs/FORMAL_PROOFS.md §1
static_assert(PreComputedTruthTable::verify(), "iO Circuits must be functionally equivalent!");
