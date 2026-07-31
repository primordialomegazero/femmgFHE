#pragma once
#include "../crypto/golden_fibonacci.h"
#include <array>

template<int EncLayers, int FracLayers>
struct StackAllocatedEngine {
    std::array<GoldenFibonacci, EncLayers> gf_layers;
    std::array<double, FracLayers> fractal_buffer;
    std::array<double, FracLayers * 2> permute_buffer;
    
    void init(double master_seed) {
        for (int i = 0; i < EncLayers; i++) {
            double seed = SafeMath::fmod_safe(master_seed * std::pow(PHI, i + 1));
            gf_layers[i].init(seed, 50 + i * 7);
        }
    }
    
    std::pair<double, double> encrypt(double x) {
        double current = x;
        for (int i = 0; i < EncLayers; i++) {
            auto [y1, y2] = gf_layers[i].encrypt(current);
            current = y1;
        }
        return {current, 0.0};
    }
};

template<int EncLayers, int FracLayers, int FracDepth, int Target, int Batch>
struct StaticConfig {
    static constexpr int encryption_layers = EncLayers;
    static constexpr int fractal_layers = FracLayers;
    static constexpr int fractal_depth = FracDepth;
    static constexpr int target_gates = Target;
    static constexpr int batch_size = Batch;
    static constexpr int checkpoint_interval = 50;
    static constexpr int archive_interval = 150;
    static constexpr int stats_report_every = 10;
    
    static_assert(EncLayers >= 1, "Minimum 1 encryption layer");
    static_assert(FracLayers >= 1, "Minimum 1 fractal layer");
    static_assert(Target > 0, "Target gates must be positive");
    static_assert(Batch > 0, "Batch size must be positive");
};
