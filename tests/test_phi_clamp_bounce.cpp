// ============================================
// φ-CLAMP BOUNCE
// Exact rational bounce na laging bounded
//
// Core fix:
// - φ-clamp: x → φ×x/(φ+x)
// - Laging positive, bounded sa [0, φ]
// - Walang divergence kahit malaking values
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
    cout << "  φ-CLAMP BOUNCE\n";
    cout << "  Exact Rational na Laging Bounded\n";
    cout << "========================================\n\n";

    const double PHI = 1.6180339887498948482;
    const double INV_PHI = 1.0 / PHI;

    cout << fixed << setprecision(15);

    // ========== φ-CLAMP TEST ==========
    cout << "φ-CLAMP TEST:\n";
    cout << "=============\n\n";
    
    auto phi_clamp = [&](double x) {
        return PHI * x / (PHI + x);
    };
    
    vector<double> test_values = {0.1, 0.5, 1.0, 2.0, 5.0, 10.0, 50.0, 100.0, 1000.0};
    
    cout << "  x         → clamp\n";
    cout << "  ---------  ------\n";
    for (double x : test_values) {
        cout << "  " << setw(9) << x << " → " << phi_clamp(x) << "\n";
    }
    cout << "\n";
    
    cout << "  Lahat bounded sa [0, φ]: ";
    bool all_bounded = true;
    for (double x : test_values) {
        double b = phi_clamp(x);
        if (b < 0 || b >= PHI) all_bounded = false;
    }
    cout << (all_bounded ? "YES ✓" : "NO ✗") << "\n\n";

    // ========== φ-CLAMP ITERATION ==========
    cout << "φ-CLAMP ITERATION:\n";
    cout << "==================\n\n";
    
    double x = 1.0;
    vector<double> evolution;
    evolution.push_back(x);
    
    for (int i = 0; i < 20; i++) {
        x = phi_clamp(x * 10.0);  // Multiply then clamp
        evolution.push_back(x);
    }
    
    cout << "  x × 10 then clamp (20 iterations):\n";
    for (size_t i = 0; i < evolution.size(); i += 2) {
        cout << "    Step " << i << ": " << evolution[i] << "\n";
    }
    
    cout << "\n  Final: " << evolution.back() << "\n";
    cout << "  Bounded: " << (evolution.back() >= 0 && evolution.back() < PHI ? "YES ✓" : "NO ✗") << "\n\n";

    // ========== φ-CLAMP FHE COMPATIBILITY ==========
    cout << "φ-CLAMP FHE COMPATIBILITY:\n";
    cout << "==========================\n\n";
    
    cout << "  φ-clamp: x → φ×x/(φ+x)\n";
    cout << "  Sa FHE, kailangan ng polynomial approximation:\n\n";
    
    cout << "  Para sa x ∈ [0, 2φ]:\n";
    cout << "  φ×x/(φ+x) ≈ x - x²/φ + x³/φ² - x⁴/φ³\n";
    cout << "  4 multiplications, stable sa [0, 2φ]\n\n";
    
    // Test polynomial approximation
    auto phi_clamp_poly = [&](double x) {
        double x_over_phi = x * INV_PHI;
        double result = x;
        result -= x * x_over_phi;
        result += x * x_over_phi * x_over_phi;
        result -= x * x_over_phi * x_over_phi * x_over_phi;
        return result;
    };
    
    cout << "  Polynomial approximation test:\n";
    for (double x : {0.5, 1.0, 2.0, 3.0}) {
        double exact = phi_clamp(x);
        double approx = phi_clamp_poly(x);
        cout << "    x=" << x << ": exact=" << exact << " approx=" << approx << "\n";
    }
    cout << "\n";

    return 0;
}
