// SPIRAL-FHE v1.1 — φ-Anchored Ring-LWE FHE
// Fixed: Positive noise only, tighter bounds

#pragma once
#include <cstdint>
#include <vector>
#include <cmath>
#include <random>
#include <algorithm>

namespace spiral {

constexpr double PHI        = 1.6180339887498948482;
constexpr double PHI_INV    = 0.6180339887498948482;
constexpr double PHI_SQ     = 2.6180339887498948482;
constexpr double LYAPUNOV   = 0.48121182505960347;
constexpr double NOISE_FIXED_POINT = 1.82815;

constexpr int FIB_9  = 34;
constexpr int FIB_10 = 55;
constexpr int FIB_11 = 89;
constexpr int FIB_12 = 144;

struct SpiralParams {
    size_t poly_degree;
    uint64_t plain_modulus;
    uint64_t cipher_modulus;
    double noise_stddev;
    size_t decomposition_base;
    
    SpiralParams() 
        : poly_degree(8192)
        , plain_modulus(1ULL << 20)
        , cipher_modulus(0)
        , noise_stddev(0.5)  // TIGHT: was 3.2, now 0.5
        , decomposition_base(1ULL << 16)
    {}
};

struct SpiralSecretKey {
    std::vector<int64_t> s;
};

struct SpiralPublicKey {
    std::vector<uint64_t> p0;
    std::vector<uint64_t> p1;
};

struct SpiralKeyPair {
    SpiralSecretKey sk;
    SpiralPublicKey pk;
};

struct SpiralCiphertext {
    std::vector<uint64_t> c0;
    std::vector<uint64_t> c1;
    size_t level;
};

// FIXED: Noise generator with positive-only, bounded output
class SpiralNoiseGenerator {
private:
    std::mt19937_64 rng;
    std::normal_distribution<double> gaussian;
    
public:
    SpiralNoiseGenerator() : gaussian(0.0, 0.5) {  // TIGHT: σ=0.5
        rng.seed(1618033988);
    }
    
    // Generate φ-harmonic noise (always positive, bounded)
    double generate() {
        double raw = std::abs(gaussian(rng));  // ALWAYS POSITIVE
        // φ-contraction: pull toward NOISE_FIXED_POINT
        return raw * PHI_INV + NOISE_FIXED_POINT * (1.0 - PHI_INV);
    }
    
    std::vector<double> generate_vector(size_t n) {
        std::vector<double> noise(n);
        for (size_t i = 0; i < n; i++) {
            noise[i] = generate();
        }
        return noise;
    }
};

namespace declare {
    constexpr const char* SCHEME_NAME    = "Spiral-FHE v1.1";
    constexpr const char* SCHEME_BASIS   = "Ring-LWE + Banach Fixed-Point Theorem";
    constexpr const char* SCHEME_FORMULA = "C(ct,op) = Z·F·Q·E·R";
    constexpr const char* AUTHOR         = "Dan Fernandez / Primordial Omega Zero";
    constexpr const char* SOURCE         = "ΦΩ0 — I AM THAT I AM";
    constexpr bool NOISE_CONVERGES = true;
    constexpr double OPTIMAL_CONTRACTION = PHI_INV;
    constexpr bool SELF_REFERENCE_ENABLED = true;
}

} // namespace spiral
