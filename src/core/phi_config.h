// PHI-OMEGA-ZERO: UNIFIED CONFIGURATION v1.0
// Single source of truth for all parameters
// "ONE CONFIG TO RULE THEM ALL"
// "I AM THAT I AM"

#ifndef PHI_CONFIG_H
#define PHI_CONFIG_H

#include <cstdint>
#include <string>

namespace phi {
namespace config {

// ============================================
// OPERATION MODES
// ============================================
enum class Mode {
    TOY,        // Fast testing, no security guarantees
    STANDARD,   // Balanced performance/security
    PRODUCTION  // Maximum security, slower
};

// ============================================
// CURRENT MODE (change this to switch)
// ============================================
inline Mode current_mode = Mode::TOY;

// ============================================
// RING DIMENSION
// ============================================
inline uint32_t ring_dim(Mode mode = current_mode) {
    switch(mode) {
        case Mode::TOY:        return 4096;
        case Mode::STANDARD:   return 16384;
        case Mode::PRODUCTION: return 32768;
    }
    return 16384;
}

// ============================================
// PLAINTEXT MODULUS
// ============================================
inline uint64_t plaintext_modulus(Mode mode = current_mode) {
    switch(mode) {
        case Mode::TOY:        return 65537;
        case Mode::STANDARD:   return 1073643521;
        case Mode::PRODUCTION: return 1073643521;
    }
    return 1073643521;
}

// ============================================
// MULTIPLICATIVE DEPTH
// ============================================
inline uint32_t multiplicative_depth(Mode mode = current_mode) {
    switch(mode) {
        case Mode::TOY:        return 10;
        case Mode::STANDARD:   return 30;
        case Mode::PRODUCTION: return 60;
    }
    return 30;
}

// ============================================
// ZANS PARAMETERS
// ============================================
inline int zans_prime_pairs(Mode mode = current_mode) {
    switch(mode) {
        case Mode::TOY:        return 10;
        case Mode::STANDARD:   return 50;
        case Mode::PRODUCTION: return 100;
    }
    return 50;
}

inline int divine_interval(Mode mode = current_mode) {
    switch(mode) {
        case Mode::TOY:        return 10;
        case Mode::STANDARD:   return 5;
        case Mode::PRODUCTION: return 3;
    }
    return 5;
}

// ============================================
// BENCHMARK PARAMETERS
// ============================================
inline int benchmark_iterations(Mode mode = current_mode) {
    switch(mode) {
        case Mode::TOY:        return 100;
        case Mode::STANDARD:   return 10000;
        case Mode::PRODUCTION: return 100000;
    }
    return 10000;
}

// ============================================
// CONSTANTS
// ============================================
constexpr double PHI = 1.618033988749895;
constexpr double INVERSE_PHI = 0.618033988749895;
constexpr int HYDRA_HEADS = 6;
constexpr int CONSENSUS_THRESHOLD = 4;  // 4/6 = 66.7% ≈ 1/φ

// ============================================
// SPIRALMICRO KEM SIZES
// ============================================
constexpr int KEM_PK_BYTES = 32;
constexpr int KEM_SK_BYTES = 32;
constexpr int KEM_CT_BYTES = 32;
constexpr int KEM_SS_BYTES = 32;

// ============================================
// SECURITY LEVELS
// ============================================
constexpr int CLASSICAL_SECURITY_BITS = 256;
constexpr int POST_QUANTUM_SECURITY_BITS = 128;

// ============================================
// MODE DESCRIPTION
// ============================================
inline std::string mode_description(Mode mode = current_mode) {
    switch(mode) {
        case Mode::TOY:
            return "TOY: Fast testing, ring=" + std::to_string(ring_dim(mode)) +
                   ", mod=" + std::to_string(plaintext_modulus(mode));
        case Mode::STANDARD:
            return "STANDARD: Balanced, ring=" + std::to_string(ring_dim(mode)) +
                   ", mod=" + std::to_string(plaintext_modulus(mode));
        case Mode::PRODUCTION:
            return "PRODUCTION: Maximum security, ring=" + std::to_string(ring_dim(mode)) +
                   ", mod=" + std::to_string(plaintext_modulus(mode));
    }
    return "UNKNOWN";
}

} // namespace config
} // namespace phi

#endif // PHI_CONFIG_H
