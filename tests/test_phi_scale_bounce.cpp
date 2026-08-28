// ============================================
// φ-SCALE BOUNCE
// Ang x/(φ+x) ang natural na bounded function
//
// Core fix:
// - scale(x) = x/(φ+x) — laging nasa [0, 1]
// - Walang divergence kahit malaking x
// - Exact rational — walang polynomial approximation
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
    cout << "  φ-SCALE BOUNCE\n";
    cout << "  x/(φ+x) ang Natural na Bounded\n";
    cout << "========================================\n\n";

    const double PHI = 1.6180339887498948482;
    const double INV_PHI = 1.0 / PHI;

    cout << fixed << setprecision(15);

    // ========== φ-SCALE TEST ==========
    cout << "φ-SCALE TEST:\n";
    cout << "=============\n\n";
    
    auto phi_scale = [&](double x) {
        return x / (PHI + x);
    };
    
    vector<double> test_values = {0.1, 0.5, 1.0, 2.0, 5.0, 10.0, 50.0, 100.0, 1000.0, 10000.0};
    
    cout << "  x           → scale\n";
    cout << "  -----------  ------\n";
    for (double x : test_values) {
        cout << "  " << setw(11) << x << " → " << phi_scale(x) << "\n";
    }
    cout << "\n";
    
    cout << "  Lahat bounded sa [0, 1]: ";
    bool all_bounded = true;
    for (double x : test_values) {
        double s = phi_scale(x);
        if (s < 0 || s >= 1.0) all_bounded = false;
    }
    cout << (all_bounded ? "YES ✓" : "NO ✗") << "\n\n";

    // ========== φ-SCALE ITERATION ==========
    cout << "φ-SCALE ITERATION:\n";
    cout << "==================\n\n";
    
    double x = 1.0;
    vector<double> evolution;
    evolution.push_back(x);
    
    for (int i = 0; i < 20; i++) {
        x = phi_scale(x * 10.0);  // Multiply then scale
        evolution.push_back(x);
    }
    
    cout << "  x × 10 then scale (20 iterations):\n";
    for (size_t i = 0; i < evolution.size(); i += 2) {
        cout << "    Step " << i << ": " << evolution[i] << "\n";
    }
    
    cout << "\n  Final: " << evolution.back() << "\n";
    cout << "  Bounded sa [0, 1]: " << (evolution.back() >= 0 && evolution.back() < 1.0 ? "YES ✓" : "NO ✗") << "\n\n";

    // ========== φ-SCALE FHE ==========
    cout << "φ-SCALE FHE COMPATIBILITY:\n";
    cout << "==========================\n\n";
    
    cout << "  Sa FHE, kailangan ng polynomial approximation:\n";
    cout << "  x/(φ+x) = 1 - φ/(φ+x)\n";
    cout << "  = 1 - 1/(1 + x/φ)\n\n";
    
    cout << "  Para sa y = x/φ ∈ [0, ∞]:\n";
    cout << "  1/(1+y) ≈ 1 - y (kung y < 1)\n";
    cout << "  1/(1+y) ≈ 1/y (kung y > 1)\n\n";
    
    cout << "  Pinaka-simple: scale(x) = 1 - 1/(1 + x/φ)\n";
    cout << "  Sa FHE: kailangan ng conditional logic\n\n";

    // ========== φ-DOUBLE SCALE ==========
    cout << "φ-DOUBLE SCALE:\n";
    cout << "===============\n\n";
    
    auto phi_double_scale = [&](double x) {
        // Doble scale para sa mas mahigpit na bound
        return x / (PHI + x) / (PHI + x / (PHI + x));
    };
    
    cout << "  x           → double scale\n";
    cout << "  -----------  -------------\n";
    for (double x : {1.0, 10.0, 100.0}) {
        cout << "  " << setw(11) << x << " → " << phi_double_scale(x) << "\n";
    }
    cout << "\n";
    
    cout << "  Bounded sa [0, 1/φ]: ";
    all_bounded = true;
    for (double x : test_values) {
        double s = phi_double_scale(x);
        if (s < 0 || s >= INV_PHI) all_bounded = false;
    }
    cout << (all_bounded ? "YES ✓" : "NO ✗") << "\n\n";

    return 0;
}
