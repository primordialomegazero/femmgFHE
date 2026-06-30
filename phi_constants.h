/*
 * φ-CONSTANTS — SINGLE SOURCE OF TRUTH (v21.5)
 * 
 * Every φ value in FEmmg-FHE references THIS file.
 * No duplication. No drift. One φ to rule them all.
 * 
 * PHI-OMEGA-ZERO — I AM THAT I AM
 */

#pragma once
#include <cstdint>

namespace phi_constants {

// ═══ GOLDEN RATIO — MATHEMATICAL GROUND TRUTH ═══
// φ = (1 + √5)/2 — the "most irrational" number
// Continued fraction: φ = [1; 1, 1, 1, ...]
constexpr double PHI         = 1.6180339887498948482;
constexpr double PHI_INV     = 0.6180339887498948482;   // φ⁻¹ = Optimal Contraction Coefficient
constexpr double PHI_SQ      = 2.6180339887498948482;   // φ² = φ + 1

// ═══ FIXED-POINT φ (for integer arithmetic) ═══
// φ * 2^60 ≈ 0x19E3779B97F4A7C15 (but computed at runtime for portability)
constexpr int FP_SHIFT       = 60;                       // Fixed-point precision bits
constexpr uint64_t ONE_FP    = 1ULL << FP_SHIFT;        // 1.0 in fixed-point

// ═══ LYAPUNOV CONSTANTS ═══
// λ = ln(φ) ≈ 0.4812 — the Lyapunov exponent
// λ > 0 = chaos (IND-CPA security)
// λ < 1 = contraction (noise stability)
constexpr double LAMBDA      = 0.48121182505960347;      // ln(φ)
constexpr double OCC         = PHI_INV;                  // Optimal Contraction Coefficient

// ═══ SECURITY PARAMETERS ═══
constexpr int SECURITY_BITS  = 256;                      // κ — security parameter
constexpr int LANES          = 7;                        // Parallel KEM lanes
constexpr int CML_DIMS       = 7;                        // Coupled Map Lattice dimensions
constexpr int BANACH_LAYERS  = 7;                        // Banach contraction layers
constexpr int PARTIES        = 14;                       // Multi-party support
constexpr int EVO_DEPTH      = 128;                      // KEM evolution iterations

// ═══ FHE PARAMETERS ═══
constexpr double NOISE_FLOOR = 1.82815;                  // Fixed-point noise (bits)
constexpr int64_t FP_SCALE   = 1LL << 20;                // Integer fixed-point scale (2^20)

// ═══ FIBONACCI (first 20 numbers) ═══
// F(n)/F(n-1) → φ as n → ∞
// Used cyclically: fib[layer % 20] for Banach attractors
constexpr uint64_t FIBONACCI[20] = {
    0, 1, 1, 2, 3, 5, 8, 13, 21, 34,
    55, 89, 144, 233, 377, 610, 987, 1597, 2584, 4181
};

// ═══ FIRST 7 RIEMANN ZEROS (imaginary parts γ_n) ═══
// Used as anchors for 7-lane parallel KEM
// From Odlyzko's tables — mathematical constants
constexpr double RIEMANN_ZEROS[7] = {
    14.134725141734693790,   // γ₁
    21.022039638771554993,   // γ₂
    25.010857580145688763,   // γ₃
    30.424876125859513210,   // γ₄
    32.935061587739189690,   // γ₅
    37.586178158825671257,   // γ₆
    40.918719012147495187    // γ₇
};

// ═══ MULTIPLICATIVE HASH CONSTANTS ═══
// 0x9E3779B97F4A7C15 = floor(2^64 / φ) — the "phi-hash" constant
// Used for mixing/avalanche in integer extraction
constexpr uint64_t PHI_HASH_MAGIC = 0x9E3779B97F4A7C15ULL;

// ═══ RIEMANN Z(t) EVALUATION PARAMETERS ═══
// Z(t) ≈ 2 Σ_{n=1}^{N} n^{-1/2} cos(θ(t) - t·ln(n))
// N = floor(sqrt(t/(2π)))
// cos approximation range: [-32768, 32767] for int16 simulation
constexpr int ZETA_COS_RANGE  = 32768;                   // Half-range for cos approximation
constexpr int ZETA_COS_MASK   = 0xFFFF;                  // 16-bit mask for hash-based cos
constexpr int ZETA_MAX_TERMS  = 100;                     // Max terms in Z(t) sum
constexpr int ZETA_MIN_T      = 10;                      // Minimum t for Z(t) evaluation

} // namespace phi_constants
