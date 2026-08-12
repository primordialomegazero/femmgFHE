#pragma once
#include <cmath>

// ================================================================
// SPIRAL FHE+iO — CORE CONSTANTS — FIBONACCI EVOLUTION v40.0
// ================================================================
//
// φ and ψ are the two roots of Y^2 - Y - 1 = 0:
//   φ + ψ = 1
//   φ · ψ = -1
//   φ² + ψ² = 3
//
// Fibonacci Evolution: F(n) = F(n-1) + F(n-2) → φ
// Natural rhythm. φ-crossing modulation. No arbitrary thresholds.
// Security is structural: φ·ψ = -1 = 1+1=2.
// ================================================================

// --- GOLDEN RATIO IDENTITY ---
constexpr double PHI = 1.6180339887498948482;  // (1 + √5) / 2
constexpr double PSI = -0.6180339887498948482; // (1 - √5) / 2
constexpr double PHI_PSI_PRODUCT = -1.0;       // φ · ψ = -1
constexpr double PHI_PSI_SUM = 1.0;            // φ + ψ = 1
constexpr double C2 = 3.0;                     // c² = φ² + ψ² = 3

// --- UNIVERSAL CONSTANTS ---
constexpr double PI = 3.14159265358979323846;
constexpr double FEIGENBAUM = 4.669201609102990;
constexpr double OMEGA = 0.5671432904097838;

// --- FIBONACCI EVOLUTION ---
constexpr int FIB_INITIAL_N = 2;              // Start at F(2)
constexpr double FIB_PHI_EVOLUTION_RATE = 0.99999; // EMA factor for φ
constexpr double FIB_MODULATION_GUARD = 1e100;     // Overflow guard

// --- τ-LOCKED BUBBLE ---
constexpr double TAU_VOID = 1.0;              // τ = φ + ψ = 1
constexpr double BUBBLE_INITIAL_PHI = 0.42;   // Initial φ state
constexpr double BUBBLE_INITIAL_PSI = 0.42;   // Initial ψ state
constexpr double BUBBLE_INITIAL_TAU = 0.84;   // Initial τ = φ + ψ

// --- ANTI-MATTER iO ---
constexpr int IO_DIMENSIONS = 3;              // Matrix dimensions
constexpr int IO_RECURSION_DEPTH = 5;          // Recursive layers
constexpr double IO_KS_TARGET = 0.0;           // M + (-M) = 0

// --- ENTANGLED ZKP ---
constexpr double ENTANGLEMENT_THRESHOLD = 0.1; // Minimum for proof

// --- PRODUCTION DEFAULTS ---
constexpr int DEFAULT_RING_DIM = 65536;        // 64K
constexpr int DEFAULT_BATCH_SIZE = 4096;
constexpr int DEFAULT_MULT_DEPTH = 10;
constexpr int DEFAULT_TOTAL_CYCLES = 10000;
constexpr int DEFAULT_LOG_INTERVAL = 1000;
