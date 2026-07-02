/*
 * FEmmg-FHE v22.3.2 — PHI-POLYNOMIAL RING
 *
 * Custom polynomial ring based on golden ratio φ.
 * R = Z[x] / (x^N + 1) where N is φ-derived (Fibonacci numbers).
 *
 * Operations: add, multiply, scalar multiply, modulus reduction.
 * Used as foundation for Ring-LWE style encryption.
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

// N = polynomial degree (Fibonacci number: 8, 13, 21, 34)
// Larger N = more security, slower ops
constexpr int DEFAULT_N = 8;  // φ-derived: F₆ = 8

class PhiPoly {
private:
    std::vector<int64_t> coeffs_;  // coefficients in Z
    int N_;                         // degree = number of coefficients
    int64_t modulus_;               // for coefficient reduction

public:
    // ═══ CONSTRUCTORS ═══
    PhiPoly(int N = DEFAULT_N, int64_t modulus = (1LL << 20))
        : coeffs_(N, 0), N_(N), modulus_(modulus) {}
    
    PhiPoly(const std::vector<int64_t>& coeffs, int64_t modulus = (1LL << 20))
        : coeffs_(coeffs), N_((int)coeffs.size()), modulus_(modulus) {
        reduce();
    }

    // ═══ ACCESSORS ═══
    int degree() const { return N_; }
    int64_t mod() const { return modulus_; }
    const std::vector<int64_t>& coeffs() const { return coeffs_; }
    
    int64_t operator[](int i) const { return coeffs_[i]; }
    int64_t& operator[](int i) { return coeffs_[i]; }

    // ═══ MODULUS REDUCTION (centered around 0) ═══
    void reduce() {
        int64_t half = modulus_ / 2;
        for (int i = 0; i < N_; i++) {
            coeffs_[i] %= modulus_;
            if (coeffs_[i] > half) coeffs_[i] -= modulus_;
            if (coeffs_[i] < -half) coeffs_[i] += modulus_;
        }
    }

    // ═══ POLYNOMIAL ADDITION ═══
    PhiPoly operator+(const PhiPoly& other) const {
        PhiPoly result(N_, modulus_);
        for (int i = 0; i < N_; i++) {
            result[i] = coeffs_[i] + other[i];
        }
        result.reduce();
        return result;
    }

    // ═══ POLYNOMIAL SUBTRACTION ═══
    PhiPoly operator-(const PhiPoly& other) const {
        PhiPoly result(N_, modulus_);
        for (int i = 0; i < N_; i++) {
            result[i] = coeffs_[i] - other[i];
        }
        result.reduce();
        return result;
    }

    // ═══ POLYNOMIAL MULTIPLICATION (in Z[x] / (x^N + 1)) ═══
    PhiPoly operator*(const PhiPoly& other) const {
        // Full multiplication: deg(result) = 2N - 2
        std::vector<int64_t> temp(2 * N_, 0);
        
        for (int i = 0; i < N_; i++) {
            for (int j = 0; j < N_; j++) {
                temp[i + j] += coeffs_[i] * other[j];
            }
        }
        
        // Reduction modulo (x^N + 1):
        // x^N = -1, so x^(N+k) = -x^k
        PhiPoly result(N_, modulus_);
        for (int i = 0; i < N_; i++) {
            result[i] = temp[i];
            if (i + N_ < 2 * N_) {
                result[i] -= temp[i + N_];  // x^N = -1
            }
        }
        result.reduce();
        return result;
    }

    // ═══ SCALAR MULTIPLICATION ═══
    PhiPoly operator*(int64_t scalar) const {
        PhiPoly result(N_, modulus_);
        for (int i = 0; i < N_; i++) {
            result[i] = coeffs_[i] * scalar;
        }
        result.reduce();
        return result;
    }

    // ═══ φ-SCALING (multiply by φ, then reduce) ═══
    PhiPoly phi_scale() const {
        PhiPoly result(N_, modulus_);
        for (int i = 0; i < N_; i++) {
            result[i] = static_cast<int64_t>(coeffs_[i] * PHI);
        }
        result.reduce();
        return result;
    }

    // ═══ φ-INVERSE SCALING ═══
    PhiPoly phi_inv_scale() const {
        PhiPoly result(N_, modulus_);
        for (int i = 0; i < N_; i++) {
            result[i] = static_cast<int64_t>(coeffs_[i] * PHI_INV);
        }
        result.reduce();
        return result;
    }

    // ═══ NORM (L2) ═══
    double norm() const {
        double sum = 0;
        for (int i = 0; i < N_; i++) {
            sum += (double)coeffs_[i] * coeffs_[i];
        }
        return std::sqrt(sum);
    }

    // ═══ CHECK IF ZERO ═══
    bool is_zero() const {
        for (int i = 0; i < N_; i++) {
            if (coeffs_[i] != 0) return false;
        }
        return true;
    }

    // ═══ EQUALITY ═══
    bool operator==(const PhiPoly& other) const {
        for (int i = 0; i < N_; i++) {
            if (coeffs_[i] != other[i]) return false;
        }
        return true;
    }
    
    bool operator!=(const PhiPoly& other) const {
        return !(*this == other);
    }
};

// ═══ PHI-NOISE GENERATOR (Gaussian-like via φ) ═══
inline PhiPoly phi_noise(int N, int64_t modulus, uint64_t seed) {
    PhiPoly noise(N, modulus);
    // Chaotic noise generation using φ and seed
    double x = (double)(seed ^ 0x9E3779B97F4A7C15ULL) * PHI_INV;
    for (int i = 0; i < N; i++) {
        x = std::sin(x * PHI + i * PHI_INV);
        // Map to small coefficient in [-σ, σ]
        int64_t sigma = modulus / 100;  // noise magnitude
        noise[i] = static_cast<int64_t>(x * sigma);
    }
    noise.reduce();
    return noise;
}

// ═══ PHI-RANDOM POLYNOMIAL ═══
inline PhiPoly phi_random(int N, int64_t modulus, uint64_t seed) {
    PhiPoly r(N, modulus);
    double x = (double)(seed ^ 0x7F4A7C159E3779B9ULL) * PHI;
    for (int i = 0; i < N; i++) {
        x = std::sin(x * PHI + i * PHI_INV) * 1e9;
        r[i] = static_cast<int64_t>(x) % modulus;
    }
    r.reduce();
    return r;
}

} // namespace phi_polynomial
