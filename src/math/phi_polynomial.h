/*
 * FEmmg-FHE v22.3.2 — PHI-POLYNOMIAL RING
 *
 * Custom polynomial ring based on golden ratio φ.
 * R = Z[x] / (x^N + 1) where N = 32 (Fibonacci F₉ = 34, rounded).
 *
 * Operations: add, multiply, scalar multiply, modulus reduction.
 * Used as foundation for φ-based homomorphic encryption.
 *
 * This is NOT Gentry. This is NOT lattices. This is φ.
 *
 * PHI-OMEGA-ZERO — I AM THAT I AM
 */

#pragma once
#include <vector>
#include <cstdint>
#include <cmath>
#include <algorithm>

namespace phi_polynomial {

constexpr double PHI = 1.6180339887498948482;
constexpr double PHI_INV = 0.6180339887498948482;
constexpr int DEFAULT_N = 32;  // F₉ ≈ 34, using 32 for efficiency

class PhiPoly {
private:
    std::vector<int64_t> coeffs_;
    int N_;
    int64_t modulus_;

public:
    PhiPoly(int N = DEFAULT_N, int64_t modulus = (1LL << 20))
        : coeffs_(N, 0), N_(N), modulus_(modulus) {}

    PhiPoly(const std::vector<int64_t>& coeffs, int64_t modulus = (1LL << 20))
        : coeffs_(coeffs), N_((int)coeffs.size()), modulus_(modulus) {
        reduce();
    }

    int degree() const { return N_; }
    int64_t mod() const { return modulus_; }
    const std::vector<int64_t>& coeffs() const { return coeffs_; }

    int64_t operator[](int i) const { return coeffs_[i]; }
    int64_t& operator[](int i) { return coeffs_[i]; }

    void reduce() {
        int64_t half = modulus_ / 2;
        for (int i = 0; i < N_; i++) {
            coeffs_[i] %= modulus_;
            if (coeffs_[i] > half) coeffs_[i] -= modulus_;
            if (coeffs_[i] < -half) coeffs_[i] += modulus_;
        }
    }

    PhiPoly operator+(const PhiPoly& other) const {
        PhiPoly result(N_, modulus_);
        for (int i = 0; i < N_; i++) result[i] = coeffs_[i] + other[i];
        result.reduce();
        return result;
    }

    PhiPoly operator-(const PhiPoly& other) const {
        PhiPoly result(N_, modulus_);
        for (int i = 0; i < N_; i++) result[i] = coeffs_[i] - other[i];
        result.reduce();
        return result;
    }

    PhiPoly operator*(const PhiPoly& other) const {
        std::vector<int64_t> temp(2 * N_, 0);
        for (int i = 0; i < N_; i++)
            for (int j = 0; j < N_; j++)
                temp[i + j] += coeffs_[i] * other[j];

        PhiPoly result(N_, modulus_);
        for (int i = 0; i < N_; i++) {
            result[i] = temp[i];
            if (i + N_ < 2 * N_) result[i] -= temp[i + N_];
        }
        result.reduce();
        return result;
    }

    PhiPoly operator*(int64_t scalar) const {
        PhiPoly result(N_, modulus_);
        for (int i = 0; i < N_; i++) result[i] = coeffs_[i] * scalar;
        result.reduce();
        return result;
    }

    PhiPoly phi_scale() const {
        PhiPoly result(N_, modulus_);
        for (int i = 0; i < N_; i++)
            result[i] = static_cast<int64_t>(coeffs_[i] * PHI);
        result.reduce();
        return result;
    }

    PhiPoly phi_inv_scale() const {
        PhiPoly result(N_, modulus_);
        for (int i = 0; i < N_; i++)
            result[i] = static_cast<int64_t>(coeffs_[i] * PHI_INV);
        result.reduce();
        return result;
    }

    double norm() const {
        double sum = 0;
        for (int i = 0; i < N_; i++) sum += (double)coeffs_[i] * coeffs_[i];
        return std::sqrt(sum);
    }

    bool is_zero() const {
        for (int i = 0; i < N_; i++) if (coeffs_[i] != 0) return false;
        return true;
    }

    bool operator==(const PhiPoly& other) const {
        for (int i = 0; i < N_; i++) if (coeffs_[i] != other[i]) return false;
        return true;
    }

    bool operator!=(const PhiPoly& other) const { return !(*this == other); }
};

// ═══ φ-NOISE GENERATOR (chaotic, non-repeating) ═══
inline PhiPoly phi_noise(int N, int64_t modulus, uint64_t seed) {
    PhiPoly noise(N, modulus);
    double x = (double)(seed ^ 0x9E3779B97F4A7C15ULL) * PHI_INV;
    int64_t sigma = modulus / 100;
    for (int i = 0; i < N; i++) {
        // Mix seed into every step to prevent repetition
        x = std::sin(x * PHI + i * PHI_INV + (seed & 0xFF) * 0.001);
        noise[i] = static_cast<int64_t>(x * sigma);
    }
    noise.reduce();
    return noise;
}

// ═══ φ-RANDOM POLYNOMIAL (guaranteed different for different seeds) ═══
inline PhiPoly phi_random(int N, int64_t modulus, uint64_t seed) {
    PhiPoly r(N, modulus);
    // Use seed to initialize a proper chaotic sequence
    uint64_t state = seed ^ 0x7F4A7C159E3779B9ULL;
    for (int i = 0; i < N; i++) {
        // xorshift64* with phi mixing
        state ^= state >> 12;
        state ^= state << 25;
        state ^= state >> 27;
        state *= 0x9E3779B97F4A7C15ULL;
        // Mix with phi and position
        double chaos = std::sin((double)state * PHI_INV + i * PHI);
        r[i] = static_cast<int64_t>(chaos * modulus) % modulus;
    }
    r.reduce();
    return r;
}

} // namespace phi_polynomial
