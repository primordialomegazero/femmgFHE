// ============================================
// φ-DATA GATHERING
// Walang agenda — puro exploration!
//
// Author: Dan Fernandez / Primordial Omega Zero
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>
#include <algorithm>
#include <complex>
#include <random>
#include <functional>

using namespace std;

int main() {
    cout << "========================================\n";
    cout << "  φ-DATA GATHERING\n";
    cout << "  Walang Agenda — Puro Exploration\n";
    cout << "========================================\n\n";

    const double PHI = 1.6180339887498948482;
    const double INV_PHI = 1.0 / PHI;
    const double SQRT5 = sqrt(5.0);

    cout << fixed << setprecision(15);

    // ========== DATA 1: φ-POWERS ==========
    cout << "DATA 1: φ-POWERS\n";
    cout << "================\n\n";
    for (int n = -10; n <= 10; n++) {
        cout << "  φ^" << setw(3) << n << " = " << pow(PHI, n) << "\n";
    }
    cout << "\n";

    // ========== DATA 2: φ-FIBONACCI RATIOS ==========
    cout << "DATA 2: FIBONACCI RATIOS\n";
    cout << "========================\n\n";
    vector<long long> fib = {0, 1};
    for (int i = 2; i <= 30; i++) fib.push_back(fib[i-1] + fib[i-2]);
    
    for (int i = 1; i <= 29; i++) {
        double ratio = (double)fib[i+1] / fib[i];
        cout << "  F(" << i+1 << ")/F(" << i << ") = " << ratio 
             << "  |ratio - φ| = " << abs(ratio - PHI) << "\n";
    }
    cout << "\n";

    // ========== DATA 3: φ-CONTINUED FRACTION ==========
    cout << "DATA 3: CONTINUED FRACTION CONVERGENTS\n";
    cout << "=====================================\n\n";
    
    vector<long long> cf_num = {0, 1};
    vector<long long> cf_den = {1, 1};
    for (int i = 2; i <= 15; i++) {
        cf_num.push_back(cf_num[i-1] + cf_num[i-2]);
        cf_den.push_back(cf_den[i-1] + cf_den[i-2]);
        double conv = (double)cf_num[i] / cf_den[i];
        cout << "  Convergent " << setw(2) << i << ": " << conv 
             << "  = " << cf_num[i] << "/" << cf_den[i] << "\n";
    }
    cout << "\n";

    // ========== DATA 4: φ-TRIGONOMETRIC ==========
    cout << "DATA 4: φ-TRIGONOMETRIC\n";
    cout << "=======================\n\n";
    cout << "  sin(φ) = " << sin(PHI) << "\n";
    cout << "  cos(φ) = " << cos(PHI) << "\n";
    cout << "  tan(φ) = " << tan(PHI) << "\n";
    cout << "  sin(φ²) = " << sin(PHI*PHI) << "\n";
    cout << "  cos(φ²) = " << cos(PHI*PHI) << "\n";
    cout << "  sin(φ³) = " << sin(PHI*PHI*PHI) << "\n\n";

    // ========== DATA 5: φ-LOGARITHMS ==========
    cout << "DATA 5: φ-LOGARITHMS\n";
    cout << "====================\n\n";
    cout << "  ln(φ) = " << log(PHI) << "\n";
    cout << "  log₁₀(φ) = " << log10(PHI) << "\n";
    cout << "  log₂(φ) = " << log2(PHI) << "\n";
    cout << "  ln(φ²) = " << log(PHI*PHI) << "\n";
    cout << "  ln(φ)/ln(2) = " << log(PHI)/log(2) << "\n\n";

    // ========== DATA 6: φ-PRIMES ==========
    cout << "DATA 6: φ-PRIME-LIKE PATTERNS\n";
    cout << "=============================\n\n";
    for (int n = 1; n <= 20; n++) {
        double phi_n = pow(PHI, n);
        double nearest = round(phi_n);
        double diff = abs(phi_n - nearest);
        cout << "  φ^" << setw(2) << n << " = " << setw(15) << phi_n 
             << "  nearest int: " << setw(10) << nearest 
             << "  diff: " << diff << "\n";
    }
    cout << "\n";

    // ========== DATA 7: φ-RANDOM WALK ==========
    cout << "DATA 7: φ-RANDOM WALK\n";
    cout << "=====================\n\n";
    random_device rd;
    mt19937 gen(rd());
    uniform_real_distribution<> dis(0, 1);
    
    double walk = 0;
    vector<double> walk_data;
    for (int i = 0; i < 50; i++) {
        walk += (dis(gen) > 0.5 ? 1.0 : -1.0) * INV_PHI;
        walk_data.push_back(walk);
    }
    
    cout << "  Random walk with φ-steps:\n";
    for (size_t i = 0; i < walk_data.size(); i += 5) {
        cout << "    Step " << i << ": " << walk_data[i] << "\n";
    }
    cout << "\n";

    // ========== DATA 8: φ-MATRIX ==========
    cout << "DATA 8: φ-MATRIX PROPERTIES\n";
    cout << "===========================\n\n";
    
    cout << "  Fibonacci Q-matrix: [[1,1],[1,0]]\n";
    cout << "  Q^n = [[F(n+1), F(n)], [F(n), F(n-1)]]\n";
    cout << "  Determinant: -1 (constant!)\n\n";
    
    cout << "  φ-matrix: [[φ,1],[1,0]]\n";
    cout << "  Eigenvalues: φ at -1/φ\n";
    cout << "  φ × (-1/φ) = " << PHI * (-INV_PHI) << "\n\n";

    // ========== DATA 9: φ-SERIES ==========
    cout << "DATA 9: φ-SERIES CONVERGENCE\n";
    cout << "============================\n\n";
    
    double sum = 0;
    for (int n = 0; n <= 20; n++) {
        sum += pow(INV_PHI, n);
    }
    cout << "  Σ(1/φ^n) para sa n=0..20: " << sum << "\n";
    cout << "  Expected (geometric): " << 1.0/(1.0 - INV_PHI) << "\n\n";
    
    double sum2 = 0;
    for (int n = 1; n <= 100; n++) {
        sum2 += 1.0/(n*n) * PHI;
    }
    cout << "  Σ(φ/n²) para sa n=1..100: " << sum2 << "\n";
    cout << "  π²/6 × φ = " << M_PI*M_PI/6 * PHI << "\n\n";

    // ========== DATA 10: φ-ITERATIONS ==========
    cout << "DATA 10: φ-ITERATIONS (VARIOUS MAPS)\n";
    cout << "====================================\n\n";
    
    vector<pair<string, function<double(double)>>> maps = {
        {"x→√(x+φ)", [PHI](double x){ return sqrt(x + PHI); }},
        {"x→φ/(1+x)", [PHI](double x){ return PHI/(1+x); }},
        {"x→x²-φ", [PHI](double x){ return x*x - PHI; }},
        {"x→φ-x²", [PHI](double x){ return PHI - x*x; }},
        {"x→1/(φ-x)", [PHI](double x){ return 1.0/(PHI - x); }}
    };
    
    for (auto& m : maps) {
        double x = 0.5;
        vector<double> iter;
        iter.push_back(x);
        
        for (int i = 0; i < 10; i++) {
            x = m.second(x);
            iter.push_back(x);
            if (abs(x) > 100) break;
        }
        
        cout << "  " << m.first << ":\n";
        cout << "    0.5 → ";
        for (size_t i = 1; i < min(iter.size(), size_t(6)); i++) {
            cout << iter[i];
            if (i < 5) cout << " → ";
        }
        cout << "\n";
    }
    cout << "\n";

    // ========== SUMMARY ==========
    cout << "DATA COLLECTED:\n";
    cout << "===============\n\n";
    cout << "  1. φ-powers (n=-10 to 10)\n";
    cout << "  2. Fibonacci ratios (convergence sa φ)\n";
    cout << "  3. Continued fractions (convergents)\n";
    cout << "  4. φ-trigonometric values\n";
    cout << "  5. φ-logarithms\n";
    cout << "  6. φ-prime-like patterns\n";
    cout << "  7. φ-random walk\n";
    cout << "  8. φ-matrix properties\n";
    cout << "  9. φ-series convergence\n";
    cout << "  10. φ-iterations (5 maps)\n\n";

    return 0;
}
