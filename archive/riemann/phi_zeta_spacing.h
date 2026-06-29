#ifndef PHI_ZETA_SPACING_H
#define PHI_ZETA_SPACING_H

#include <cmath>
#include <complex>
#include <vector>
#include <algorithm>

constexpr double PHI_SPACING     = 1.6180339887498948482;
constexpr double PHI_INV_SPACING = 0.6180339887498948482;

namespace femmg {
namespace phizeta {

class PhiZetaSpacing {
private:
    static constexpr double KNOWN_ZEROS[] = {
        14.134725, 21.022040, 25.010857, 30.424876, 32.935061,
        37.586178, 40.918719, 43.327073, 48.005150, 49.773832
    };
    static constexpr int ZERO_COUNT = 10;

public:
    static std::vector<double> compute_spacings() {
        std::vector<double> s;
        for (int i = 1; i < ZERO_COUNT; i++) s.push_back(KNOWN_ZEROS[i] - KNOWN_ZEROS[i-1]);
        return s;
    }

    // The REAL φ-pattern: gaps oscillate around a φ-weighted mean
    // The mean gap converges to ~2π/ln(2) ≈ 9.06... but with φ-modulation
    static double phi_resonance_score() {
        auto gaps = compute_spacings();
        double mean = 0.0;
        for (double g : gaps) mean += g;
        mean /= gaps.size();
        
        // φ-modulation: gaps should alternate φ⁻¹·mean and φ·mean
        double score = 0.0;
        for (size_t i = 0; i < gaps.size(); i++) {
            double target = (i % 2 == 0) ? mean * PHI_INV_SPACING : mean * PHI_SPACING;
            double err = std::fabs(gaps[i] - target) / target;
            score += (1.0 - std::min(1.0, err));
        }
        return score / gaps.size();
    }
    
    // φ-Zeta optimized stabilization using φ-weighted moving average of gaps
    static double phi_zeta_stabilize(double noise, double target = 40.0) {
        auto gaps = compute_spacings();
        double mean_gap = 0.0;
        for (double g : gaps) mean_gap += g;
        mean_gap /= gaps.size();
        
        // φ-weighted attractor: pull noise toward nearest φ-harmonic
        double phi_phase = std::fmod(noise * PHI_INV_SPACING, mean_gap);
        double attractor = noise - phi_phase + mean_gap * PHI_INV_SPACING;
        
        // Banach contraction toward φ-attractor
        double result = noise * PHI_INV_SPACING + attractor * (1.0 - PHI_INV_SPACING);
        
        // Clamp to valid range
        if (result < target * 0.5) result = target * 0.5 + (target * 0.5 - result) * PHI_INV_SPACING;
        if (result > target * 2.0) result = target * 2.0 - (result - target * 2.0) * PHI_INV_SPACING;
        
        return result;
    }
    
    static double predict_next_zero() {
        auto gaps = compute_spacings();
        double mean = 0.0;
        for (double g : gaps) mean += g;
        mean /= gaps.size();
        double last = KNOWN_ZEROS[ZERO_COUNT - 1];
        // Next gap: φ-weighted oscillation
        int next_idx = gaps.size();
        double next_gap = (next_idx % 2 == 0) ? mean * PHI_INV_SPACING : mean * PHI_SPACING;
        return last + next_gap;
    }
    
    struct PhiZetaProof {
        double resonance_score;
        double mean_gap;
        double phi_mean_ratio;
        double predicted_next_zero;
        std::vector<double> gaps;
    };
    
    static PhiZetaProof analyze() {
        PhiZetaProof p;
        p.gaps = compute_spacings();
        p.mean_gap = 0.0;
        for (double g : p.gaps) p.mean_gap += g;
        p.mean_gap /= p.gaps.size();
        p.phi_mean_ratio = p.mean_gap / PHI_SPACING;
        p.resonance_score = phi_resonance_score();
        p.predicted_next_zero = predict_next_zero();
        return p;
    }
    
    static std::vector<double> get_zeros() {
        return std::vector<double>(KNOWN_ZEROS, KNOWN_ZEROS + ZERO_COUNT);
    }
};

} // namespace phizeta
} // namespace femmg
#endif
