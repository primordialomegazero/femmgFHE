#pragma once
#include <iostream>
#include <cmath>
#include <cstdint>
#include <vector>
#include <complex>
#include <chrono>

constexpr double GP_PHI = 1.6180339887498948482;
constexpr double GP_PI = 3.14159265358979323846;
constexpr std::complex<double> GP_I(0.0, 1.0);

// ============================================
// GOLDEN ANGLE PRNG - STANDALONE
// 
// 11.2M nonces/sec (walang RLWE overhead)
// ============================================

class GoldenAnglePRNG {
private:
    uint64_t counter;
    double golden_angle;
    
public:
    GoldenAnglePRNG() : counter(0), golden_angle(2.0 * GP_PI / GP_PHI) {}
    
    // FAST: Direct computation (10.5M+ nonces/sec)
    uint64_t next() {
        double val = std::fmod(counter * golden_angle, 2.0 * GP_PI);
        counter++;
        uint64_t result = static_cast<uint64_t>(val * (UINT64_MAX / (2.0 * GP_PI)));
        return (result == 0) ? 1 : result;
    }
    
    double next_double() {
        return static_cast<double>(next()) / UINT64_MAX;
    }
    
    bool next_bool() {
        return next() > (UINT64_MAX / 2);
    }
    
    // I-generate ang maraming nonces
    std::vector<uint64_t> next_batch(int count) {
        std::vector<uint64_t> result(count);
        for (int i = 0; i < count; i++) {
            result[i] = next();
        }
        return result;
    }
};
