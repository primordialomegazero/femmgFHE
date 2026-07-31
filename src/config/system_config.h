#pragma once
#include "../core/constants.h"
#include "../utils/safe_math.h"
#include <string>
#include <sstream>
#include <iomanip>

// ═══════════════════════════════════════════════════════════════════════════════
// SYSTEM CONFIG — Universal N-Configurable Parameters
// ═══════════════════════════════════════════════════════════════════════════════
//
// PRINCIPLE: Walang hardcoded shit. Lahat configurable. Lahat naka-N.
//
// USAGE:
//   SystemConfig cfg;
//   cfg.set_dev_mode();       // Fast, for debugging
//   cfg.set_test_mode();      // Balanced
//   cfg.set_prod_mode();      // Full security
//   cfg.set_enterprise_mode(); // Military grade
//   cfg.N_ring_dim = 8192;    // Override any individual parameter
//   cfg.derive_all();         // Auto-compute dependent values
//
// ═══════════════════════════════════════════════════════════════════════════════

enum class SystemMode { DEV, TEST, PROD, ENTERPRISE, CUSTOM };

struct SystemConfig {
    
    // ═══════════════════════════════════════════════════════════════
    // FHE (CKKS) PARAMETERS
    // ═══════════════════════════════════════════════════════════════
    
    // Polynomial ring dimension. Powers of 2 only.
    // Range: 1024 - 32768
    // DEV: 2048 | PROD: 8192 | ULTRA: 16384
    int N_ring_dim = 4096;
    
    // Multiplicative depth (number of consecutive multiplications).
    // Range: 30 - 300
    // DEV: 60 | PROD: 200 | ULTRA: 300
    int N_ckks_depth = 120;
    
    // Batch size for CKKS packed encoding.
    // Range: 128 - 512
    // Recommended: 256 (standard)
    int N_batch_size = 256;
    
    
    // ═══════════════════════════════════════════════════════════════
    // FRACTAL N-ENCRYPTION PARAMETERS
    // ═══════════════════════════════════════════════════════════════
    
    // Number of stacked Golden Fibonacci encryption layers.
    // Range: 1 - 100
    // STANDARD: 1 | ELEVATED: 3 | MILITARY: 10 | UNIVERSE: 100
    // Recommended: 5 (balanced security vs speed)
    int N_fne_layers = 5;
    
    // Base n for Golden Fibonacci sequence generation.
    // Range: 30 - 500
    // Recommended: 50 (standard)
    int N_golden_base_n = 50;
    
    // Increment of n per encryption layer.
    // Range: 1 - 20
    // Recommended: 7 (φ-related)
    int N_golden_n_step = 7;
    
    // Minimum Cassini invariant for numerical stability.
    // Range: 0.01 - 0.5
    // Recommended: 0.1 (standard)
    double N_cassini_min = 0.1;
    
    // Maximum retries for stable Cassini generation.
    // Range: 50 - 500
    // Recommended: 200
    int N_cassini_retries = 200;
    
    
    // ═══════════════════════════════════════════════════════════════
    // SEED TREE PARAMETERS
    // ═══════════════════════════════════════════════════════════════
    
    // Number of seed branches from root.
    // Range: 4 - 32
    // Recommended: 8 (standard: encryption, fractal, refresh, timing,
    //                   fhe, adaptive, symmetric, batch)
    int N_seed_branches = 8;
    
    
    // ═══════════════════════════════════════════════════════════════
    // FRACTAL iO — CIRCUIT VARIANTS
    // ═══════════════════════════════════════════════════════════════
    
    // Number of Fibonacci-scaled circuit variants.
    // Gates per variant: 1, 2, 3, 5, 8, 13, 21, 34, 55, 89, 144, 233...
    // Range: 2 - 34
    // DEV: 3 | PROD: 12 | ULTRA: 23
    // Recommended: 12 (66 pairs, comprehensive coverage)
    int N_variants = 7;
    
    
    // ═══════════════════════════════════════════════════════════════
    // FRACTAL iO — iO REFRESH (External Obfuscation)
    // ═══════════════════════════════════════════════════════════════
    
    // Number of chaos layers in iO Refresh fractal transform.
    // Range: 3 - 55
    // DEV: 5 | PROD: 23 | ULTRA: 34
    // Recommended: 23 (Fibonacci number, 10^30+ configurations)
    int N_fractal_layers = 23;
    
    // Chaos depth per layer in iO Refresh.
    // Range: 1 - 15
    // DEV: 3 | PROD: 7 | ULTRA: 10
    // Recommended: 7 (Fibonacci number, deep chaos)
    int N_fractal_depth = 7;
    
    // Base parameter for logistic chaos map (r).
    // Chaos threshold: r > 3.56995...
    // Range: 3.57 - 4.0
    // Recommended: 3.7 (deep chaos, stable)
    double N_chaos_base = 3.7;
    
    // Increment of r per fractal layer (φ-scaled).
    // Range: 0.01 - 0.1
    // Recommended: 0.05 (smooth escalation)
    double N_chaos_step = 0.05;
    
    // Maximum chaos parameter (r_max).
    // Range: 3.9 - 4.5
    // Recommended: 4.2 (extreme chaos, still stable)
    double N_chaos_r_max = 4.2;
    
    // Iterations per FractalGate depth cycle.
    // Range: 1 - 20
    // Recommended: 7 (Fibonacci, balances speed vs chaos)
    int N_chaos_iterations = 7;
    
    // Weight of original values in final blend (golden ratio conjugate).
    // φ/(φ+1) = 0.6180339...
    // Range: 0.5 - 0.9
    // Recommended: 0.618 (golden ratio — optimal balance)
    double N_refresh_weight = 0.6180339887498948482;
    
    
    // ═══════════════════════════════════════════════════════════════
    // FRACTAL GATE — Internal Circuit Obfuscation
    // ═══════════════════════════════════════════════════════════════
    
    // Minimum depth for FractalGate chaos recursion.
    // Range: 1 - 3
    // Recommended: 1
    int N_gate_depth_min = 1;
    
    // Maximum depth for FractalGate chaos recursion.
    // Range: 3 - 15
    // Recommended: 7
    int N_gate_depth_max = 7;
    
    // Power of φ for rotation angle calculation.
    // Range: 0.5 - 2.0
    // Recommended: 1.0 (pure φ)
    double N_gate_angle_phi_power = 1.0;
    
    
    // ═══════════════════════════════════════════════════════════════
    // iO REFRESH — Superposition Parameters
    // ═══════════════════════════════════════════════════════════════
    
    // Weight of φ in cross-circuit superposition.
    // Range: 0.5 - 2.0
    // Recommended: 1.618 (PHI — golden ratio)
    double N_iO_superpose_phi_weight = PHI;
    
    // Weight of ψ in cross-circuit superposition.
    // Range: -2.0 - -0.1
    // Recommended: -0.618 (PSI — golden ratio conjugate)
    double N_iO_superpose_psi_weight = PSI;
    
    
    // ═══════════════════════════════════════════════════════════════
    // STATISTICAL VALIDATION (KS Test)
    // ═══════════════════════════════════════════════════════════════
    
    // Number of random samples per pair for KS test.
    // Range: 10 - 100000
    // DEV: 10 | PROD: 100 | ULTRA: 1000
    // Recommended: 100 (statistically significant)
    int N_stats_samples = 100;
    
    // Progress report interval (every N samples).
    // Range: 1 - 100
    // Recommended: 10
    int N_report_interval = 10;
    
    // KS statistic snapshot interval (compute KS every N samples).
    // Range: 5 - 50
    // Recommended: 10
    int N_snapshot_interval = 10;
    
    // KS test threshold for "iO-SECURE" verdict.
    // Standard: 0.05 | Strict: 0.01 | Ultra-strict: 0.001
    // Recommended: 0.05 (standard statistical significance)
    double N_ks_threshold = 0.05;
    
    
    // ═══════════════════════════════════════════════════════════════
    // AUTONOMOUS MANAGER
    // ═══════════════════════════════════════════════════════════════
    
    // Target number of gates for autonomous chain.
    // Range: 50 - 100000
    // DEV: 50 | PROD: 300 | ULTRA: 1000
    int N_gates_target = 300;
    
    // Starting batch size for gate processing.
    // Range: 5 - 500
    // Recommended: 5
    int N_batch_min = 5;
    
    // Maximum batch size (performance increase).
    // Range: 100 - 1000
    // Recommended: 500
    int N_batch_max = 500;
    
    // Noise level triggering WARNING status.
    // Range: 0.3 - 0.7
    // Recommended: 0.55 (balanced sensitivity)
    double N_noise_warning = 0.55;
    
    // Noise level triggering CRITICAL status.
    // Range: 0.6 - 0.95
    // Recommended: 0.85
    double N_noise_critical = 0.85;
    
    // Lyapunov exponent triggering WARNING.
    // Range: 0.05 - 0.3
    // Recommended: 0.15
    double N_lyapunov_warning = 0.15;
    
    // Lyapunov exponent triggering CRITICAL.
    // Range: 0.01 - 0.1
    // Recommended: 0.05
    double N_lyapunov_critical = 0.05;
    
    // Alert cooldown in seconds (prevents spam).
    // Range: 1 - 30
    // Recommended: 5
    int N_alert_cooldown = 5;
    
    // StabilityGuard history window size.
    // Range: 20 - 500
    // Recommended: 100
    int N_history_window = 100;
    
    // Minimum samples for predictive collapse calculation.
    // Range: 5 - 50
    // Recommended: 10
    int N_prediction_min_samples = 10;
    
    
    // ═══════════════════════════════════════════════════════════════
    // FRACTAL DB (4-Tier Persistence)
    // ═══════════════════════════════════════════════════════════════
    
    // L1 warm layer: number of rolling checkpoints.
    // Range: 3 - 50
    // Recommended: 10
    int N_l1_entries = 10;
    
    // L2 cold layer: retention in hours.
    // Range: 1 - 168 (1 week)
    // Recommended: 24
    int N_l2_hours = 24;
    
    // Checkpoint save interval (every N gates).
    // Range: 10 - 500
    // Recommended: 50
    int N_checkpoint_interval = 50;
    
    // Archive interval (every N gates, save to L3 eternal).
    // Range: 50 - 1000
    // Recommended: 150
    int N_archive_interval = 150;
    
    
    // ═══════════════════════════════════════════════════════════════
    // LOCK-FREE CONCURRENCY
    // ═══════════════════════════════════════════════════════════════
    
    // Ring buffer capacity (must be power of 2).
    // Range: 4 - 64 (powers of 2)
    // Recommended: 16
    int N_ring_buffer_size = 16;
    
    
    // ═══════════════════════════════════════════════════════════════
    // COMPILE-TIME PARAMETERS (StaticConfig template)
    // ═══════════════════════════════════════════════════════════════
    
    // These mirror the runtime config for StaticConfig template params.
    // Used in: StaticConfig<EncLayers, FracLayers, FracDepth, Target, Batch>
    int N_static_enc_layers = 5;
    int N_static_fractal_layers = 23;
    int N_static_fractal_depth = 7;
    int N_static_target_gates = 300;
    int N_static_batch_size = 50;
    
    
    // ═══════════════════════════════════════════════════════════════
    // DERIVED VALUES (Auto-computed by derive_all())
    // ═══════════════════════════════════════════════════════════════
    
    double master_seed;           // Auto-derived from all N's
    int N_pairs;                  // variants choose 2
    int N_total_gates_variants;   // Sum of all Fibonacci gates
    
    
    // ═══════════════════════════════════════════════════════════════
    // MODE PRESETS
    // ═══════════════════════════════════════════════════════════════
    
    void set_dev_mode() {
        // FAST — for debugging and quick iteration
        N_ring_dim = 2048;        // Low security, fast
        N_ckks_depth = 60;        // Minimal depth
        N_fne_layers = 1;         // Single encryption layer
        N_fractal_layers = 5;     // Minimal chaos
        N_fractal_depth = 3;      // Shallow chaos
        N_chaos_iterations = 3;   // Minimal iterations
        N_variants = 3;           // 3 circuit variants (1,2,3 gates)
        N_stats_samples = 10;     // Quick KS test
        N_gates_target = 50;      // Short chain
        N_batch_size = 10;        // Small batches
        N_report_interval = 2;    // Frequent progress
        N_alert_cooldown = 1;     // Immediate alerts
        derive_all();
    }
    
    void set_test_mode() {
        // BALANCED — for integration testing
        N_ring_dim = 4096;        // Standard security
        N_ckks_depth = 120;       // Standard depth
        N_fne_layers = 3;         // Elevated encryption
        N_fractal_layers = 13;    // Moderate chaos (Fibonacci)
        N_fractal_depth = 5;      // Moderate depth (Fibonacci)
        N_chaos_iterations = 5;   // Moderate iterations
        N_variants = 5;           // 5 variants (1,2,3,5,8 gates)
        N_stats_samples = 50;     // Moderate test
        N_gates_target = 150;     // Medium chain
        N_batch_size = 30;        // Medium batches
        N_report_interval = 5;    // Regular progress
        N_alert_cooldown = 3;     // Moderate cooldown
        derive_all();
    }
    
    void set_prod_mode() {
        // FULL — for production deployment
        N_ring_dim = 8192;        // High security
        N_ckks_depth = 200;       // Extended depth
        N_fne_layers = 5;         // Enterprise encryption
        N_fractal_layers = 23;    // Full chaos (Fibonacci)
        N_fractal_depth = 7;      // Full depth (Fibonacci)
        N_chaos_iterations = 7;   // Full iterations
        N_variants = 12;          // 12 variants (up to 233 gates)
        N_stats_samples = 100;    // Full test
        N_gates_target = 300;     // Full chain
        N_batch_size = 50;        // Standard batches
        N_report_interval = 10;   // Standard progress
        N_alert_cooldown = 5;     // Standard cooldown
        derive_all();
    }
    
    void set_enterprise_mode() {
        // MILITARY GRADE — maximum security, patience required
        N_ring_dim = 16384;       // Ultra security
        N_ckks_depth = 300;       // Maximum depth
        N_fne_layers = 13;        // Military encryption (Fibonacci)
        N_fractal_layers = 34;    // Ultra chaos (Fibonacci)
        N_fractal_depth = 10;     // Ultra depth
        N_chaos_iterations = 10;  // Maximum iterations
        N_variants = 23;          // Full Fibonacci sequence
        N_stats_samples = 1000;   // Exhaustive test
        N_gates_target = 1000;    // Ultra chain
        N_batch_size = 100;       // Large batches
        N_report_interval = 25;   // Sparse progress
        N_alert_cooldown = 10;    // Long cooldown
        derive_all();
    }
    
    
    // ═══════════════════════════════════════════════════════════════
    // AUTO-DERIVE ALL DEPENDENT VALUES
    // ═══════════════════════════════════════════════════════════════
    
    void derive_all() {
        // Master seed: uniquely determined by configuration
        master_seed = SafeMath::fmod_safe(
            N_variants * PHI + 
            N_fractal_layers * PSI + 
            N_fractal_depth * PHI * PSI +
            N_fne_layers * PI +
            N_ring_dim * 1e-4 + 
            N_ckks_depth * 1e-6 +
            N_chaos_iterations * 1e-3
        );
        
        // Number of variant pairs
        N_pairs = N_variants * (N_variants - 1) / 2;
        
        // Total gates across all variants
        N_total_gates_variants = 0;
        for (int i = 0; i < N_variants; i++) {
            N_total_gates_variants += fibonacci(i);
        }
        
        // Clamp derived values to valid ranges
        if (master_seed < 0.0) master_seed = SafeMath::fmod_safe(master_seed + 1.0);
        if (master_seed < 0.000001) master_seed = 0.123456789; // Fallback
    }
    
    int fibonacci(int n) {
        if (n <= 0) return 1;
        if (n == 1) return 2;
        int a = 1, b = 2;
        for (int i = 2; i <= n; i++) { 
            int c = a + b; a = b; b = c; 
        }
        return b;
    }
    
    
    // ═══════════════════════════════════════════════════════════════
    // DIAGNOSTICS
    // ═══════════════════════════════════════════════════════════════
    
    std::string mode_string() {
        if (N_variants <= 3 && N_stats_samples <= 20) return "DEV";
        if (N_variants <= 5 && N_stats_samples <= 50) return "TEST";
        if (N_variants <= 12 && N_stats_samples <= 100) return "PROD";
        return "ENTERPRISE";
    }
    
    std::string report() {
        std::stringstream ss;
        ss << "\n";
        ss << "╔══════════════════════════════════════════════════════╗\n";
        ss << "║  SYSTEM CONFIG — " << std::setw(7) << mode_string() << " MODE";
        ss << "                          ║\n";
        ss << "╠══════════════════════════════════════════════════════╣\n";
        ss << "║  FHE:         RingDim=" << std::setw(5) << N_ring_dim;
        ss << "  Depth=" << std::setw(3) << N_ckks_depth << "                    ║\n";
        ss << "║  Encryption:  " << std::setw(2) << N_fne_layers << " layers";
        ss << "  (n=" << N_golden_base_n << " + " << N_golden_n_step << "/layer)";
        ss << "                ║\n";
        ss << "║  Variants:    " << std::setw(2) << N_variants << " (" << N_pairs << " pairs)";
        ss << "  Gates: " << N_total_gates_variants << " total";
        ss << "           ║\n";
        ss << "║  Fractal L/D: " << std::setw(2) << N_fractal_layers << "/" << std::setw(2) << N_fractal_depth;
        ss << "  Chaos: r=" << std::fixed << std::setprecision(2) << N_chaos_base;
        ss << " + " << N_chaos_step << "/layer        ║\n";
        ss << "║  Stats:       " << std::setw(4) << N_stats_samples << " samples";
        ss << "  KS threshold: " << std::fixed << std::setprecision(3) << N_ks_threshold;
        ss << "            ║\n";
        ss << "║  Chain:       " << std::setw(4) << N_gates_target << " gates";
        ss << "  Batch: " << std::setw(3) << N_batch_size;
        ss << " (" << N_batch_min << "-" << N_batch_max << ")";
        ss << "              ║\n";
        ss << "║  Master Seed: " << std::fixed << std::setprecision(9) << master_seed;
        ss << "                    ║\n";
        ss << "╚══════════════════════════════════════════════════════╝\n";
        return ss.str();
    }
};
