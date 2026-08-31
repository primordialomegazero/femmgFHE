// ============================================
// φ-AUTO NORMALIZE — φ-PERIODICITY BILANG
// NATURAL NA NORMALIZATION
//
// Ang golden identity φ² = φ + 1 ay nagbibigay
// ng auto-normalization para sa Rule 110
//
// Author: Dan Fernandez / Primordial Omega Zero
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>
#include <algorithm>

using namespace std;

int main() {
    cout << "========================================\n";
    cout << "  φ-AUTO NORMALIZE\n";
    cout << "========================================\n\n";

    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);

    int rule110[8] = {0, 1, 1, 0, 1, 1, 1, 0};

    // ============================================
    // ANG SUMS AT ANG KANILANG φ-PERIODICITY
    // ============================================

    cout << "========================================\n";
    cout << "  SUMS AT φ-PERIODICITY\n";
    cout << "========================================\n\n";

    const double V_ZERO = pow(PHI, -5);  // 0.09017
    const double V_ONE = pow(PHI, -2);   // 0.38197

    cout << "  Sum      | φ-log     | φ-period  | Normalized | Output\n";
    cout << "  ---------|-----------|-----------|------------|-------\n";

    for (int L : {0, 1}) {
        for (int C : {0, 1}) {
            for (int R : {0, 1}) {
                double l_val = (L ? V_ONE : V_ZERO) * PHI;
                double c_val = (C ? V_ONE : V_ZERO) * PHI * PHI;
                double r_val = (R ? V_ONE : V_ZERO) * PHI * PHI;
                double sum = l_val + c_val + r_val;
                
                double log_sum = log(sum) / LN_PHI;
                int nearest_n = (int)round(log_sum);
                int normalized_exp = ((nearest_n % 5) + 5) % 5;
                
                int expected = rule110[(L << 2) | (C << 1) | R];
                int output = ((int)floor(sum)) % 2;
                
                cout << "  " << setw(8) << fixed << setprecision(4) << sum << " | "
                     << setw(9) << fixed << setprecision(2) << log_sum << " | "
                     << setw(9) << nearest_n << " | "
                     << setw(10) << normalized_exp << " | "
                     << setw(6) << output << " (expected " << expected << ")\n";
            }
        }
    }

    // ============================================
    // DIRECT MAPPING
    // ============================================

    cout << "\n========================================\n";
    cout << "  DIRECT MAPPING\n";
    cout << "========================================\n\n";

    cout << "  Sum      | φ-log  | floor | mod 2 | Output\n";
    cout << "  ---------|--------|-------|-------|-------\n";

    vector<pair<double, int>> sum_output_pairs;
    for (int L : {0, 1}) {
        for (int C : {0, 1}) {
            for (int R : {0, 1}) {
                double l_val = (L ? V_ONE : V_ZERO) * PHI;
                double c_val = (C ? V_ONE : V_ZERO) * PHI * PHI;
                double r_val = (R ? V_ONE : V_ZERO) * PHI * PHI;
                double sum = l_val + c_val + r_val;
                int output = ((int)floor(sum)) % 2;
                sum_output_pairs.push_back({sum, output});
            }
        }
    }

    sort(sum_output_pairs.begin(), sum_output_pairs.end());

    for (auto& [sum, output] : sum_output_pairs) {
        double log_sum = log(sum) / LN_PHI;
        int floor_val = (int)floor(sum);
        cout << "  " << setw(8) << fixed << setprecision(4) << sum << " | "
             << setw(6) << fixed << setprecision(2) << log_sum << " | "
             << setw(5) << floor_val << " | "
             << setw(5) << floor_val % 2 << " | "
             << setw(6) << output << "\n";
    }

    // ============================================
    // ANG AUTO-NORMALIZATION SA LOG SPACE
    // ============================================

    cout << "\n========================================\n";
    cout << "  AUTO-NORMALIZATION SA LOG SPACE\n";
    cout << "========================================\n\n";

    cout << "  Sa φ-log space, ang sum ay:\n";
    cout << "  log_sum = log(φ^a + φ^b + φ^c)\n\n";

    cout << "  Ang φ-periodicity ay nagbibigay ng:\n";
    cout << "  log_sum mod ln(φ⁵) = normalized_log\n\n";

    cout << "  Kung saan:\n";
    cout << "  ln(φ⁵) = 5×ln(φ) = " << 5 * LN_PHI << "\n\n";

    cout << "  At ang output ay:\n";
    cout << "  output = floor(exp(normalized_log)) mod 2\n\n";

    // ============================================
    // ANG PERIOD-5 NA OUTPUT
    // ============================================

    cout << "========================================\n";
    cout << "  PERIOD-5 OUTPUT\n";
    cout << "========================================\n\n";

    cout << "  Ang normalized φ-exponent ay nasa range [0, 5):\n\n";

    cout << "  Normalized | φ^exp    | floor | mod 2\n";
    cout << "  -----------|----------|-------|-------\n";

    for (double exp_val = 0; exp_val < 5; exp_val += 0.5) {
        double phi_pow = pow(PHI, exp_val);
        int floor_val = (int)floor(phi_pow);
        cout << "  " << setw(9) << fixed << setprecision(1) << exp_val << " | "
             << setw(8) << fixed << setprecision(4) << phi_pow << " | "
             << setw(5) << floor_val << " | "
             << setw(5) << floor_val % 2 << "\n";
    }

    // ============================================
    // ANG KEY: NORMALIZED EXPONENT → OUTPUT
    // ============================================

    cout << "\n========================================\n";
    cout << "  NORMALIZED EXPONENT → OUTPUT\n";
    cout << "========================================\n\n";

    cout << "  Normalized | floor(φ^exp) | mod 2 | Output\n";
    cout << "  -----------|-------------|-------|-------\n";

    for (int exp_val = -5; exp_val <= 5; exp_val++) {
        double phi_pow = pow(PHI, exp_val);
        int floor_val = (int)floor(phi_pow);
        int mod2 = floor_val % 2;
        int normalized = ((exp_val % 5) + 5) % 5;
        
        cout << "  " << setw(9) << exp_val << " | "
             << setw(11) << floor_val << " | "
             << setw(5) << mod2 << " | "
             << setw(6) << mod2 << "\n";
    }

    return 0;
}
