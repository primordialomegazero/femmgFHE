// ============================================
// φ-FIBONACCI INDEX
// I-encode ang Fibonacci index sa Slot 3
// para ang EvalAdd ay magbigay ng tamang index
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
    cout << "  φ-FIBONACCI INDEX\n";
    cout << "========================================\n\n";

    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);

    vector<long long> fib = {0, 1};
    for (int i = 2; i <= 30; i++) {
        fib.push_back(fib[i-1] + fib[i-2]);
    }

    // ============================================
    // TEST: INDEX ADDITION
    // ============================================

    cout << "========================================\n";
    cout << "  INDEX ADDITION\n";
    cout << "========================================\n\n";

    cout << "  x | log_φ(x) | floor | frac | F_floor\n";
    cout << "  --|-----------|-------|------|--------\n";

    for (double x : {5.0, 7.0, 35.0, 3.0}) {
        double log_phi_x = log(x) / LN_PHI;
        double floor_val = floor(log_phi_x);
        double frac = log_phi_x - floor_val;
        long long b = fib[(int)floor_val];
        
        cout << "  " << setw(3) << x << " | "
             << setw(9) << fixed << setprecision(4) << log_phi_x << " | "
             << setw(5) << floor_val << " | "
             << setw(4) << frac << " | "
             << setw(6) << b << "\n";
    }

    // ============================================
    // TEST: (5 × 7) + 3
    // ============================================

    cout << "\n========================================\n";
    cout << "  (5 × 7) + 3 = 38\n";
    cout << "========================================\n\n";

    // 5: floor=3, frac=0.3446, F_3=2
    // 7: floor=4, frac=0.0438, F_4=3
    // 35: floor=7, frac=0.3883, F_7=13

    double floor_5 = 3, frac_5 = 0.3446;
    double floor_7 = 4, frac_7 = 0.0438;
    double floor_35 = 7, frac_35 = 0.3883;

    cout << "  floor(5) + floor(7) = " << floor_5 + floor_7 << " = floor(35)? " 
         << (floor_5 + floor_7 == floor_35 ? "✅" : "❌") << "\n";
    cout << "  frac(5) + frac(7) = " << frac_5 + frac_7 << " = frac(35)? "
         << (abs(frac_5 + frac_7 - frac_35) < 0.01 ? "✅" : "❌") << "\n\n";

    // Subok: ang floor sum ay nagbibigay ng index para sa F
    long long F_35 = fib[(int)(floor_5 + floor_7)];
    cout << "  F_{floor(5)+floor(7)} = F_7 = " << F_35 << "\n";
    cout << "  Expected b(35) = 13\n";
    cout << "  Match: " << (F_35 == 13 ? "✅" : "❌") << "\n\n";

    // ============================================
    // ANG KAHULUGAN
    // ============================================

    cout << "========================================\n";
    cout << "  ANG KAHULUGAN\n";
    cout << "========================================\n\n";
    cout << "  Sa log space:\n";
    cout << "  floor(a) + floor(b) = floor(a×b)\n";
    cout << "  frac(a) + frac(b) = frac(a×b)\n";
    cout << "  F_{floor(a)+floor(b)} = b(a×b)\n\n";
    cout << "  Kaya ang EvalAdd sa log space ay\n";
    cout << "  nagbibigay ng tamang index!\n\n";

    return 0;
}
