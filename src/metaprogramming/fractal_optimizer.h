#pragma once
#include "../core/constants.h"
#include "../config/system_config.h"
#include <type_traits>

// ═══════════════════════════════════════════════════════════════════════════════
// FRACTAL OPTIMIZER — Compile-Time Auto-Tuning Metaprogramming
// ═══════════════════════════════════════════════════════════════════════════════

template<int KS_MILLI>
struct KS_Target {
    static constexpr int value = KS_MILLI;
    static constexpr double as_double = KS_MILLI / 1000.0;
};

template<int RING_DIM>
struct RingDim {
    static constexpr int value = RING_DIM;
    static_assert(RING_DIM >= 1024 && RING_DIM <= 65536, "RingDim must be 1024-65536");
    static_assert((RING_DIM & (RING_DIM - 1)) == 0, "RingDim must be power of 2");
};

template<int VARIANTS>
struct Variants {
    static constexpr int value = VARIANTS;
    static_assert(VARIANTS >= 2 && VARIANTS <= 34, "Variants must be 2-34");
};

template<int M>
struct Mode {
    static constexpr int value = M;
    static_assert(M >= 0 && M <= 3, "Mode must be 0-3");
};

// Default sentinel (no static_assert)
template<>
struct RingDim<0> {
    static constexpr int value = 0;  // "Use auto"
};

template<>
struct Variants<0> {
    static constexpr int value = 0;  // "Use auto"
};

// ═══════════════════════════════════════════════════════════════
// COMPILE-TIME FIBONACCI
// ═══════════════════════════════════════════════════════════════
template<int N>
struct Fibonacci {
    static constexpr int value = Fibonacci<N-1>::value + Fibonacci<N-2>::value;
};
template<> struct Fibonacci<0> { static constexpr int value = 1; };
template<> struct Fibonacci<1> { static constexpr int value = 2; };

// Compile-time Fibonacci sum (recursive template, not loop)
template<int N>
struct FibonacciSum {
    static constexpr int value = Fibonacci<N>::value + FibonacciSum<N-1>::value;
};
template<> struct FibonacciSum<-1> { static constexpr int value = 0; };

// ═══════════════════════════════════════════════════════════════
// AUTO-TUNE TEMPLATES
// ═══════════════════════════════════════════════════════════════
template<int KS_MILLI>
struct AutoLayers {
    static constexpr int value = 
        KS_MILLI == 0 ? 34 :
        KS_MILLI <= 1 ? 23 :
        KS_MILLI <= 5 ? 13 :
        KS_MILLI <= 10 ? 8 :
        KS_MILLI <= 50 ? 5 : 3;
};

template<int KS_MILLI>
struct AutoChaosBase {
    static constexpr double value = 
        KS_MILLI <= 1 ? 3.70 :
        KS_MILLI <= 5 ? 3.75 :
        KS_MILLI <= 10 ? 3.80 :
        KS_MILLI <= 50 ? 3.90 : 3.99;
};

template<int KS_MILLI>
struct AutoRefreshWeight {
    static constexpr double value = PHI / (PHI + 1.0) + (KS_MILLI * 0.0001);
};

template<int MODE>
struct AutoRingDim {
    static constexpr int value = 
        MODE == 0 ? 2048 : MODE == 1 ? 4096 : MODE == 2 ? 32768 : 65536;
};

template<int MODE>
struct AutoCKKSDepth {
    static constexpr int value = 
        MODE == 0 ? 60 : MODE == 1 ? 120 : MODE == 2 ? 200 : 300;
};

template<int MODE>
struct AutoVariants {
    static constexpr int value = 
        MODE == 0 ? 3 : MODE == 1 ? 5 : MODE == 2 ? 12 : 23;
};

template<int MODE>
struct AutoStatsSamples {
    static constexpr int value = 
        MODE == 0 ? 10 : MODE == 1 ? 50 : MODE == 2 ? 100 : 1000;
};

template<int MODE>
struct AutoFNELayers {
    static constexpr int value = 
        MODE == 0 ? 1 : MODE == 1 ? 3 : MODE == 2 ? 5 : 13;
};

// ═══════════════════════════════════════════════════════════════
// MASTER OPTIMIZER
// ═══════════════════════════════════════════════════════════════
template<typename KS_T, typename M, typename RD = RingDim<0>, typename V = Variants<0>>
struct FractalOptimizer {
    static constexpr int ks_milli = KS_T::value;
    static constexpr int mode = M::value;
    static constexpr int ring_dim = (RD::value > 0) ? RD::value : AutoRingDim<mode>::value;
    static constexpr int variants = (V::value > 0) ? V::value : AutoVariants<mode>::value;
    
    static constexpr int N_fractal_layers = AutoLayers<ks_milli>::value;
    static constexpr int N_fractal_depth = (N_fractal_layers >= 23) ? 7 : (N_fractal_layers >= 13) ? 5 : 3;
    static constexpr double N_chaos_base = AutoChaosBase<ks_milli>::value;
    static constexpr double N_refresh_weight = AutoRefreshWeight<ks_milli>::value;
    static constexpr int N_ckks_depth = AutoCKKSDepth<mode>::value;
    static constexpr int N_fne_layers = AutoFNELayers<mode>::value;
    static constexpr int N_stats_samples = AutoStatsSamples<mode>::value;
    static constexpr int N_chaos_iterations = (N_fractal_depth >= 7) ? 7 : (N_fractal_depth >= 5) ? 5 : 3;
    static constexpr int N_pairs = variants * (variants - 1) / 2;
    
    // Compile-time Fibonacci sum using recursive template
    static constexpr int N_total_gates = FibonacciSum<variants - 1>::value;
    
    // Compile-time master seed
    static constexpr double master_seed = []() {
        double s = variants * PHI + N_fractal_layers * PSI + N_fractal_depth * PHI * PSI;
        return (s > 1.0) ? s - (int)s : (s < 0.0) ? s + 1.0 - (int)(s + 1.0) : s;
    }();
    
    static void apply(SystemConfig& cfg) {
        cfg.N_ring_dim = ring_dim;
        cfg.N_ckks_depth = N_ckks_depth;
        cfg.N_fne_layers = N_fne_layers;
        cfg.N_variants = variants;
        cfg.N_fractal_layers = N_fractal_layers;
        cfg.N_fractal_depth = N_fractal_depth;
        cfg.N_chaos_base = N_chaos_base;
        cfg.N_chaos_iterations = N_chaos_iterations;
        cfg.N_refresh_weight = N_refresh_weight;
        cfg.N_stats_samples = N_stats_samples;
        cfg.master_seed = master_seed;
        cfg.derive_all();
    }
    
    static constexpr const char* mode_string() {
        return mode == 0 ? "DEV" : mode == 1 ? "TEST" : mode == 2 ? "PROD" : "ENTERPRISE";
    }
};

// ═══════════════════════════════════════════════════════════════
// SHORTHAND
// ═══════════════════════════════════════════════════════════════
template<int KS = 5, int M = 1, int RD = 0, int V = 0>
using FractalConfig = FractalOptimizer<KS_Target<KS>, Mode<M>, RingDim<RD>, Variants<V>>;

using DevConfig    = FractalConfig<50, 0>;
using TestConfig   = FractalConfig<5, 1>;
using ProdConfig   = FractalConfig<5, 2>;
using EnterpriseConfig = FractalConfig<0, 3>;
using UltraConfig  = FractalConfig<0, 3, 65536, 34>;

// ═══════════════════════════════════════════════════════════════
// COMPILE-TIME VERIFICATION
// ═══════════════════════════════════════════════════════════════
static_assert(FractalConfig<5, 1>::N_fractal_layers >= 5, "Minimum 5 fractal layers");
static_assert(FractalConfig<0, 3>::N_fractal_layers >= 23, "Enterprise needs >=23 layers");
static_assert(FractalConfig<5, 2>::ring_dim >= 4096, "Production needs >=4096 RingDim");
