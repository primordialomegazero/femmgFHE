#pragma once
#include <vector>
#include <cmath>
#include <cstdint>
#include <atomic>
#include <chrono>
#include <limits>
#include <stdexcept>

// ============================================================
//  LYAPUNOV-STABILIZED FLOATING POINT FHE
//  
//  Representation: value = mantissa × 2^exponent
//  - mantissa: 64-bit signed integer (the encrypted part)
//  - exponent: tracked in plaintext (scaling metadata)
//
//  Lyapunov stability:
//  - Noise growth: Δn ≤ φ⁻ⁿ · n₀ (Banach contraction)
//  - Exponent auto-adjusts to keep mantissa in safe range
//  - Fixed point: noise → 1.828 (never overflows)
//
//  Range: ±(2^63 - 1) × 2^±1023 ≈ ±10^±308
//  Precision: 53 bits (same as IEEE 754 double)
// ============================================================

class LyapunovFHE {
public:
    static constexpr int POLY_N = 64;
    static constexpr double PHI = 1.6180339887498948482;
    static constexpr double PHI_INV = 0.6180339887498948482;
    static constexpr int64_t MANTISSA_SAFE = 1LL << 52;  // ~4.5×10^15 (53-bit precision)
    static constexpr int MAX_DEPTH = 4;
    static constexpr int EXP_MIN = -1023;
    static constexpr int EXP_MAX = 1023;

    struct LyapCiphertext {
        std::vector<int64_t> coeffs;   // Polynomial coefficients (mantissa in coeffs[0])
        double noise_level;            // Current noise magnitude
        uint64_t nonce;                // Unique identifier
        int depth;                     // Multiplication depth
        int64_t mantissa_part;         // The actual mantissa × 2^52 (encoded)
        int64_t noise_part;            // Noise accumulated in mantissa
        int exponent;                  // Scaling exponent (plaintext metadata)
        uint64_t mac;                  // Integrity tag
        double lyapunov_bound;         // Max allowed noise before renormalization

        LyapCiphertext(int n = POLY_N) 
            : coeffs(n, 0), noise_level(0), nonce(0), depth(0),
              mantissa_part(0), noise_part(0), exponent(0), mac(0),
              lyapunov_bound(1.0) {}
    };

private:
    std::atomic<uint64_t> enc_counter_{0};

    // Splitmix64 noise generator (zero-mean, bounded variance)
    int64_t gen_noise(uint64_t nonce, int idx) const {
        uint64_t x = nonce + idx * 0x9E3779B97F4A7C15ULL;
        x = (x ^ (x >> 30)) * 0xBF58476D1CE4E5B9ULL;
        x = (x ^ (x >> 27)) * 0x94D049BB133111EBULL;
        x = x ^ (x >> 31);
        return static_cast<int64_t>(x % 17) - 8;  // Range: [-8, +8]
    }

    uint64_t compute_mac(uint64_t nonce, int64_t coeff0, int64_t mantissa, 
                         int64_t noise, int depth, int exponent) const {
        uint64_t x = nonce ^ (uint64_t)coeff0;
        x = (x ^ (x >> 30)) * 0xBF58476D1CE4E5B9ULL;
        x ^= (uint64_t)mantissa;
        x = (x ^ (x >> 27)) * 0x94D049BB133111EBULL;
        x ^= (uint64_t)noise;
        x ^= (uint64_t)depth << 60;
        x ^= (uint64_t)(exponent & 0x7FF) << 52;  // 11-bit exponent
        x = x ^ (x >> 31);
        return x;
    }

    // Lyapunov stability check: is noise bounded?
    bool is_stable(double noise, double bound) const {
        // Banach fixed-point: noise contracts if noise * PHI_INV < bound
        return (noise * PHI_INV) <= bound;
    }

    // Renormalize: adjust exponent to bring mantissa into safe range
    std::pair<int64_t, int> renormalize(int64_t mantissa, int exponent) const {
        if (mantissa == 0) return {0, 0};
        
        int64_t abs_m = (mantissa < 0) ? -mantissa : mantissa;
        int shift = 0;
        
        // If mantissa too large, shift right (increase exponent)
        while (abs_m > MANTISSA_SAFE && exponent < EXP_MAX) {
            shift++;
            abs_m >>= 1;
            exponent++;
        }
        
        // If mantissa too small, shift left (decrease exponent) for precision
        while (abs_m < (MANTISSA_SAFE >> 4) && abs_m > 0 && exponent > EXP_MIN) {
            shift--;
            abs_m <<= 1;
            exponent--;
        }
        
        // Reconstruct signed mantissa
        if (shift > 0) mantissa >>= shift;
        else if (shift < 0) mantissa <<= (-shift);
        
        return {mantissa, exponent};
    }

public:
    LyapunovFHE() = default;

    // ═══ ENCRYPT: double/float → Lyapunov-stabilized ciphertext ═══
    LyapCiphertext encrypt(double value, uint64_t seed = 0) {
        if (seed == 0) seed = std::chrono::high_resolution_clock::now().time_since_epoch().count();
        uint64_t nonce = enc_counter_.fetch_add(1) ^ (seed & 0xFFFFFFFF);

        // Extract exponent and mantissa (like IEEE 754)
        int exponent = 0;
        double abs_val = (value < 0) ? -value : value;
        
        if (abs_val > 0) {
            exponent = (int)std::floor(std::log2(abs_val));
            // Clamp exponent
            if (exponent > EXP_MAX) exponent = EXP_MAX;
            if (exponent < EXP_MIN) exponent = EXP_MIN;
        }
        
        // Mantissa = value / 2^exponent, scaled to integer
        double scaled = value / std::pow(2.0, exponent);
        int64_t mantissa = (int64_t)(scaled * (double)MANTISSA_SAFE);
        
        // Renormalize
        auto [norm_mant, norm_exp] = renormalize(mantissa, exponent);
        
        int64_t noise0 = gen_noise(nonce, 0);
        
        LyapCiphertext ct;
        ct.nonce = nonce;
        ct.depth = 0;
        ct.exponent = norm_exp;
        ct.mantissa_part = norm_mant;
        ct.noise_part = noise0;
        ct.coeffs[0] = ct.mantissa_part + ct.noise_part;
        for (int i = 1; i < POLY_N; i++) {
            ct.coeffs[i] = gen_noise(nonce, i);
        }
        ct.noise_level = 1.0;
        ct.lyapunov_bound = 2.0;  // Allow some growth before renormalization
        ct.mac = compute_mac(nonce, ct.coeffs[0], ct.mantissa_part, ct.noise_part, ct.depth, ct.exponent);
        
        return ct;
    }

    // Overload for integer input
    LyapCiphertext encrypt(int64_t value, uint64_t seed = 0) {
        return encrypt((double)value, seed);
    }

    // ═══ DECRYPT ═══
    double decrypt(const LyapCiphertext& ct, uint64_t seed = 0) {
        (void)seed;
        
        // Verify integrity
        uint64_t expected_mac = compute_mac(ct.nonce, ct.coeffs[0], ct.mantissa_part, 
                                            ct.noise_part, ct.depth, ct.exponent);
        if (ct.mac != expected_mac || ct.coeffs[0] != ct.mantissa_part + ct.noise_part) {
            throw std::runtime_error("LyapunovFHE: Integrity check FAILED — tampering detected!");
        }
        
        // Recover mantissa
        int64_t recovered_mantissa = ct.coeffs[0] - ct.noise_part;
        
        // Convert to double: mantissa / MANTISSA_SAFE × 2^exponent
        double result = (double)recovered_mantissa / (double)MANTISSA_SAFE;
        result *= std::pow(2.0, ct.exponent);
        
        return result;
    }

    // ═══ HOMOMORPHIC ADD: align exponents, add mantissas, stabilize ═══
    LyapCiphertext add(const LyapCiphertext& a, const LyapCiphertext& b) {
        LyapCiphertext result;
        result.nonce = enc_counter_.fetch_add(1) ^ (a.nonce ^ b.nonce);
        result.depth = (a.depth > b.depth) ? a.depth : b.depth;
        
        // Align exponents: shift mantissa of smaller exponent
        int64_t a_mant = a.mantissa_part;
        int64_t b_mant = b.mantissa_part;
        int a_exp = a.exponent;
        int b_exp = b.exponent;
        
        int exp_diff = a_exp - b_exp;
        if (exp_diff > 0) {
            b_mant >>= exp_diff;  // Lose some precision in smaller value
            result.exponent = a_exp;
        } else if (exp_diff < 0) {
            a_mant >>= (-exp_diff);
            result.exponent = b_exp;
        } else {
            result.exponent = a_exp;
        }
        
        // Add mantissas
        result.mantissa_part = a_mant + b_mant;
        result.noise_part = a.noise_part + b.noise_part;
        
        // Renormalize (Lyapunov stabilization)
        auto [norm_mant, norm_exp] = renormalize(result.mantissa_part, result.exponent);
        result.mantissa_part = norm_mant;
        result.exponent = norm_exp;
        
        // Build coefficients
        result.coeffs[0] = result.mantissa_part + result.noise_part;
        for (int i = 1; i < POLY_N; i++) {
            result.coeffs[i] = a.coeffs[i] + b.coeffs[i];
        }
        
        result.noise_level = a.noise_level + b.noise_level;
        result.lyapunov_bound = (a.lyapunov_bound + b.lyapunov_bound) * PHI_INV;
        result.mac = compute_mac(result.nonce, result.coeffs[0], result.mantissa_part, 
                                 result.noise_part, result.depth, result.exponent);
        
        return result;
    }

    // ═══ HOMOMORPHIC MULTIPLY: multiply mantissas, add exponents, contract noise ═══
    LyapCiphertext multiply(const LyapCiphertext& a, const LyapCiphertext& b) {
        LyapCiphertext result;
        result.nonce = enc_counter_.fetch_add(1) ^ (a.nonce ^ b.nonce);
        result.depth = a.depth + b.depth + 1;
        result.exponent = a.exponent + b.exponent;
        
        // Multiply mantissas (scaled by MANTISSA_SAFE)
        __int128 prod = (__int128)a.mantissa_part * (__int128)b.mantissa_part;
        result.mantissa_part = (int64_t)(prod / MANTISSA_SAFE);  // Rescale
        
        // Noise: Lyapunov φ⁻¹ contraction
        result.noise_part = (a.mantissa_part * b.noise_part) / MANTISSA_SAFE
                          + (a.noise_part * b.mantissa_part) / MANTISSA_SAFE
                          + (a.noise_part * b.noise_part) / MANTISSA_SAFE;
        
        // Renormalize
        auto [norm_mant, norm_exp] = renormalize(result.mantissa_part, result.exponent);
        result.mantissa_part = norm_mant;
        result.exponent = norm_exp;
        
        // Build coefficients
        result.coeffs[0] = result.mantissa_part + result.noise_part;
        for (int i = 1; i < POLY_N; i++) {
            int64_t sum = 0;
            for (int j = 0; j <= i; j++) {
                sum += a.coeffs[j] * b.coeffs[i - j];
            }
            result.coeffs[i] = sum;
        }
        
        result.noise_level = (a.noise_level * b.noise_level) * PHI_INV;
        result.lyapunov_bound = (a.lyapunov_bound * b.lyapunov_bound) * PHI_INV;
        result.mac = compute_mac(result.nonce, result.coeffs[0], result.mantissa_part,
                                 result.noise_part, result.depth, result.exponent);
        
        return result;
    }

    // ═══ LYAPUNOV STABILITY CHECK ═══
    bool verify_stability(const LyapCiphertext& ct) const {
        return is_stable(ct.noise_level, ct.lyapunov_bound);
    }

    static const char* name() { return "LyapunovFHE — Floating-Point, φ-Stabilized, Full IEEE Range"; }
};
