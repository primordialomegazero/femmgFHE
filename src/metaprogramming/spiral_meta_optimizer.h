#pragma once
#include "../core/constants.h"
#include "../config/system_config.h"
#include <type_traits>

// ═══════════════════════════════════════════════════════════════════════════════
// SPIRAL META-OPTIMIZER — Compile-Time Parameter Auto-Tuning
// ═══════════════════════════════════════════════════════════════════════════════

template<int N>
struct MetaFibonacci {
    static constexpr int value = MetaFibonacci<N-1>::value + MetaFibonacci<N-2>::value;
};
template<> struct MetaFibonacci<0> { static constexpr int value = 1; };
template<> struct MetaFibonacci<1> { static constexpr int value = 2; };

// ═══════════════════════════════════════════════════════════════════
// SPIRAL BOOTSTRAP — Compile-Time Optimal Parameters
// ═══════════════════════════════════════════════════════════════════
template<int RingDim, int SecurityLevel>
struct SpiralMetaConfig {
    static constexpr int N_gf_layers = 
        SecurityLevel == 0 ? 1 : SecurityLevel == 1 ? 3 : SecurityLevel == 2 ? 5 : 13;
    
    static constexpr int N_spiral_rounds = MetaFibonacci<SecurityLevel + 3>::value;
    
    static constexpr int N_spiral_depth = N_spiral_rounds * 3;
    
    static constexpr int N_timing_iterations = MetaFibonacci<SecurityLevel + 2>::value;
    
    static constexpr int N_obfuscation_layers = MetaFibonacci<SecurityLevel + 3>::value;
    
    static constexpr double N_timing_base_delay = 0.00005;
    
    static constexpr double N_timing_chaos_r = 
        SecurityLevel == 0 ? 3.7 : SecurityLevel == 1 ? 3.85 : SecurityLevel == 2 ? 3.99 : 4.0;
    
    static constexpr double N_obfuscation_blend = PHI / (PHI + 1.0);
    
    static constexpr int N_gf_base_n = 50 + (SecurityLevel * 10);
};

// ═══════════════════════════════════════════════════════════════
// FRACTAL REFRESH — Compile-Time Optimal Parameters
// ═══════════════════════════════════════════════════════════════
template<int RingDim, int SecurityLevel>
struct FractalMetaConfig {
    static constexpr int N_fractal_layers = 
        SecurityLevel == 0 ? 5 : SecurityLevel == 1 ? 13 : SecurityLevel == 2 ? 23 : 34;
    
    static constexpr int N_fractal_depth = 
        SecurityLevel == 0 ? 3 : SecurityLevel == 1 ? 5 : SecurityLevel == 2 ? 7 : 10;
    
    static constexpr double N_refresh_weight = PHI / (PHI + 1.0);
    
    static constexpr int N_chaos_iterations = MetaFibonacci<SecurityLevel + 3>::value;
    
    static constexpr double N_chaos_base = 
        SecurityLevel == 0 ? 3.70 : SecurityLevel == 1 ? 3.80 : SecurityLevel == 2 ? 3.90 : 3.99;
    
    static constexpr int N_emerge_noise_gate_min = 
        SecurityLevel == 0 ? 20 : SecurityLevel == 1 ? 15 : SecurityLevel == 2 ? 10 : 5;
    
    static constexpr int N_emerge_safety_gate_max = 
        SecurityLevel == 0 ? 100 : SecurityLevel == 1 ? 50 : SecurityLevel == 2 ? 35 : 20;
};

// ═══════════════════════════════════════════════════════════════
// PRE-MADE CONFIGURATIONS
// ═══════════════════════════════════════════════════════════════

using SpiralDevConfig  = SpiralMetaConfig<2048, 0>;
using SpiralTestConfig = SpiralMetaConfig<4096, 1>;
using SpiralProdConfig = SpiralMetaConfig<32768, 2>;
using SpiralEntConfig  = SpiralMetaConfig<65536, 3>;

using FractalDevConfig  = FractalMetaConfig<2048, 0>;
using FractalTestConfig = FractalMetaConfig<4096, 1>;
using FractalProdConfig = FractalMetaConfig<32768, 2>;
using FractalEntConfig  = FractalMetaConfig<65536, 3>;

// ═══════════════════════════════════════════════════════════════
// COMPILE-TIME VERIFICATION
// ═══════════════════════════════════════════════════════════════
static_assert(SpiralDevConfig::N_spiral_rounds >= 2, "Minimum 2 spiral rounds");
static_assert(SpiralEntConfig::N_spiral_depth >= 30, "Enterprise needs >=30 spiral depth");
static_assert(FractalDevConfig::N_fractal_layers >= 3, "Minimum 3 fractal layers");
static_assert(FractalEntConfig::N_fractal_layers >= 30, "Enterprise needs >=30 fractal layers");
