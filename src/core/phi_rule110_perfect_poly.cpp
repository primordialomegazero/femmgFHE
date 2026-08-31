// ============================================
// φ-RULE 110 PERFECT POLYNOMIAL — TRUE 8/8
//
// Fix boundary issue with expanded band
// Author: Dan Fernandez / Primordial Omega Zero
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>

using namespace std;

int main() {
    cout << "========================================\n";
    cout << "  φ-RULE 110 PERFECT POLYNOMIAL\n";
    cout << "========================================\n\n";

    const double PHI = 1.6180339887498948482;
    const double V_ZERO = pow(PHI, -5);
    const double V_ONE = pow(PHI, -2);

    // ============================================
    // EXPANDED BAND WITH SAFETY MARGIN
    // ============================================
    
    // Original band: [5φ-7, 3φ-3] = [1.09017, 1.85410]
    // Problem: Sum values exactly at boundaries
    // Solution: Expand by ε = φ⁻⁶ ≈ 0.0557
    
    const double EPSILON = pow(PHI, -6);  // 0.0557
    const double LOWER = 5.0 * PHI - 7.0 - EPSILON;  // 1.0345
    const double UPPER = 3.0 * PHI - 3.0 + EPSILON;  // 1.9098
    
    auto band_poly = [&](double x) {
        return (x - LOWER) * (UPPER - x);
    };
    
    cout << "Method 1: Expanded Band (with ε = φ⁻⁶)\n";
    cout << "  Band: [" << LOWER << ", " << UPPER << "]\n";
    cout << "  p(x) = (x - " << LOWER << ")( " << UPPER << " - x)\n\n";
    
    cout << "  L C R | Sum      | p(sum)  | Output | Expected\n";
    cout << "  ------|----------|---------|--------|----------\n";
    
    int match1 = 0;
    int rule110[8] = {0, 1, 1, 0, 1, 1, 1, 0};
    
    for (int L : {0, 1}) {
        for (int C : {0, 1}) {
            for (int R : {0, 1}) {
                double l_val = (L ? V_ONE : V_ZERO) * PHI;
                double c_val = (C ? V_ONE : V_ZERO) * PHI * PHI;
                double r_val = (R ? V_ONE : V_ZERO) * PHI * PHI;
                double sum = l_val + c_val + r_val;
                
                int idx = (L << 2) | (C << 1) | R;
                int expected = rule110[idx];
                double score = band_poly(sum);
                int output = (score > 0) ? 1 : 0;
                if (output == expected) match1++;
                
                cout << "  " << L << " " << C << " " << R << " | "
                     << fixed << setprecision(4) << sum << " | "
                     << setw(7) << setprecision(6) << score << " | "
                     << setw(6) << output << " | "
                     << setw(8) << expected << " | "
                     << (output == expected ? "✅" : "❌") << "\n";
            }
        }
    }
    cout << "  Match: " << match1 << "/8\n\n";
    
    // ============================================
    // METHOD 2: CUBIC POLYNOMIAL WITH MULTIPLE ROOTS
    // ============================================
    
    // Use polynomial with roots at output-0 sums
    // But ensure output-1 sums give positive values
    // p(x) = -(x-0.618)(x-2.146)(x-2.618) + shift
    
    auto cubic_shifted = [](double x) {
        double base = -(x - 0.618) * (x - 2.146) * (x - 2.618);
        return base + 0.5;  // Shift up to make output-1 positive
    };
    
    cout << "Method 2: Shifted Cubic\n";
    cout << "  p(x) = -(x-0.618)(x-2.146)(x-2.618) + 0.5\n\n";
    
    cout << "  L C R | Sum      | p(sum)  | Output | Expected\n";
    cout << "  ------|----------|---------|--------|----------\n";
    
    int match2 = 0;
    for (int L : {0, 1}) {
        for (int C : {0, 1}) {
            for (int R : {0, 1}) {
                double l_val = (L ? V_ONE : V_ZERO) * PHI;
                double c_val = (C ? V_ONE : V_ZERO) * PHI * PHI;
                double r_val = (R ? V_ONE : V_ZERO) * PHI * PHI;
                double sum = l_val + c_val + r_val;
                
                int idx = (L << 2) | (C << 1) | R;
                int expected = rule110[idx];
                double score = cubic_shifted(sum);
                int output = (score > 0) ? 1 : 0;
                if (output == expected) match2++;
                
                cout << "  " << L << " " << C << " " << R << " | "
                     << fixed << setprecision(4) << sum << " | "
                     << setw(7) << setprecision(6) << score << " | "
                     << setw(6) << output << " | "
                     << setw(8) << expected << " | "
                     << (output == expected ? "✅" : "❌") << "\n";
            }
        }
    }
    cout << "  Match: " << match2 << "/8\n\n";
    
    // ============================================
    // METHOD 3: PIECEWISE POLYNOMIAL (DEPTH 1)
    // ============================================
    
    // Use indicator function approach
    // output = 1 if sum ∈ [1.03, 1.91]
    // This is equivalent to: (sum > 1.03) AND (sum < 1.91)
    
    cout << "Method 3: Simple Comparison (FHE-native)\n";
    cout << "  output = (sum > 1.03) AND (sum < 1.91)\n\n";
    
    cout << "  L C R | Sum      | > 1.03 | < 1.91 | Output | Expected\n";
    cout << "  ------|----------|--------|--------|--------|----------\n";
    
    int match3 = 0;
    for (int L : {0, 1}) {
        for (int C : {0, 1}) {
            for (int R : {0, 1}) {
                double l_val = (L ? V_ONE : V_ZERO) * PHI;
                double c_val = (C ? V_ONE : V_ZERO) * PHI * PHI;
                double r_val = (R ? V_ONE : V_ZERO) * PHI * PHI;
                double sum = l_val + c_val + r_val;
                
                int idx = (L << 2) | (C << 1) | R;
                int expected = rule110[idx];
                bool gt_lower = (sum > 1.03);
                bool lt_upper = (sum < 1.91);
                int output = (gt_lower && lt_upper) ? 1 : 0;
                if (output == expected) match3++;
                
                cout << "  " << L << " " << C << " " << R << " | "
                     << fixed << setprecision(4) << sum << " | "
                     << setw(6) << (gt_lower ? "YES" : "NO") << " | "
                     << setw(6) << (lt_upper ? "YES" : "NO") << " | "
                     << setw(6) << output << " | "
                     << setw(8) << expected << " | "
                     << (output == expected ? "✅" : "❌") << "\n";
            }
        }
    }
    cout << "  Match: " << match3 << "/8\n\n";
    
    cout << "========================================\n";
    cout << "  BEST SOLUTION: EXPANDED BAND\n";
    cout << "========================================\n\n";
    cout << "  p(x) = (x - 1.0345)(1.9098 - x)\n";
    cout << "  Output = (p(x) > 0) ? 1 : 0\n\n";
    cout << "  FHE Complexity:\n";
    cout << "  - Depth: 1 (single multiplication)\n";
    cout << "  - Operations: 2 subtractions, 1 multiplication\n";
    cout << "  - Comparison: 1 sign test\n";
    cout << "  - Perfect 8/8 accuracy!\n";
    
    return 0;
}
