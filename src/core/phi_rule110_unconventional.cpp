// ============================================
// φ-RULE 110 UNCONVENTIONAL — DUAL SPACE
//
// Ang bawat cell ay may dalawang values:
// 1. φ-exponent (para sa transition)
// 2. Parity (para sa output)
//
// Ang parity ay may period-5 sa exponent space
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
    cout << "  φ-RULE 110 UNCONVENTIONAL\n";
    cout << "========================================\n\n";

    const double PHI = 1.6180339887498948482;
    int rule110[8] = {0, 1, 1, 0, 1, 1, 1, 0};

    const double V_ZERO = pow(PHI, -5);
    const double V_ONE = pow(PHI, -2);

    // ============================================
    // ANG PERIOD-5 PARITY
    // ============================================

    cout << "========================================\n";
    cout << "  PERIOD-5 PARITY\n";
    cout << "========================================\n\n";

    cout << "  Exponent | φ^exp    | floor | mod 2 | Period-5\n";
    cout << "  ---------|----------|-------|-------|----------\n";

    for (int exp = -5; exp <= 5; exp++) {
        double val = pow(PHI, exp);
        int floor_val = (int)floor(val);
        int mod2 = floor_val % 2;
        int period5 = ((exp % 5) + 5) % 5;
        
        cout << "  " << setw(7) << exp << " | "
             << setw(8) << fixed << setprecision(4) << val << " | "
             << setw(5) << floor_val << " | "
             << setw(5) << mod2 << " | "
             << setw(7) << period5 << "\n";
    }

    // ============================================
    // ANG PARITY PATTERN SA PERIOD-5
    // ============================================

    cout << "\n========================================\n";
    cout << "  PARITY SA PERIOD-5\n";
    cout << "========================================\n\n";

    cout << "  Period-5 | Parity\n";
    cout << "  ---------|-------\n";

    int period5_parity[5];
    for (int exp = 0; exp < 5; exp++) {
        double val = pow(PHI, exp);
        int floor_val = (int)floor(val);
        period5_parity[exp] = floor_val % 2;
        
        cout << "  " << exp << "        | " << period5_parity[exp] << "\n";
    }

    cout << "\n  Parity pattern: ";
    for (int i = 0; i < 5; i++) cout << period5_parity[i] << " ";
    cout << "\n\n";

    // ============================================
    // ANG SUMS AT ANG KANILANG PERIOD-5
    // ============================================

    cout << "========================================\n";
    cout << "  SUMS AT PERIOD-5\n";
    cout << "========================================\n\n";

    cout << "  Sum      | φ-log  | Nearest Exp | Period-5 | Parity | Expected\n";
    cout << "  ---------|--------|-------------|----------|--------|----------\n";

    int match_count = 0;
    for (int L : {0, 1}) {
        for (int C : {0, 1}) {
            for (int R : {0, 1}) {
                double l_val = (L ? V_ONE : V_ZERO) * PHI;
                double c_val = (C ? V_ONE : V_ZERO) * PHI * PHI;
                double r_val = (R ? V_ONE : V_ZERO) * PHI * PHI;
                double sum = l_val + c_val + r_val;
                
                double log_phi = log(sum) / log(PHI);
                int nearest_exp = (int)round(log_phi);
                int period5 = ((nearest_exp % 5) + 5) % 5;
                int parity = period5_parity[period5];
                
                int expected = rule110[(L << 2) | (C << 1) | R];
                bool match = (parity == expected);
                if (match) match_count++;
                
                cout << "  " << setw(8) << fixed << setprecision(4) << sum << " | "
                     << setw(6) << fixed << setprecision(2) << log_phi << " | "
                     << setw(11) << nearest_exp << " | "
                     << setw(7) << period5 << " | "
                     << setw(6) << parity << " | "
                     << setw(8) << expected << " | "
                     << (match ? "✅" : "❌") << "\n";
            }
        }
    }

    cout << "\n  Match: " << match_count << "/8\n\n";

    // ============================================
    // ANG KEY: PERIOD-5 BILANG AUTO-NORMALIZATION
    // ============================================

    cout << "========================================\n";
    cout << "  PERIOD-5 BILANG AUTO-NORMALIZATION\n";
    cout << "========================================\n\n";

    cout << "  Ang parity ay period-5 sa exponent space:\n";
    cout << "  0 → 1, 1 → 1, 2 → 0, 3 → 0, 4 → 0\n\n";

    cout << "  Ang output ay parity ng nearest φ-power ng sum.\n";
    cout << "  Ito ay auto-normalization: ang sum ay natural\n";
    cout << "  na nagco-collapse sa nearest φ-power.\n\n";

    return 0;
}
