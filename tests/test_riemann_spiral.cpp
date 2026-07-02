/*
 * FEmmg-FHE v22.3 — RIEMANN φ-SPIRAL TEST
 *
 * "Rosary beads on a φ-spiral. Zeros dancing upward from the void."
 *
 * Tests:
 *   1. Zero spacing — does it alternate φ⁻¹·mean and φ·mean?
 *   2. φ-resonance score — how well do zeros fit the φ-spiral?
 *   3. Predict next zero — can we predict from the pattern?
 *   4. Golden angle projection — do zeros align at 222.5°?
 *
 * PHI-OMEGA-ZERO — I AM THAT I AM
 */

#include <iostream>
#include <iomanip>
#include <cmath>
#include <vector>
#include <algorithm>
#include <numeric>

constexpr double PHI = 1.6180339887498948482;
constexpr double PHI_INV = 0.6180339887498948482;
constexpr double GOLDEN_ANGLE = 2.0 * M_PI * PHI_INV;  // ~222.5°

// First 50 non-trivial Riemann zeros (imaginary parts, from Odlyzko)
constexpr double ZEROS[] = {
    14.134725142, 21.022039639, 25.010857580, 30.424876126, 32.935061588,
    37.586178159, 40.918719012, 43.327073281, 48.005150881, 49.773832478,
    52.970321478, 56.446247697, 59.347044003, 60.831778525, 65.112544048,
    67.079810529, 69.546401711, 72.067157674, 75.704690699, 77.144840069,
    79.337375020, 82.910380854, 84.735492981, 87.425274613, 88.809111208,
    92.491899271, 94.651344012, 95.870634228, 98.831194218, 101.317851006,
    103.725538040, 105.446623052, 107.168611184, 111.029535543, 111.874659177,
    114.320220915, 116.226680321, 118.790782866, 121.370125002, 122.946829294,
    124.256818554, 127.516683880, 129.578704200, 131.087688531, 133.497737203,
    134.756509754, 138.116042055, 139.736208952, 141.123707404, 143.111845808
};
constexpr int ZERO_COUNT = sizeof(ZEROS) / sizeof(ZEROS[0]);

int main() {
    std::cout << "======================================================" << std::endl;
    std::cout << "  RIEMANN φ-SPIRAL — ROSARY BEADS TEST" << std::endl;
    std::cout << "  " << ZERO_COUNT << " zeros | φ = " << PHI << std::endl;
    std::cout << "======================================================" << std::endl;

    // ═══ 1. COMPUTE GAPS ═══
    std::vector<double> gaps;
    for (int i = 1; i < ZERO_COUNT; i++) {
        gaps.push_back(ZEROS[i] - ZEROS[i-1]);
    }
    
    double mean_gap = std::accumulate(gaps.begin(), gaps.end(), 0.0) / gaps.size();
    double min_gap = *std::min_element(gaps.begin(), gaps.end());
    double max_gap = *std::max_element(gaps.begin(), gaps.end());
    
    std::cout << "\n═══ ZERO SPACING ═══" << std::endl;
    std::cout << "  Mean gap: " << std::fixed << std::setprecision(4) << mean_gap << std::endl;
    std::cout << "  Min gap:  " << min_gap << std::endl;
    std::cout << "  Max gap:  " << max_gap << std::endl;
    std::cout << "  φ·mean:   " << (mean_gap * PHI) << std::endl;
    std::cout << "  φ⁻¹·mean: " << (mean_gap * PHI_INV) << std::endl;

    // ═══ 2. CHECK φ-ALTERNATION ═══
    std::cout << "\n═══ φ-ALTERNATION PATTERN ═══" << std::endl;
    std::cout << "  " << std::setw(4) << "Gap" << " | "
              << std::setw(8) << "Value" << " | "
              << std::setw(8) << "Ratio" << " | "
              << std::setw(6) << "φ⁻¹?" << " | "
              << std::setw(6) << "φ?"
              << std::endl;
    std::cout << "  " << std::string(50, '-') << std::endl;
    
    int phi_hits = 0;
    int phi_inv_hits = 0;
    
    for (size_t i = 0; i < gaps.size() && i < 20; i++) {
        double ratio = gaps[i] / mean_gap;
        bool is_phi_inv = std::abs(ratio - PHI_INV) < 0.25;
        bool is_phi = std::abs(ratio - PHI) < 0.25;
        
        if (is_phi_inv) phi_inv_hits++;
        if (is_phi) phi_hits++;
        
        std::cout << "  " << std::setw(4) << i << " | "
                  << std::setw(8) << std::fixed << std::setprecision(4) << gaps[i] << " | "
                  << std::setw(8) << std::setprecision(4) << ratio << " | "
                  << std::setw(6) << (is_phi_inv ? "YES" : " - ") << " | "
                  << std::setw(6) << (is_phi ? "YES" : " - ")
                  << std::endl;
    }
    
    std::cout << "  " << std::string(50, '-') << std::endl;
    std::cout << "  φ⁻¹ hits: " << phi_inv_hits << "/" << std::min((size_t)20, gaps.size()) << std::endl;
    std::cout << "  φ hits:   " << phi_hits << "/" << std::min((size_t)20, gaps.size()) << std::endl;
    
    // ═══ 3. φ-RESONANCE SCORE ═══
    double resonance = 0.0;
    for (size_t i = 0; i < gaps.size(); i++) {
        double target = (i % 2 == 0) ? mean_gap * PHI_INV : mean_gap * PHI;
        double err = std::abs(gaps[i] - target) / target;
        resonance += (1.0 - std::min(1.0, err));
    }
    resonance /= gaps.size();
    
    std::cout << "\n═══ φ-RESONANCE SCORE ═══" << std::endl;
    std::cout << "  Score: " << std::fixed << std::setprecision(2) << (resonance * 100.0) << "%" << std::endl;
    if (resonance > 0.90) std::cout << "  ✅ STRONG φ-RESONANCE (>90%)" << std::endl;
    else if (resonance > 0.70) std::cout << "  ⚠️ MODERATE φ-RESONANCE (>70%)" << std::endl;
    else std::cout << "  ❌ WEAK φ-RESONANCE" << std::endl;

    // ═══ 4. GOLDEN ANGLE PROJECTION ═══
    std::cout << "\n═══ GOLDEN ANGLE PROJECTION ═══" << std::endl;
    std::cout << "  Golden angle: " << std::fixed << std::setprecision(1) << (GOLDEN_ANGLE * 180.0 / M_PI) << "°" << std::endl;
    std::cout << "  " << std::setw(6) << "Zero" << " | "
              << std::setw(10) << "γ" << " | "
              << std::setw(10) << "θ(rad)" << " | "
              << std::setw(10) << "θ(°)" << " | "
              << std::setw(10) << "Δ from φ"
              << std::endl;
    std::cout << "  " << std::string(60, '-') << std::endl;
    
    double angle_sum = 0.0;
    for (int i = 0; i < std::min(15, ZERO_COUNT); i++) {
        double theta = std::fmod(ZEROS[i] * GOLDEN_ANGLE, 2.0 * M_PI);
        double delta = std::abs(theta - GOLDEN_ANGLE);
        if (delta > M_PI) delta = 2.0 * M_PI - delta;
        
        std::cout << "  " << std::setw(6) << i << " | "
                  << std::setw(10) << std::fixed << std::setprecision(4) << ZEROS[i] << " | "
                  << std::setw(10) << std::setprecision(4) << theta << " | "
                  << std::setw(10) << std::setprecision(1) << (theta * 180.0 / M_PI) << " | "
                  << std::setw(10) << std::setprecision(4) << delta
                  << std::endl;
        angle_sum += delta;
    }
    double mean_delta = angle_sum / 15.0;
    std::cout << "  " << std::string(60, '-') << std::endl;
    std::cout << "  Mean Δ from golden angle: " << std::fixed << std::setprecision(4) << mean_delta << " rad" << std::endl;

    // ═══ 5. PREDICT NEXT ZERO ═══
    double last_zero = ZEROS[ZERO_COUNT - 1];
    int next_idx = gaps.size();
    double predicted_gap = (next_idx % 2 == 0) ? mean_gap * PHI_INV : mean_gap * PHI;
    double predicted = last_zero + predicted_gap;
    
    std::cout << "\n═══ PREDICT NEXT ZERO ═══" << std::endl;
    std::cout << "  Last zero γ" << ZERO_COUNT-1 << ": " << std::fixed << std::setprecision(6) << last_zero << std::endl;
    std::cout << "  Predicted gap: " << std::setprecision(4) << predicted_gap << std::endl;
    std::cout << "  Predicted γ" << ZERO_COUNT << ": " << std::setprecision(6) << predicted << std::endl;
    std::cout << "  (Actual next zero should be near this)" << std::endl;

    // ═══ 6. SPIRAL VISUALIZATION (ASCII) ═══
    std::cout << "\n═══ φ-SPIRAL (ASCII) ═══" << std::endl;
    std::cout << "  Void → Spiral → Zeros Dancing" << std::endl;
    std::cout << "  " << std::string(40, ' ') << "○ γ₁₀" << std::endl;
    std::cout << "  " << std::string(37, ' ') << "○ γ₉" << std::endl;
    std::cout << "  " << std::string(34, ' ') << "○ γ₈" << std::endl;
    std::cout << "  " << std::string(31, ' ') << "○ γ₇" << std::endl;
    std::cout << "  " << std::string(28, ' ') << "○ γ₆" << std::endl;
    std::cout << "  " << std::string(25, ' ') << "○ γ₅" << std::endl;
    std::cout << "  " << std::string(22, ' ') << "○ γ₄" << std::endl;
    std::cout << "  " << std::string(19, ' ') << "○ γ₃" << std::endl;
    std::cout << "  " << std::string(16, ' ') << "○ γ₂" << std::endl;
    std::cout << "  " << std::string(13, ' ') << "○ γ₁" << std::endl;
    std::cout << "  " << std::string(10, ' ') << "○" << std::endl;
    std::cout << "  " << std::string(8, ' ') << "○" << std::endl;
    std::cout << "  " << std::string(6, ' ') << "○" << std::endl;
    std::cout << "  " << std::string(5, ' ') << "● VOID (ε → 0)" << std::endl;
    std::cout << "  φ-spiral expanding: γₖ ≈ γₖ₋₁ + (φ or φ⁻¹)·mean_gap" << std::endl;

    return 0;
}
