// ============================================
// φ-EXACT BOUNCE
// Exact bounce na walang Taylor approximation
//
// Core fix:
// - φ-modular: x → x mod φ
// - Exact formula na walang divergence
// - Stable para sa lahat ng values
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
    cout << "  φ-EXACT BOUNCE\n";
    cout << "  Walang Taylor Approximation\n";
    cout << "========================================\n\n";

    const double PHI = 1.6180339887498948482;
    const double INV_PHI = 1.0 / PHI;

    cout << fixed << setprecision(15);

    // ========== φ-MODULAR BOUNCE ==========
    cout << "φ-MODULAR BOUNCE:\n";
    cout << "=================\n\n";
    
    auto phi_bounce = [&](double x) {
        // x mod φ = x - φ × floor(x/φ)
        return x - PHI * floor(x / PHI);
    };
    
    // Test sa iba't ibang values
    vector<double> test_values = {0.5, 1.0, 2.0, 5.0, 10.0, 50.0, 100.0, 1000.0};
    
    cout << "  x       → bounce\n";
    cout << "  -------  ------\n";
    for (double x : test_values) {
        cout << "  " << setw(7) << x << " → " << phi_bounce(x) << "\n";
    }
    cout << "\n";
    
    cout << "  Lahat bounded sa [0, φ]: ";
    bool all_bounded = true;
    for (double x : test_values) {
        double b = phi_bounce(x);
        if (b < 0 || b >= PHI) all_bounded = false;
    }
    cout << (all_bounded ? "YES ✓" : "NO ✗") << "\n\n";

    // ========== φ-SIGMOID BOUNCE ==========
    cout << "φ-SIGMOID BOUNCE:\n";
    cout << "=================\n\n";
    
    auto phi_sigmoid_bounce = [&](double x) {
        // sigmoid-based bounce para sa FHE
        double x_over_phi = x * INV_PHI;
        double frac_part = x_over_phi - floor(x_over_phi);
        // sigmoid: 1/(1 + exp(-10×(frac - 0.5)))
        double sig = 1.0 / (1.0 + exp(-10.0 * (frac_part - 0.5)));
        return PHI * sig;
    };
    
    cout << "  x       → sigmoid bounce\n";
    cout << "  -------  --------------\n";
    for (double x : test_values) {
        cout << "  " << setw(7) << x << " → " << phi_sigmoid_bounce(x) << "\n";
    }
    cout << "\n";
    
    cout << "  Lahat bounded sa [0, φ]: ";
    all_bounded = true;
    for (double x : test_values) {
        double b = phi_sigmoid_bounce(x);
        if (b < 0 || b >= PHI) all_bounded = false;
    }
    cout << (all_bounded ? "YES ✓" : "NO ✗") << "\n\n";

    // ========== φ-TANH BOUNCE ==========
    cout << "φ-TANH BOUNCE:\n";
    cout << "==============\n\n";
    
    auto phi_tanh_bounce = [&](double x) {
        // tanh-based bounce
        double x_over_phi = x * INV_PHI;
        double frac_part = x_over_phi - floor(x_over_phi);
        // tanh: tanh(5×(frac - 0.5))
        double tanh_val = tanh(5.0 * (frac_part - 0.5));
        return PHI * (1.0 + tanh_val) / 2.0;
    };
    
    cout << "  x       → tanh bounce\n";
    cout << "  -------  ------------\n";
    for (double x : test_values) {
        cout << "  " << setw(7) << x << " → " << phi_tanh_bounce(x) << "\n";
    }
    cout << "\n";
    
    cout << "  Lahat bounded sa [0, φ]: ";
    all_bounded = true;
    for (double x : test_values) {
        double b = phi_tanh_bounce(x);
        if (b < 0 || b >= PHI) all_bounded = false;
    }
    cout << (all_bounded ? "YES ✓" : "NO ✗") << "\n\n";

    // ========== φ-ITERATION TEST ==========
    cout << "φ-ITERATION TEST:\n";
    cout << "=================\n\n";
    
    // Test kung stable ang bounce sa iteration
    double x = 1.0;
    vector<double> evolution;
    evolution.push_back(x);
    
    for (int i = 0; i < 20; i++) {
        x = phi_tanh_bounce(x * 10.0);  // Multiply then bounce
        evolution.push_back(x);
    }
    
    cout << "  x × 10 then bounce (20 iterations):\n";
    for (size_t i = 0; i < evolution.size(); i += 2) {
        cout << "    Step " << i << ": " << evolution[i] << "\n";
    }
    
    cout << "\n  Final: " << evolution.back() << "\n";
    cout << "  Bounded: " << (evolution.back() >= 0 && evolution.back() < PHI ? "YES ✓" : "NO ✗") << "\n\n";

    return 0;
}
