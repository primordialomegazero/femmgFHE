// ============================================
// φ-RULE 110 SELF-REF EVOLUTION
//
// Ang transition ay self-referential:
// Ang output ay parity ng φ-exponent
// Ang bagong state ay φ^parity
//
// Encoding: 0 → 1 (φ⁰), 1 → φ (φ¹)
// Output: parity ng exponent sum
// New state: φ^output
//
// Author: Dan Fernandez / Primordial Omega Zero
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>
#include <chrono>

using namespace std;

int main() {
    cout << "========================================\n";
    cout << "  φ-RULE 110 SELF-REF EVOLUTION\n";
    cout << "========================================\n\n";

    const double PHI = 1.6180339887498948482;

    int rule110[8] = {0, 1, 1, 0, 1, 1, 1, 0};

    // ============================================
    // SELF-REFERENTIAL TRANSITION (PLAINTEXT)
    // ============================================

    cout << "========================================\n";
    cout << "  SELF-REF TRANSITION TABLE\n";
    cout << "========================================\n\n";

    cout << "  L C R | Exp Sum | Parity | Expected | Match?\n";
    cout << "  ------|---------|--------|----------|--------\n";

    int match_count = 0;
    for (int L : {0, 1}) {
        for (int C : {0, 1}) {
            for (int R : {0, 1}) {
                // Encoding: 0 → exp 0, 1 → exp 1
                int exp_sum = L + C + R;
                
                // Parity: floor(φ^exp_sum) mod 2
                double val = pow(PHI, exp_sum);
                int parity = ((int)floor(val)) % 2;
                
                int expected = rule110[(L << 2) | (C << 1) | R];
                bool match = (parity == expected);
                if (match) match_count++;
                
                cout << "  " << L << " " << C << " " << R << " | "
                     << setw(7) << exp_sum << " | "
                     << setw(6) << parity << " | "
                     << setw(8) << expected << " | "
                     << (match ? "✅" : "❌") << "\n";
            }
        }
    }
    cout << "\n  Match: " << match_count << "/8\n\n";

    // ============================================
    // SELF-REFERENTIAL ENCODING
    // ============================================
    //
    // Ang problema: ang parity ng exp_sum ay hindi
    // nagbibigay ng tamang Rule 110 output.
    //
    // ANG KEY: Kailangan natin ng POSITIONAL encoding.
    // Ang L, C, R ay may iba't ibang φ-weights.
    //
    // L → φ⁻³ (exp -3)
    // C → φ⁻² (exp -2)
    // R → φ/2 (exp -0.44) o φ (exp 1)
    //
    // Ang exp sum ay:
    // 000 → 0 + 0 + (-0.44) = -0.44 → parity ?
    // 001 → 0 + 0 + 1 = 1 → parity 1
    // 010 → 0 + (-2) + (-0.44) = -2.44 → parity ?
    // ...

    cout << "========================================\n";
    cout << "  POSITIONAL EXPONENT ENCODING\n";
    cout << "========================================\n\n";

    cout << "  L C R | Exp Sum | φ^sum  | Floor | mod 2 | Expected\n";
    cout << "  ------|---------|--------|-------|-------|----------\n";

    // Positional exponents
    double L_exp_0 = -999;  // -∞ (0)
    double L_exp_1 = -3.0;
    double C_exp_0 = -999;  // -∞ (0)
    double C_exp_1 = -2.0;
    double R_exp_0 = -0.44; // log_φ(φ/2)
    double R_exp_1 = 1.0;

    int match2 = 0;
    for (int L : {0, 1}) {
        for (int C : {0, 1}) {
            for (int R : {0, 1}) {
                double l_val = (L ? pow(PHI, L_exp_1) : 0.0);
                double c_val = (C ? pow(PHI, C_exp_1) : 0.0);
                double r_val = (R ? pow(PHI, R_exp_1) : pow(PHI, R_exp_0));
                double sum = l_val + c_val + r_val;
                
                double exp_sum = log(sum) / log(PHI);
                int floor_val = (int)floor(sum);
                int mod2 = floor_val % 2;
                
                int expected = rule110[(L << 2) | (C << 1) | R];
                bool match = (mod2 == expected);
                if (match) match2++;
                
                cout << "  " << L << " " << C << " " << R << " | "
                     << setw(7) << fixed << setprecision(2) << exp_sum << " | "
                     << setw(6) << fixed << setprecision(3) << sum << " | "
                     << setw(5) << floor_val << " | "
                     << setw(5) << mod2 << " | "
                     << setw(8) << expected << " | "
                     << (match ? "✅" : "❌") << "\n";
            }
        }
    }
    cout << "\n  Match: " << match2 << "/8\n\n";

    // ============================================
    // ANG TAMANG WEIGHTS (MULA SA 8/8 SEARCH)
    // ============================================

    cout << "========================================\n";
    cout << "  TAMANG 8/8 WEIGHTS\n";
    cout << "========================================\n\n";

    double W_L_ZERO = 0.0;
    double W_L_ONE = pow(PHI, -3);
    double W_C_ZERO = 0.0;
    double W_C_ONE = pow(PHI, -2);
    double W_R_ZERO = PHI / 2.0;
    double W_R_ONE = PHI;

    cout << "  L C R | Sum      | Floor | mod 2 | Expected\n";
    cout << "  ------|----------|-------|-------|----------\n";

    int match3 = 0;
    for (int L : {0, 1}) {
        for (int C : {0, 1}) {
            for (int R : {0, 1}) {
                double sum = (L ? W_L_ONE : W_L_ZERO) + 
                            (C ? W_C_ONE : W_C_ZERO) + 
                            (R ? W_R_ONE : W_R_ZERO);
                int floor_val = (int)floor(sum);
                int mod2 = floor_val % 2;
                int expected = rule110[(L << 2) | (C << 1) | R];
                bool match = (mod2 == expected);
                if (match) match3++;
                
                cout << "  " << L << " " << C << " " << R << " | "
                     << setw(8) << fixed << setprecision(4) << sum << " | "
                     << setw(5) << floor_val << " | "
                     << setw(5) << mod2 << " | "
                     << setw(8) << expected << " | "
                     << (match ? "✅" : "❌") << "\n";
            }
        }
    }
    cout << "\n  Match: " << match3 << "/8\n\n";

    return 0;
}
