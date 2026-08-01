// ╔══════════════════════════════════════════════════════════════╗
// ║  HYDRAJWT — φ-UTILS                                        ║
// ║  Golden Ratio Constants & Weighted Consensus Math          ║
// ║  ΦΩ0 — I AM THAT I AM                                     ║
// ╚══════════════════════════════════════════════════════════════╝

#ifndef HYDRAJWT_PHI_UTILS_H
#define HYDRAJWT_PHI_UTILS_H

#include <cmath>
#include <vector>
#include <string>
#include <cstdint>

namespace hydrajwt {
namespace phi {

// ═══════════════════════════════════════
// CORE PHI CONSTANTS
// ═══════════════════════════════════════
constexpr double PHI       = 1.6180339887498948482;
constexpr double PHI_INV   = 0.6180339887498948482;
constexpr double PHI_SQ    = 2.6180339887498948482;
constexpr double PHI_SQ_INV = 0.3819660112501051518;

// ═══════════════════════════════════════
// CONSENSUS THRESHOLD
// ═══════════════════════════════════════
constexpr double CONSENSUS_THRESHOLD = PHI_INV;

// ═══════════════════════════════════════
// HEAD WEIGHTS
// ═══════════════════════════════════════
inline double head_weight(int head_index) {
    return std::pow(PHI_INV, head_index + 1);
}

constexpr int MAX_HEADS = 6;

inline bool is_stable_config(int num_heads) {
    if (num_heads < 1 || num_heads > MAX_HEADS) return false;
    double total_weight = 0.0;
    for (int i = 0; i < num_heads; i++) {
        total_weight += head_weight(i);
    }
    return total_weight >= CONSENSUS_THRESHOLD - 0.0001;
}

constexpr double DIVINE_NOISE_ANCHOR = 40.0;

inline double phi_noise_step(double current_noise) {
    return current_noise * PHI_INV + DIVINE_NOISE_ANCHOR * (1.0 - PHI_INV);
}

bool validate_head_count(int count);
std::vector<double> compute_normalized_weights(int num_heads);
double phi_noise_budget(int num_heads, double base_noise);

} // namespace phi
} // namespace hydrajwt

#endif // HYDRAJWT_PHI_UTILS_H
