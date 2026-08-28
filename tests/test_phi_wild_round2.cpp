// ============================================
// φ-WILD RESEARCH ROUND 2
// Mas wild na φ-experiments
//
// Core philosophy:
// - Walang masamang ideya sa φ-space
// - Ang φ ang magsasabi kung ano ang totoo
// - Subukan lahat ng walang assumptions
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
    cout << "  φ-WILD RESEARCH ROUND 2\n";
    cout << "  Walang Masamang Ideya\n";
    cout << "========================================\n\n";

    const double PHI = 1.6180339887498948482;
    const double PSI = -0.6180339887498948482;
    const double INV_PHI = 1.0 / PHI;

    cout << fixed << setprecision(15);

    // ========== EXPERIMENT 1: φ-INVERSE ==========
    cout << "EXPERIMENT 1: φ-INVERSE\n";
    cout << "=======================\n\n";
    cout << "  x → 1/(x + φ)\n\n";
    
    double x = 1.0;
    vector<double> inv_evolution;
    inv_evolution.push_back(x);
    
    for (int i = 0; i < 20; i++) {
        x = 1.0 / (x + PHI);
        inv_evolution.push_back(x);
    }
    
    cout << "  Evolution:\n";
    for (size_t i = 0; i < inv_evolution.size(); i += 2) {
        cout << "    Step " << i << ": " << inv_evolution[i] << "\n";
    }
    cout << "  Final: " << inv_evolution.back() << "\n";
    cout << "  1/φ = " << INV_PHI << "\n";
    cout << "  Converges: " << (abs(inv_evolution.back() - INV_PHI) < 0.1 ? "YES ✓" : "NO ✗") << "\n\n";

    // ========== EXPERIMENT 2: φ-SQUARE ROOT ==========
    cout << "EXPERIMENT 2: φ-SQUARE ROOT\n";
    cout << "===========================\n\n";
    cout << "  x → sqrt(x × φ)\n\n";
    
    double y = 0.5;
    vector<double> sqrt_evolution;
    sqrt_evolution.push_back(y);
    
    for (int i = 0; i < 20; i++) {
        y = sqrt(y * PHI);
        sqrt_evolution.push_back(y);
    }
    
    cout << "  Evolution:\n";
    for (size_t i = 0; i < sqrt_evolution.size(); i += 2) {
        cout << "    Step " << i << ": " << sqrt_evolution[i] << "\n";
    }
    cout << "  Final: " << sqrt_evolution.back() << "\n";
    cout << "  φ = " << PHI << "\n";
    cout << "  Converges to φ: " << (abs(sqrt_evolution.back() - PHI) < 0.01 ? "YES ✓" : "NO ✗") << "\n\n";

    // ========== EXPERIMENT 3: φ-EXPONENTIAL ==========
    cout << "EXPERIMENT 3: φ-EXPONENTIAL\n";
    cout << "===========================\n\n";
    cout << "  x → exp(-x/φ)\n\n";
    
    double z = 1.0;
    vector<double> exp_evolution;
    exp_evolution.push_back(z);
    
    for (int i = 0; i < 20; i++) {
        z = exp(-z * INV_PHI);
        exp_evolution.push_back(z);
    }
    
    cout << "  Evolution:\n";
    for (size_t i = 0; i < exp_evolution.size(); i += 2) {
        cout << "    Step " << i << ": " << exp_evolution[i] << "\n";
    }
    cout << "  Final: " << exp_evolution.back() << "\n";
    cout << "  Bounded: " << (exp_evolution.back() > 0 && exp_evolution.back() < 1 ? "YES ✓" : "NO ✗") << "\n\n";

    // ========== EXPERIMENT 4: φ-TANGENT ==========
    cout << "EXPERIMENT 4: φ-TANGENT\n";
    cout << "=======================\n\n";
    cout << "  x → tan(x/φ)\n\n";
    
    double w = 0.5;
    vector<double> tan_evolution;
    tan_evolution.push_back(w);
    
    for (int i = 0; i < 20; i++) {
        w = tan(w * INV_PHI);
        tan_evolution.push_back(w);
    }
    
    cout << "  Evolution:\n";
    for (size_t i = 0; i < tan_evolution.size(); i += 2) {
        cout << "    Step " << i << ": " << tan_evolution[i] << "\n";
    }
    cout << "  Final: " << tan_evolution.back() << "\n";
    cout << "  Bounded: " << (abs(tan_evolution.back()) < 10 ? "YES ✓" : "NO ✗") << "\n\n";

    // ========== EXPERIMENT 5: φ-FRACTION ==========
    cout << "EXPERIMENT 5: φ-FRACTION\n";
    cout << "========================\n\n";
    cout << "  x → x/(x + φ)\n\n";
    
    double v = 10.0;
    vector<double> frac_evolution;
    frac_evolution.push_back(v);
    
    for (int i = 0; i < 20; i++) {
        v = v / (v + PHI);
        frac_evolution.push_back(v);
    }
    
    cout << "  Evolution:\n";
    for (size_t i = 0; i < frac_evolution.size(); i += 2) {
        cout << "    Step " << i << ": " << frac_evolution[i] << "\n";
    }
    cout << "  Final: " << frac_evolution.back() << "\n";
    cout << "  Converges to 0: " << (frac_evolution.back() < 0.01 ? "YES ✓" : "NO ✗") << "\n\n";

    // ========== EXPERIMENT 6: φ-DOUBLE ==========
    cout << "EXPERIMENT 6: φ-DOUBLE ITERATION\n";
    cout << "================================\n\n";
    cout << "  x → φ - 1/x\n\n";
    
    double u = 2.0;
    vector<double> double_evolution;
    double_evolution.push_back(u);
    
    for (int i = 0; i < 20; i++) {
        u = PHI - 1.0 / u;
        double_evolution.push_back(u);
    }
    
    cout << "  Evolution:\n";
    for (size_t i = 0; i < double_evolution.size(); i += 2) {
        cout << "    Step " << i << ": " << double_evolution[i] << "\n";
    }
    cout << "  Final: " << double_evolution.back() << "\n";
    cout << "  φ = " << PHI << "\n";
    cout << "  Converges to φ: " << (abs(double_evolution.back() - PHI) < 0.1 ? "YES ✓" : "NO ✗") << "\n\n";

    // ========== EXPERIMENT 7: φ-CUBIC ==========
    cout << "EXPERIMENT 7: φ-CUBIC\n";
    cout << "=====================\n\n";
    cout << "  x → x³ - φ×x² + x\n\n";
    
    double q = 0.5;
    vector<double> cubic_evolution;
    cubic_evolution.push_back(q);
    
    for (int i = 0; i < 20; i++) {
        q = q * q * q - PHI * q * q + q;
        cubic_evolution.push_back(q);
    }
    
    cout << "  Evolution:\n";
    for (size_t i = 0; i < cubic_evolution.size(); i += 2) {
        cout << "    Step " << i << ": " << cubic_evolution[i] << "\n";
    }
    cout << "  Final: " << cubic_evolution.back() << "\n";
    cout << "  Bounded: " << (abs(cubic_evolution.back()) < 10 ? "YES ✓" : "NO ✗") << "\n\n";

    // ========== EXPERIMENT 8: φ-MIXED ==========
    cout << "EXPERIMENT 8: φ-MIXED ITERATION\n";
    cout << "===============================\n\n";
    cout << "  x → (x + φ)/(x + 1)\n\n";
    
    double m = 5.0;
    vector<double> mixed_evolution;
    mixed_evolution.push_back(m);
    
    for (int i = 0; i < 20; i++) {
        m = (m + PHI) / (m + 1.0);
        mixed_evolution.push_back(m);
    }
    
    cout << "  Evolution:\n";
    for (size_t i = 0; i < mixed_evolution.size(); i += 2) {
        cout << "    Step " << i << ": " << mixed_evolution[i] << "\n";
    }
    cout << "  Final: " << mixed_evolution.back() << "\n";
    cout << "  φ = " << PHI << "\n";
    cout << "  Converges to φ: " << (abs(mixed_evolution.back() - PHI) < 0.1 ? "YES ✓" : "NO ✗") << "\n\n";

    return 0;
}
