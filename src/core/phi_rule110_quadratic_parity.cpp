// ============================================
// φ-RULE 110 QUADRATIC PARITY — DEPTH 1
//
// Hanapin ang quadratic polynomial na nag-a-approximate
// ng period-5 parity pattern: 1 1 0 0 0
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
    cout << "  φ-RULE 110 QUADRATIC PARITY\n";
    cout << "========================================\n\n";

    const double PHI = 1.6180339887498948482;

    // Period-5 parity: 1 1 0 0 0
    // Mod 5 values: 0→1, 1→1, 2→0, 3→0, 4→0
    
    int parity[5] = {1, 1, 0, 0, 0};

    // ============================================
    // QUADRATIC FIT
    // ============================================
    //
    // Hanapin ang p(x) = ax² + bx + c na nagbibigay ng:
    // p(0) = 1, p(1) = 1, p(2) = 0, p(3) = 0, p(4) = 0
    //
    // May 5 constraints at 3 unknowns — overdetermined.
    // Kailangan ng least squares fit.

    cout << "========================================\n";
    cout << "  QUADRATIC FIT (LEAST SQUARES)\n";
    cout << "========================================\n\n";

    // Least squares: minimize Σ(p(x) - parity[x])²
    // Normal equations:
    // [Σx⁴  Σx³  Σx²] [a]   [Σx²×parity]
    // [Σx³  Σx²  Σx ] [b] = [Σx×parity ]
    // [Σx²  Σx   n ] [c]   [Σparity   ]

    double sum_x4 = 0, sum_x3 = 0, sum_x2 = 0, sum_x = 0, sum_n = 5;
    double sum_x2y = 0, sum_xy = 0, sum_y = 0;

    for (int x = 0; x < 5; x++) {
        sum_x4 += pow(x, 4);
        sum_x3 += pow(x, 3);
        sum_x2 += pow(x, 2);
        sum_x += x;
        sum_x2y += pow(x, 2) * parity[x];
        sum_xy += x * parity[x];
        sum_y += parity[x];
    }

    // Solve 3x3 system
    // [sum_x4  sum_x3  sum_x2] [a]   [sum_x2y]
    // [sum_x3  sum_x2  sum_x ] [b] = [sum_xy ]
    // [sum_x2  sum_x   sum_n ] [c]   [sum_y  ]

    double A[3][3] = {
        {sum_x4, sum_x3, sum_x2},
        {sum_x3, sum_x2, sum_x},
        {sum_x2, sum_x, sum_n}
    };
    double B[3] = {sum_x2y, sum_xy, sum_y};

    // Gaussian elimination
    for (int i = 0; i < 3; i++) {
        double pivot = A[i][i];
        for (int j = i; j < 3; j++) A[i][j] /= pivot;
        B[i] /= pivot;
        
        for (int k = 0; k < 3; k++) {
            if (k != i) {
                double factor = A[k][i];
                for (int j = i; j < 3; j++) A[k][j] -= factor * A[i][j];
                B[k] -= factor * B[i];
            }
        }
    }

    double a = B[0], b = B[1], c = B[2];

    cout << "  Quadratic: p(x) = " << a << "x² + " << b << "x + " << c << "\n\n";

    cout << "  x | p(x)    | Threshold | Output | Expected\n";
    cout << "  --|---------|-----------|--------|----------\n";

    for (int x = 0; x < 5; x++) {
        double p = a * x * x + b * x + c;
        int output = (p > 0.5) ? 1 : 0;
        
        cout << "  " << x << " | "
             << setw(7) << fixed << setprecision(4) << p << " | "
             << setw(9) << (p > 0.5 ? "> 0.5" : "≤ 0.5") << " | "
             << setw(6) << output << " | "
             << setw(8) << parity[x] << " | "
             << (output == parity[x] ? "✅" : "❌") << "\n";
    }

    // ============================================
    // MAS MAGANDANG APPROACH: DIFFERENCE QUADRATIC
    // ============================================

    cout << "\n========================================\n";
    cout << "  DIFFERENCE QUADRATIC\n";
    cout << "========================================\n\n";

    // Subukan: p(x) = (x-2)(x-4) na may roots sa 2 at 4
    // p(0) = 8, p(1) = 3, p(2) = 0, p(3) = -3, p(4) = 0
    // Normalized: divide by 8
    // p(0) = 1, p(1) = 0.375, p(2) = 0, p(3) = -0.375, p(4) = 0

    auto p1 = [](double x) {
        return (x - 2.0) * (x - 4.0) / 8.0;
    };

    cout << "  p(x) = (x-2)(x-4)/8\n\n";

    cout << "  x | p(x)    | Output | Expected\n";
    cout << "  --|---------|--------|----------\n";

    int match1 = 0;
    for (int x = 0; x < 5; x++) {
        double p = p1(x);
        int output = (p > 0.2) ? 1 : 0;
        
        cout << "  " << x << " | "
             << setw(7) << fixed << setprecision(4) << p << " | "
             << setw(6) << output << " | "
             << setw(8) << parity[x] << " | "
             << (output == parity[x] ? "✅" : "❌") << "\n";
        if (output == parity[x]) match1++;
    }
    cout << "  Match: " << match1 << "/5\n\n";

    // Subukan: p(x) = (x-2)(x-3) na may roots sa 2 at 3
    // p(0) = 6, p(1) = 2, p(2) = 0, p(3) = 0, p(4) = 2
    // Normalized: divide by 6
    // p(0) = 1, p(1) = 0.333, p(2) = 0, p(3) = 0, p(4) = 0.333

    auto p2 = [](double x) {
        return (x - 2.0) * (x - 3.0) / 6.0;
    };

    cout << "  p(x) = (x-2)(x-3)/6\n\n";

    cout << "  x | p(x)    | Output | Expected\n";
    cout << "  --|---------|--------|----------\n";

    int match2 = 0;
    for (int x = 0; x < 5; x++) {
        double p = p2(x);
        int output = (p > 0.2) ? 1 : 0;
        
        cout << "  " << x << " | "
             << setw(7) << fixed << setprecision(4) << p << " | "
             << setw(6) << output << " | "
             << setw(8) << parity[x] << " | "
             << (output == parity[x] ? "✅" : "❌") << "\n";
        if (output == parity[x]) match2++;
    }
    cout << "  Match: " << match2 << "/5\n\n";

    // Subukan: p(x) = (x-3)(x-4) na may roots sa 3 at 4
    // p(0) = 12, p(1) = 6, p(2) = 2, p(3) = 0, p(4) = 0
    // Normalized: divide by 12
    // p(0) = 1, p(1) = 0.5, p(2) = 0.167, p(3) = 0, p(4) = 0

    auto p3 = [](double x) {
        return (x - 3.0) * (x - 4.0) / 12.0;
    };

    cout << "  p(x) = (x-3)(x-4)/12\n\n";

    cout << "  x | p(x)    | Output | Expected\n";
    cout << "  --|---------|--------|----------\n";

    int match3 = 0;
    for (int x = 0; x < 5; x++) {
        double p = p3(x);
        int output = (p > 0.15) ? 1 : 0;
        
        cout << "  " << x << " | "
             << setw(7) << fixed << setprecision(4) << p << " | "
             << setw(6) << output << " | "
             << setw(8) << parity[x] << " | "
             << (output == parity[x] ? "✅" : "❌") << "\n";
        if (output == parity[x]) match3++;
    }
    cout << "  Match: " << match3 << "/5\n\n";

    return 0;
}
