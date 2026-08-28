// ============================================
// φ-UNEXPECTED EXPERIMENTS
// Mga bagay na hindi dapat gumana pero subukan pa rin
//
// Core philosophy:
// - Sa φ-space, ang hindi inaasahan ay lumalabas
// - Walang assumptions — subukan lahat
// - Ang φ ang magsasabi kung ano ang totoo
//
// Author: Dan Fernandez / Primordial Omega Zero
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>
#include <algorithm>
#include <random>

using namespace std;

int main() {
    cout << "========================================\n";
    cout << "  φ-UNEXPECTED EXPERIMENTS\n";
    cout << "  Subukan ang Hindi Dapat Gumana\n";
    cout << "========================================\n\n";

    const double PHI = 1.6180339887498948482;
    const double PSI = -0.6180339887498948482;
    const double SQRT5 = sqrt(5.0);
    const double INV_PHI = 1.0 / PHI;

    cout << fixed << setprecision(15);

    // ========== EXPERIMENT 1: φ-REVERSE ==========
    cout << "EXPERIMENT 1: φ-REVERSE ITERATION\n";
    cout << "==================================\n\n";
    
    cout << "  x → 1/sqrt(x + 1)\n";
    cout << "  Hindi dapat mag-converge... pero subukan natin\n\n";
    
    double x = 1.0;
    vector<double> reverse_evolution;
    reverse_evolution.push_back(x);
    
    for (int i = 0; i < 20; i++) {
        x = 1.0 / sqrt(x + 1.0);
        reverse_evolution.push_back(x);
    }
    
    for (size_t i = 0; i < reverse_evolution.size(); i += 2) {
        cout << "    Step " << i << ": " << reverse_evolution[i] << "\n";
    }
    
    cout << "    Final: " << reverse_evolution.back() << "\n";
    cout << "    1/φ = " << INV_PHI << "\n";
    cout << "    Converges to 1/φ: " << (abs(reverse_evolution.back() - INV_PHI) < 0.01 ? "YES ✓" : "NO ✗") << "\n\n";

    // ========== EXPERIMENT 2: φ-MIRROR ==========
    cout << "EXPERIMENT 2: φ-MIRROR\n";
    cout << "======================\n\n";
    
    cout << "  x → φ - x\n";
    cout << "  Reflection sa φ... baka may symmetry\n\n";
    
    double y = 0.5;
    vector<double> mirror_evolution;
    mirror_evolution.push_back(y);
    
    for (int i = 0; i < 20; i++) {
        y = PHI - y;
        mirror_evolution.push_back(y);
    }
    
    for (size_t i = 0; i < mirror_evolution.size(); i += 2) {
        cout << "    Step " << i << ": " << mirror_evolution[i] << "\n";
    }
    
    cout << "    Pattern: " << mirror_evolution[0] << " ↔ " << mirror_evolution[1] << "\n";
    cout << "    Oscillating: " << (abs(mirror_evolution[0] - mirror_evolution[2]) < 0.001 ? "YES ✓" : "NO ✗") << "\n\n";

    // ========== EXPERIMENT 3: φ-SINE ==========
    cout << "EXPERIMENT 3: φ-SINE\n";
    cout << "====================\n\n";
    
    cout << "  x → sin(x × φ)\n";
    cout << "  Trigonometric φ... baka may period\n\n";
    
    double z = 1.0;
    vector<double> sine_evolution;
    sine_evolution.push_back(z);
    
    for (int i = 0; i < 20; i++) {
        z = sin(z * PHI);
        sine_evolution.push_back(z);
    }
    
    for (size_t i = 0; i < sine_evolution.size(); i += 2) {
        cout << "    Step " << i << ": " << sine_evolution[i] << "\n";
    }
    
    cout << "    Final: " << sine_evolution.back() << "\n";
    cout << "    Bounded: " << (abs(sine_evolution.back()) <= 1.0 ? "YES ✓" : "NO ✗") << "\n\n";

    // ========== EXPERIMENT 4: φ-RANDOM WALK ==========
    cout << "EXPERIMENT 4: φ-RANDOM WALK\n";
    cout << "===========================\n\n";
    
    cout << "  Random φ-powers... baka may hidden pattern\n\n";
    
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> dis(-3, 3);
    
    double w = 1.0;
    vector<double> random_evolution;
    random_evolution.push_back(w);
    
    for (int i = 0; i < 20; i++) {
        int power = dis(gen);
        w = w * pow(PHI, power);
        // Normalize kung masyadong malaki o maliit
        if (abs(w) > 100) w = w / 100;
        if (abs(w) < 0.01) w = w * 100;
        random_evolution.push_back(w);
    }
    
    for (size_t i = 0; i < random_evolution.size(); i += 2) {
        cout << "    Step " << i << ": " << random_evolution[i] << "\n";
    }
    
    cout << "    Final: " << random_evolution.back() << "\n";
    cout << "    Bounded: " << (abs(random_evolution.back()) < 10 ? "YES ✓" : "NO ✗") << "\n\n";

    // ========== EXPERIMENT 5: φ-FACTORIAL ==========
    cout << "EXPERIMENT 5: φ-FACTORIAL\n";
    cout << "=========================\n\n";
    
    cout << "  x → x! / φ^n... baka may convergence\n\n";
    
    double f = 1.0;
    vector<double> factorial_evolution;
    factorial_evolution.push_back(f);
    
    for (int i = 1; i <= 10; i++) {
        f = f * i / pow(PHI, i);
        factorial_evolution.push_back(f);
    }
    
    for (size_t i = 0; i < factorial_evolution.size(); i += 2) {
        cout << "    Step " << i << ": " << factorial_evolution[i] << "\n";
    }
    
    cout << "    Final: " << factorial_evolution.back() << "\n";
    cout << "    Converges: " << (abs(factorial_evolution.back()) < 0.001 ? "YES ✓" : "NO ✗") << "\n\n";

    // ========== EXPERIMENT 6: φ-LOGARITHM ==========
    cout << "EXPERIMENT 6: φ-LOGARITHM\n";
    cout << "=========================\n\n";
    
    cout << "  x → log(x + φ)... baka may fixed point\n\n";
    
    double l = 10.0;
    vector<double> log_evolution;
    log_evolution.push_back(l);
    
    for (int i = 0; i < 20; i++) {
        l = log(l + PHI);
        log_evolution.push_back(l);
    }
    
    for (size_t i = 0; i < log_evolution.size(); i += 2) {
        cout << "    Step " << i << ": " << log_evolution[i] << "\n";
    }
    
    cout << "    Final: " << log_evolution.back() << "\n";
    cout << "    Stable: " << (abs(log_evolution.back() - log_evolution[log_evolution.size()-2]) < 0.001 ? "YES ✓" : "NO ✗") << "\n\n";

    // ========== EXPERIMENT 7: φ-HARMONIC SERIES ==========
    cout << "EXPERIMENT 7: φ-HARMONIC SERIES\n";
    cout << "================================\n\n";
    
    cout << "  Sum ng 1/(n×φ)... baka may convergence\n\n";
    
    double harmonic = 0;
    vector<double> harmonic_series;
    
    for (int i = 1; i <= 20; i++) {
        harmonic += 1.0 / (i * PHI);
        harmonic_series.push_back(harmonic);
    }
    
    for (size_t i = 0; i < harmonic_series.size(); i += 2) {
        cout << "    n=" << i+1 << ": " << harmonic_series[i] << "\n";
    }
    
    cout << "    Final: " << harmonic_series.back() << "\n";
    cout << "    Diverges: " << (harmonic_series.back() > 1 ? "YES (pero mabagal)" : "NO") << "\n\n";

    return 0;
}
