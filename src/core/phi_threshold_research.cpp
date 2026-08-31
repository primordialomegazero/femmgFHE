// ============================================
// φ-THRESHOLD RESEARCH — GOLDEN RATIO THRESHOLD
//
// Ang output ay natutukoy ng fractional part ng φ-log:
// frac = 0 → Output 0
// frac ∈ (0, φ⁻¹) → Output 1
// frac ≥ φ⁻¹ → Output 0
//
// Ito ay maaaring i-approximate sa depth 1
//
// Author: Dan Fernandez / Primordial Omega Zero
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>

using namespace std;

int main() {
    cout << "========================================\n";
    cout << "  φ-THRESHOLD RESEARCH\n";
    cout << "========================================\n\n";

    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);

    int rule110[8] = {0, 1, 1, 0, 1, 1, 1, 0};

    const double V_ZERO = pow(PHI, -5);
    const double V_ONE = pow(PHI, -2);

    // ============================================
    // ANG SUMS AT ANG KANILANG φ-LOGS
    // ============================================

    cout << "========================================\n";
    cout << "  SUMS AT φ-LOGS\n";
    cout << "========================================\n\n";

    cout << "  Sum      | φ-log  | Frac    | Expected | Threshold?\n";
    cout << "  ---------|--------|---------|----------|-----------\n";

    for (int L : {0, 1}) {
        for (int C : {0, 1}) {
            for (int R : {0, 1}) {
                double l_val = (L ? V_ONE : V_ZERO) * PHI;
                double c_val = (C ? V_ONE : V_ZERO) * PHI * PHI;
                double r_val = (R ? V_ONE : V_ZERO) * PHI * PHI;
                double sum = l_val + c_val + r_val;
                
                double log_phi = log(sum) / LN_PHI;
                double frac = log_phi - floor(log_phi);
                
                int expected = rule110[(L << 2) | (C << 1) | R];
                
                // Golden threshold: frac < φ⁻¹ → 1, frac ≥ φ⁻¹ → 0
                // Maliban sa frac = 0 → 0
                int threshold_output;
                if (frac < 0.001) threshold_output = 0;
                else if (frac < PHI - 1.0) threshold_output = 1;
                else threshold_output = 0;
                
                bool match = (threshold_output == expected);
                
                cout << "  " << setw(8) << fixed << setprecision(4) << sum << " | "
                     << setw(6) << fixed << setprecision(2) << log_phi << " | "
                     << setw(7) << fixed << setprecision(4) << frac << " | "
                     << setw(8) << expected << " | "
                     << setw(9) << threshold_output << " | "
                     << (match ? "✅" : "❌") << "\n";
            }
        }
    }

    // ============================================
    // ANG GOLDEN THRESHOLD FUNCTION
    // ============================================

    cout << "\n========================================\n";
    cout << "  GOLDEN THRESHOLD FUNCTION\n";
    cout << "========================================\n\n";

    cout << "  output = 0 kung frac = 0\n";
    cout << "  output = 1 kung 0 < frac < φ⁻¹\n";
    cout << "  output = 0 kung frac ≥ φ⁻¹\n\n";

    cout << "  Sa φ-value space:\n";
    cout << "  output = 0 kung sum = φ^n (eksaktong φ-power)\n";
    cout << "  output = 1 kung φ^n < sum < φ^(n+φ⁻¹)\n";
    cout << "  output = 0 kung sum ≥ φ^(n+φ⁻¹)\n\n";

    // ============================================
    // ANG THRESHOLD SA VALUE SPACE
    // ============================================

    cout << "========================================\n";
    cout << "  THRESHOLD SA VALUE SPACE\n";
    cout << "========================================\n\n";

    cout << "  Ang sums ay nasa range [0.618, 2.618]:\n\n";

    cout << "  Output 0 sums:\n";
    cout << "  0.618 = φ⁻¹ (frac = 0)\n";
    cout << "  2.146 = φ^1.59 (frac = 0.59 ≥ φ⁻¹? 0.59 < 0.618 → ❌)\n";
    cout << "  2.618 = φ² (frac = 0)\n\n";

    cout << "  Output 1 sums:\n";
    cout << "  1.090 = φ^0.18 (frac = 0.18 < φ⁻¹ → ✅)\n";
    cout << "  1.382 = φ^0.67 (frac = 0.67 ≥ φ⁻¹? 0.67 > 0.618 → ❌)\n";
    cout << "  1.854 = φ^1.28 (frac = 0.28 < φ⁻¹ → ✅)\n\n";

    cout << "  ANG PROBLEMA: Ang frac = 0.59 (output 0) at frac = 0.67 (output 1)\n";
    cout << "  ay magkalapit — mahirap i-distinguish sa φ⁻¹ threshold.\n\n";

    // ============================================
    // ANG MAS MALINIS NA THRESHOLD
    // ============================================

    cout << "========================================\n";
    cout << "  MAS MALINIS NA THRESHOLD\n";
    cout << "========================================\n\n";

    cout << "  Subukan: output = 1 kung frac ∈ [0.18, 0.28] o [0.67]\n";
    cout << "  Output = 0 kung frac ∈ {0.00, 0.59}\n\n";

    cout << "  Sa value space:\n";
    cout << "  Output 1: sums ∈ {1.090, 1.382, 1.854}\n";
    cout << "  Output 0: sums ∈ {0.618, 2.146, 2.618}\n\n";

    cout << "  ANG KEY: Ang output 1 sums ay nasa gitna\n";
    cout << "  ng range [0.618, 2.618]:\n";
    cout << "  Gitna = (0.618 + 2.618) / 2 = 1.618 = φ¹\n\n";

    cout << "  Output 1 sums ay malapit sa φ¹ = 1.618!\n";
    cout << "  1.090, 1.382, 1.854 — lahat nasa paligid ng φ¹\n\n";

    cout << "  Output 0 sums ay malayo sa φ¹:\n";
    cout << "  0.618 = φ⁻¹, 2.146 ≈ φ² - φ⁻¹, 2.618 = φ²\n\n";

    // ============================================
    // ANG DISTANCE-BASED THRESHOLD
    // ============================================

    cout << "========================================\n";
    cout << "  DISTANCE-BASED THRESHOLD\n";
    cout << "========================================\n\n";

    cout << "  output = 1 kung |sum - φ¹| < 0.5\n";
    cout << "  output = 0 kung |sum - φ¹| ≥ 0.5\n\n";

    cout << "  Sum      | |sum - φ¹| | Output\n";
    cout << "  ---------|-----------|-------\n";

    for (int L : {0, 1}) {
        for (int C : {0, 1}) {
            for (int R : {0, 1}) {
                double l_val = (L ? V_ONE : V_ZERO) * PHI;
                double c_val = (C ? V_ONE : V_ZERO) * PHI * PHI;
                double r_val = (R ? V_ONE : V_ZERO) * PHI * PHI;
                double sum = l_val + c_val + r_val;
                
                double dist = abs(sum - PHI);
                int expected = rule110[(L << 2) | (C << 1) | R];
                int threshold_output = (dist < 0.5) ? 1 : 0;
                bool match = (threshold_output == expected);
                
                cout << "  " << setw(8) << fixed << setprecision(4) << sum << " | "
                     << setw(9) << fixed << setprecision(4) << dist << " | "
                     << setw(6) << threshold_output << " | "
                     << (match ? "✅" : "❌") << "\n";
            }
        }
    }

    // ============================================
    // ANG PINAKAMALINIS: φ-CENTERED BAND
    // ============================================

    cout << "\n========================================\n";
    cout << "  φ-CENTERED BAND\n";
    cout << "========================================\n\n";

    cout << "  output = 1 kung sum ∈ [φ - δ, φ + δ]\n";
    cout << "  kung saan δ = φ⁻² = 0.382\n\n";

    cout << "  Band: [" << PHI - pow(PHI, -2) << ", " << PHI + pow(PHI, -2) << "]\n\n";

    cout << "  Sum      | In band? | Output\n";
    cout << "  ---------|----------|-------\n";

    int match_count = 0;
    for (int L : {0, 1}) {
        for (int C : {0, 1}) {
            for (int R : {0, 1}) {
                double l_val = (L ? V_ONE : V_ZERO) * PHI;
                double c_val = (C ? V_ONE : V_ZERO) * PHI * PHI;
                double r_val = (R ? V_ONE : V_ZERO) * PHI * PHI;
                double sum = l_val + c_val + r_val;
                
                double band_low = PHI - pow(PHI, -2);
                double band_high = PHI + pow(PHI, -2);
                bool in_band = (sum >= band_low && sum <= band_high);
                int expected = rule110[(L << 2) | (C << 1) | R];
                int output = in_band ? 1 : 0;
                bool match = (output == expected);
                if (match) match_count++;
                
                cout << "  " << setw(8) << fixed << setprecision(4) << sum << " | "
                     << setw(8) << (in_band ? "YES" : "NO") << " | "
                     << setw(6) << output << " | "
                     << (match ? "✅" : "❌") << "\n";
            }
        }
    }

    cout << "\n  Match: " << match_count << "/8\n";

    return 0;
}
